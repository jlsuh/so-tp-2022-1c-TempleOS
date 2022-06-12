#include "scheduler.h"

#include <commons/collections/list.h>
#include <commons/log.h>
#include <commons/string.h>
#include <commons/temporal.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdint.h>
#include <stdio.h>
#include <stdnoreturn.h>
#include <unistd.h>

#include "buffer.h"
#include "common_flags.h"
#include "common_utils.h"
#include "cpu_adapter.h"
#include "estado.h"
#include "instruccion.h"
#include "kernel_config.h"
#include "mem_adapter.h"
#include "pcb.h"
#include "stream.h"

typedef void (*t_suspension_handler)(void);

extern pthread_mutex_t mutexSocketMemoria;

extern t_log* kernelLogger;
extern t_kernel_config* kernelConfig;

static t_dispatch_handler elegir_pcb;
static t_suspension_handler evaluar_suspension;
static t_onBlocked_handler actualizar_pcb_por_bloqueo;

static uint32_t nextPid;
static pthread_mutex_t nextPidMutex;

static sem_t gradoMultiprog;
static sem_t hayPcbsParaAgregarAlSistema;
static sem_t dispatchPermitido;
static sem_t transicionarSusreadyAReady;

static t_estado* estadoNew;
static t_estado* estadoReady;
static t_estado* estadoExec;
static t_estado* estadoExit;
static t_estado* estadoBlocked;
static t_estado* estadoSuspendedBlocked;
static t_estado* estadoSuspendedReady;

static t_estado* pcbsEsperandoParaIO;

static bool hayQueDesalojar;
static pthread_mutex_t hayQueDesalojarMutex;

//////////////////////////////////////// General Functions ////////////////////////////////////////
static void log_transition(const char* prev, const char* post, int pid) {
    char* transicion = string_from_format("\e[1;93m%s->%s\e[0m", prev, post);
    log_info(kernelLogger, "Transición de %s PCB <ID %d>", transicion, pid);
    free(transicion);
}

static void responder_memoria_insuficiente(t_pcb* pcb) {
    estado_encolar_pcb_atomic(estadoExit, pcb);
    log_transition("NEW", "EXIT", pcb_get_pid(pcb));
    log_info(kernelLogger, "Memoria insuficiente para alojar el proceso %d", pcb_get_pid(pcb));
    pcb_responder_a_consola(pcb, HEADER_memoria_insuficiente);
}

static uint32_t obtener_siguiente_pid(void) {
    pthread_mutex_lock(&nextPidMutex);
    uint32_t newNextPid = nextPid++;
    pthread_mutex_unlock(&nextPidMutex);
    return newNextPid;
}

//////////////////////////////////////// Scheduling Algorithms ////////////////////////////////////////
//////////////////// FIFO ////////////////////
t_pcb* elegir_pcb_segun_fifo(t_estado* estado, double _) {
    return estado_desencolar_primer_pcb_atomic(estado);
}

static void __evaluar_suspension_segun_fifo(void) {}

static void __actualizar_pcb_por_bloqueo_segun_fifo(t_pcb* _, uint32_t __, double ___) {}

//////////////////// SRT ////////////////////
static void __evaluar_suspension_segun_srt(void) {
    pthread_mutex_lock(&hayQueDesalojarMutex);
    if (hayQueDesalojar) {
        hayQueDesalojar = false;
        pthread_mutex_lock(estado_get_mutex(estadoExec));
        if (list_size(estado_get_list(estadoExec)) == 1) {
            cpu_adapter_interrumpir_cpu(kernelConfig, kernelLogger);
        }
        pthread_mutex_unlock(estado_get_mutex(estadoExec));
    }
    pthread_mutex_unlock(&hayQueDesalojarMutex);
}

void actualizar_pcb_por_bloqueo_segun_srt(t_pcb* pcb, uint32_t realEjecutado, double alfa) {
    pcb_set_real_anterior(pcb, pcb_get_reales_ejecutados_hasta_ahora(pcb) + realEjecutado);  // Digo que el real anterior es la sumatoria de los reales ejecutados hasta ahora, antes de bloquearse
    pcb_set_reales_ejecutados_hasta_ahora(pcb, 0);                                           // Resetear reales ejecutados debido a que ya ejecutaron toda su ráfaga de CPU
    double siguienteEstimacion = calcular_siguiente_estimacion(pcb, alfa);
    pcb_set_estimacion_actual(pcb, siguienteEstimacion);  // Establezco la nueva estimación para cuando me desbloquee (estimamos antes por conveniencia)
}

double calcular_estimacion_restante(t_pcb* pcb) {
    return pcb_get_estimacion_actual(pcb) - pcb_get_reales_ejecutados_hasta_ahora(pcb);
}

t_pcb* menor_rafaga_restante(t_pcb* unPcb, t_pcb* otroPcb) {
    double unaRafagaRestante = calcular_estimacion_restante(unPcb);
    double otraRafagaRestante = calcular_estimacion_restante(otroPcb);
    // log_debug(kernelLogger, "PCB <ID %d> RAFAGA DE %lf miliseconds", pcb_get_pid(unPcb), unaRafagaRestante);
    // log_debug(kernelLogger, "PCB <ID %d> RAFAGA DE %lf miliseconds", pcb_get_pid(otroPcb), otraRafagaRestante);
    return unaRafagaRestante <= otraRafagaRestante
               ? unPcb
               : otroPcb;
}

double media_exponencial(double alfa, double realAnterior, double estimacionAnterior) {
    return alfa * realAnterior + (1 - alfa) * estimacionAnterior;
}

double calcular_siguiente_estimacion(t_pcb* pcb, double alfa) {
    return media_exponencial(
        alfa,
        pcb_get_real_anterior(pcb),
        pcb_get_estimacion_actual(pcb)  // La estimación actual es la estimación anterior al momento de calcular la media exponencial
    );
}

t_pcb* elegir_pcb_segun_srt(t_estado* estado, double alfa) {
    t_pcb* pcbElecto = NULL;
    pthread_mutex_lock(estado_get_mutex(estado));
    int cantidadPcbsEnLista = list_size(estado_get_list(estado));
    if (cantidadPcbsEnLista == 1) {
        pcbElecto = estado_desencolar_primer_pcb(estado);
        // log_debug(kernelLogger, "PCB <ID %d> ESTÁ SOLO EN READY", pcb_get_pid(pcbElecto));
    } else if (cantidadPcbsEnLista > 1) {
        pcbElecto = list_get_minimum(estado_get_list(estado), (void*)menor_rafaga_restante);
        estado_remover_pcb_de_cola(estado, pcbElecto);
    }
    pthread_mutex_unlock(estado_get_mutex(estado));
    return pcbElecto;
}

//////////////////////////////////////// Scheduler Functions ////////////////////////////////////////
static void iniciar_contador_blocked_a_suspended_blocked(void* pcbVoid) {
    t_pcb* pcb = (t_pcb*)pcbVoid;
    pthread_mutex_lock(pcb_get_mutex(pcb));
    pcb_set_veces_bloqueado(pcb, pcb_get_veces_bloqueado(pcb) + 1);
    int laVezBloqueadaQueRepresentaElHilo = pcb_get_veces_bloqueado(pcb);
    pthread_mutex_unlock(pcb_get_mutex(pcb));
    log_debug(kernelLogger, "Iniciando contador de blocked a suspended blocked de PCB <ID %d>", pcb_get_pid(pcb));
    t_pcb* dummyPCB = pcb_create(pcb_get_pid(pcb), pcb_get_tamanio(pcb), pcb_get_estimacion_actual(pcb));

    intervalo_de_pausa(kernel_config_get_tiempo_maximo_bloqueado(kernelConfig));

    if (estado_contiene_pcb_atomic(estadoBlocked, dummyPCB) && pcb_get_veces_bloqueado(pcb) == laVezBloqueadaQueRepresentaElHilo) {
        if (estado_remover_pcb_de_cola_atomic(estadoBlocked, dummyPCB) != NULL) {
            pcb_test_and_set_tiempo_final_bloqueado(pcb);

            pthread_mutex_lock(pcb_get_mutex(pcb));
            double tiempoEsperandoEnBlocked = difftime(pcb_get_tiempo_final_bloqueado(pcb), pcb_get_tiempo_inicial_bloqueado(pcb));
            if (pcb_get_tiempo_de_bloqueo(pcb) / 1000.0 <= kernel_config_get_tiempo_maximo_bloqueado(kernelConfig) / 1000.0) {
                if (pcb_get_tiempo_de_bloqueo(pcb) / 1000.0 + tiempoEsperandoEnBlocked <= kernel_config_get_tiempo_maximo_bloqueado(kernelConfig) / 1000.0) {
                    pcb_destroy(dummyPCB);
                    pthread_mutex_unlock(pcb_get_mutex(pcb));
                    return;
                }
            }

            pcb_set_estado_actual(pcb, SUSPENDED_BLOCKED);
            mem_adapter_avisar_suspension(pcb, kernelConfig, kernelLogger);
            estado_encolar_pcb_atomic(estadoSuspendedBlocked, pcb);
            log_debug(kernelLogger, "Entra en suspensión PCB <ID %d>", pcb_get_pid(pcb));
            log_transition("BLOCKED", "SUSBLOCKED", pcb_get_pid(pcb));
            sem_post(&gradoMultiprog);
            pthread_mutex_unlock(pcb_get_mutex(pcb));
        }
    }

    pcb_destroy(dummyPCB);
}

static void noreturn iniciar_dispositivo_io(void) {
    for (;;) {
        sem_wait(estado_get_sem(pcbsEsperandoParaIO));
        t_pcb* pcbAEjecutarRafagasIO = estado_desencolar_primer_pcb_atomic(pcbsEsperandoParaIO);
        log_info(kernelLogger, "Ejecutando ráfagas I/O de PCB <ID %d> por %d milisegundos", pcb_get_pid(pcbAEjecutarRafagasIO), pcb_get_tiempo_de_bloqueo(pcbAEjecutarRafagasIO));
        pcb_test_and_set_tiempo_final_bloqueado(pcbAEjecutarRafagasIO);

        intervalo_de_pausa(pcb_get_tiempo_de_bloqueo(pcbAEjecutarRafagasIO));

        pthread_mutex_lock(pcb_get_mutex(pcbAEjecutarRafagasIO));
        if (pcb_get_estado_actual(pcbAEjecutarRafagasIO) == BLOCKED) {
            estado_remover_pcb_de_cola_atomic(estadoBlocked, pcbAEjecutarRafagasIO);
            pcb_set_estado_actual(pcbAEjecutarRafagasIO, READY);
            estado_encolar_pcb_atomic(estadoReady, pcbAEjecutarRafagasIO);
            pcb_set_tiempo_de_bloqueo(pcbAEjecutarRafagasIO, 0);
            pthread_mutex_lock(&hayQueDesalojarMutex);
            hayQueDesalojar = true;
            pthread_mutex_unlock(&hayQueDesalojarMutex);
            log_transition("BLOCKED", "READY", pcb_get_pid(pcbAEjecutarRafagasIO));
            sem_post(estado_get_sem(estadoReady));
        } else if (pcb_get_estado_actual(pcbAEjecutarRafagasIO) == SUSPENDED_BLOCKED) {
            estado_remover_pcb_de_cola_atomic(estadoSuspendedBlocked, pcbAEjecutarRafagasIO);
            pcb_set_estado_actual(pcbAEjecutarRafagasIO, SUSPENDED_READY);
            estado_encolar_pcb_atomic(estadoSuspendedReady, pcbAEjecutarRafagasIO);
            pcb_set_tiempo_de_bloqueo(pcbAEjecutarRafagasIO, 0);
            log_transition("SUSBLOCKED", "SUSREADY", pcb_get_pid(pcbAEjecutarRafagasIO));
            sem_post(&hayPcbsParaAgregarAlSistema);
        }
        pcb_marcar_tiempo_final_como_no_establecido(pcbAEjecutarRafagasIO);
        pthread_mutex_unlock(pcb_get_mutex(pcbAEjecutarRafagasIO));
    }
}

static void noreturn hilo_que_libera_pcbs_en_exit(void) {
    for (;;) {
        sem_wait(estado_get_sem(estadoExit));
        t_pcb* pcbALiberar = estado_desencolar_primer_pcb_atomic(estadoExit);
        mem_adapter_finalizar_proceso(pcbALiberar, kernelConfig, kernelLogger);
        log_info(kernelLogger, "Se finaliza PCB <ID %d> de tamaño %d", pcb_get_pid(pcbALiberar), pcb_get_tamanio(pcbALiberar));
        pcb_destroy(pcbALiberar);
        sem_post(&gradoMultiprog);
    }
}

static void noreturn planificador_largo_plazo(void) {
    pthread_t liberarPcbsEnExitTh;
    pthread_create(&liberarPcbsEnExitTh, NULL, (void*)hilo_que_libera_pcbs_en_exit, NULL);
    pthread_detach(liberarPcbsEnExitTh);

    for (;;) {
        sem_wait(&hayPcbsParaAgregarAlSistema);
        sem_wait(&gradoMultiprog);
        if (list_size(estado_get_list(estadoSuspendedReady)) > 0) {
            sem_post(&transicionarSusreadyAReady);
        } else {
            pthread_mutex_lock(estado_get_mutex(estadoNew));
            t_pcb* pcbQuePasaAReady = list_remove(estado_get_list(estadoNew), 0);
            pthread_mutex_unlock(estado_get_mutex(estadoNew));
            int nuevaTablaPagina = mem_adapter_obtener_tabla_pagina(pcbQuePasaAReady, kernelConfig, kernelLogger);
            pcb_set_tabla_pagina_primer_nivel(pcbQuePasaAReady, nuevaTablaPagina);
            if (nuevaTablaPagina == -1) {
                responder_memoria_insuficiente(pcbQuePasaAReady);
            } else {
                estado_encolar_pcb_atomic(estadoReady, pcbQuePasaAReady);
                pthread_mutex_lock(&hayQueDesalojarMutex);
                hayQueDesalojar = true;
                pthread_mutex_unlock(&hayQueDesalojarMutex);
                log_transition("NEW", "READY", pcb_get_pid(pcbQuePasaAReady));
                sem_post(estado_get_sem(estadoReady));
            }
            pcbQuePasaAReady = NULL;
        }
    }
}

static void noreturn planificador_mediano_plazo(void) {
    for (;;) {
        sem_wait(&transicionarSusreadyAReady);
        pthread_mutex_lock(estado_get_mutex(estadoSuspendedReady));
        t_pcb* pcbQuePasaAReady = list_remove(estado_get_list(estadoSuspendedReady), 0);
        pthread_mutex_unlock(estado_get_mutex(estadoSuspendedReady));
        pcb_set_estado_actual(pcbQuePasaAReady, READY);
        estado_encolar_pcb_atomic(estadoReady, pcbQuePasaAReady);
        pthread_mutex_lock(&hayQueDesalojarMutex);
        hayQueDesalojar = true;
        pthread_mutex_unlock(&hayQueDesalojarMutex);
        log_transition("SUSREADY", "READY", pcb_get_pid(pcbQuePasaAReady));
        sem_post(estado_get_sem(estadoReady));
        pcbQuePasaAReady = NULL;
    }
}

void actualizar_pcb_por_desalojo(t_pcb* pcb, double realEjecutado) {
    pcb_set_reales_ejecutados_hasta_ahora(pcb, pcb_get_reales_ejecutados_hasta_ahora(pcb) + realEjecutado);
}

static void atender_bloqueo(t_pcb* pcb) {
    pcb_marcar_tiempo_inicial_bloqueado(pcb);
    estado_encolar_pcb_atomic(pcbsEsperandoParaIO, pcb);
    log_transition("EXEC", "BLOCKED", pcb_get_pid(pcb));
    log_info(kernelLogger, "PCB <ID %d> ingresa a la cola de espera de I/O", pcb_get_pid(pcb));
    sem_post(estado_get_sem(pcbsEsperandoParaIO));
    pcb_set_estado_actual(pcb, BLOCKED);
    estado_encolar_pcb_atomic(estadoBlocked, pcb);
    pthread_t contadorASuspendedBlocked;
    pthread_create(&contadorASuspendedBlocked, NULL, (void*)iniciar_contador_blocked_a_suspended_blocked, (void*)pcb);
    pthread_detach(contadorASuspendedBlocked);
}

void set_timespec(struct timespec* timespec) {
    int retVal = clock_gettime(CLOCK_REALTIME, timespec);
    if (retVal == -1) {
        perror("clock_gettime");
        exit(-1);
    }
}

uint32_t obtener_diferencial_de_tiempo_en_milisegundos(struct timespec end, struct timespec start) {
    const uint32_t SECS_TO_MILISECS = 1000;
    const uint32_t NANOSECS_TO_MILISECS = 1000000;
    return (end.tv_sec - start.tv_sec) * SECS_TO_MILISECS + (end.tv_nsec - start.tv_nsec) / NANOSECS_TO_MILISECS;
}

static void noreturn atender_pcb(void) {
    for (;;) {
        sem_wait(estado_get_sem(estadoExec));

        pthread_mutex_lock(estado_get_mutex(estadoExec));
        t_pcb* pcb = list_get(estado_get_list(estadoExec), 0);
        log_transition("READY", "EXEC", pcb_get_pid(pcb));
        pthread_mutex_unlock(estado_get_mutex(estadoExec));

        struct timespec start;
        set_timespec(&start);

        cpu_adapter_enviar_pcb_a_cpu(pcb, kernelConfig, kernelLogger);
        uint8_t cpuResponse = stream_recv_header(kernel_config_get_socket_dispatch_cpu(kernelConfig));

        struct timespec end;
        set_timespec(&end);

        pthread_mutex_lock(estado_get_mutex(estadoExec));
        pcb = cpu_adapter_recibir_pcb_actualizado_de_cpu(pcb, cpuResponse, kernelConfig, kernelLogger);
        list_remove(estado_get_list(estadoExec), 0);
        pthread_mutex_unlock(estado_get_mutex(estadoExec));

        uint32_t realEjecutado = 0;
        realEjecutado = obtener_diferencial_de_tiempo_en_milisegundos(end, start);
        log_debug(kernelLogger, "PCB <ID %d> estuvo en ejecución por %d miliseconds", pcb_get_pid(pcb), realEjecutado);

        switch (cpuResponse) {
            case HEADER_proceso_desalojado:
                actualizar_pcb_por_desalojo(pcb, realEjecutado);
                pcb_set_estado_actual(pcb, READY);
                estado_encolar_pcb_atomic(estadoReady, pcb);
                log_transition("EXEC", "READY", pcb_get_pid(pcb));
                sem_post(estado_get_sem(estadoReady));
                break;
            case HEADER_proceso_terminado:
                pcb_set_estado_actual(pcb, EXIT);
                estado_encolar_pcb_atomic(estadoExit, pcb);
                log_transition("EXEC", "EXIT", pcb_get_pid(pcb));
                pcb_responder_a_consola(pcb, HEADER_proceso_terminado);
                sem_post(estado_get_sem(estadoExit));
                break;
            case HEADER_proceso_bloqueado:
                actualizar_pcb_por_bloqueo(pcb, realEjecutado, kernel_config_get_alfa(kernelConfig));
                atender_bloqueo(pcb);
                break;
            default:
                log_error(kernelLogger, "Error al recibir mensaje de CPU");
                break;
        }

        sem_post(&dispatchPermitido);
    }
}

static void noreturn planificador_corto_plazo(void) {
    pthread_t atenderPCBThread;
    pthread_create(&atenderPCBThread, NULL, (void*)atender_pcb, NULL);
    pthread_detach(atenderPCBThread);

    for (;;) {
        sem_wait(estado_get_sem(estadoReady));
        log_info(kernelLogger, "Se toma una instancia de READY");

        evaluar_suspension();

        sem_wait(&dispatchPermitido);
        log_info(kernelLogger, "Se permite dispatch");

        t_pcb* pcbToDispatch = elegir_pcb(estadoReady, kernel_config_get_alfa(kernelConfig));

        estado_encolar_pcb_atomic(estadoExec, pcbToDispatch);
        sem_post(estado_get_sem(estadoExec));
    }
}

void* encolar_en_new_a_nuevo_pcb_entrante(void* socket) {
    int* socketProceso = (int*)socket;
    uint32_t tamanio = 0;

    uint8_t response = stream_recv_header(*socketProceso);
    if (response != HANDSHAKE_consola) {
        log_error(kernelLogger, "Error al intentar establecer conexión con proceso mediante <socket %d>", *socketProceso);
    } else {
        t_buffer* bufferHandshakeInicial = buffer_create();
        stream_recv_buffer(*socketProceso, bufferHandshakeInicial);
        buffer_unpack(bufferHandshakeInicial, &tamanio, sizeof(tamanio));
        buffer_destroy(bufferHandshakeInicial);
        stream_send_empty_buffer(*socketProceso, HANDSHAKE_ok_continue);  // TODO: Descomentarlo en consola.c luego en producción

        uint8_t consolaResponse = stream_recv_header(*socketProceso);
        if (consolaResponse != HEADER_lista_instrucciones) {
            log_error(kernelLogger, "Error al intentar recibir lista de instrucciones del proceso mediante <socket %d>", *socketProceso);
            return NULL;
        }

        t_buffer* instructionsBuffer = buffer_create();
        stream_recv_buffer(*socketProceso, instructionsBuffer);
        t_buffer* instructionsBufferCopy = buffer_create_copy(instructionsBuffer);

        uint32_t newPid = obtener_siguiente_pid();
        t_pcb* newPcb = pcb_create(newPid, tamanio, kernel_config_get_est_inicial(kernelConfig));
        pcb_set_socket(newPcb, socketProceso);
        pcb_set_instruction_buffer(newPcb, instructionsBufferCopy);

        log_info(kernelLogger, "Creación de nuevo proceso ID %d de tamaño %d mediante <socket %d>", pcb_get_pid(newPcb), tamanio, *socketProceso);

        estado_encolar_pcb_atomic(estadoNew, newPcb);
        log_transition("NULL", "NEW", pcb_get_pid(newPcb));
        sem_post(&hayPcbsParaAgregarAlSistema);
        buffer_destroy(instructionsBuffer);
    }
    return NULL;
}

void inicializar_estructuras(void) {
    if (kernel_config_es_algoritmo_srt(kernelConfig)) {
        elegir_pcb = elegir_pcb_segun_srt;
        evaluar_suspension = __evaluar_suspension_segun_srt;
        actualizar_pcb_por_bloqueo = actualizar_pcb_por_bloqueo_segun_srt;
    } else if (kernel_config_es_algoritmo_fifo(kernelConfig)) {
        elegir_pcb = elegir_pcb_segun_fifo;
        evaluar_suspension = __evaluar_suspension_segun_fifo;
        actualizar_pcb_por_bloqueo = __actualizar_pcb_por_bloqueo_segun_fifo;
    } else {
        log_error(kernelLogger, "No se pudo inicializar el planificador, no se encontró un algoritmo de planificación válido");
        exit(-1);
    }

    nextPid = 0;
    hayQueDesalojar = false;

    pthread_mutex_init(&nextPidMutex, NULL);
    pthread_mutex_init(&hayQueDesalojarMutex, NULL);
    pthread_mutex_init(&mutexSocketMemoria, NULL);

    int valorInicialGradoMultiprog = kernel_config_get_grado_multiprogramacion(kernelConfig);

    sem_init(&hayPcbsParaAgregarAlSistema, 0, 0);
    sem_init(&gradoMultiprog, 0, valorInicialGradoMultiprog);
    sem_init(&dispatchPermitido, 0, 1);
    sem_init(&transicionarSusreadyAReady, 0, 0);
    log_info(kernelLogger, "Se inicializa el grado multiprogramación en %d", valorInicialGradoMultiprog);

    estadoNew = estado_create(NEW);
    estadoReady = estado_create(READY);
    estadoExec = estado_create(EXEC);
    estadoExit = estado_create(EXIT);
    estadoBlocked = estado_create(BLOCKED);
    estadoSuspendedBlocked = estado_create(SUSPENDED_BLOCKED);
    estadoSuspendedReady = estado_create(SUSPENDED_READY);

    pcbsEsperandoParaIO = estado_create(PCBS_ESPERANDO_PARA_IO);

    pthread_t largoPlazoTh;
    pthread_create(&largoPlazoTh, NULL, (void*)planificador_largo_plazo, NULL);
    pthread_detach(largoPlazoTh);

    pthread_t medianoPlazoTh;
    pthread_create(&medianoPlazoTh, NULL, (void*)planificador_mediano_plazo, NULL);
    pthread_detach(medianoPlazoTh);

    pthread_t cortoPlazoTh;
    pthread_create(&cortoPlazoTh, NULL, (void*)planificador_corto_plazo, NULL);
    pthread_detach(cortoPlazoTh);

    pthread_t dispositivoIOTh;
    pthread_create(&dispositivoIOTh, NULL, (void*)iniciar_dispositivo_io, NULL);
    pthread_detach(dispositivoIOTh);

    log_info(kernelLogger, "Se crean los hilos planificadores");
}
