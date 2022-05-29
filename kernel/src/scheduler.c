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

extern t_log* kernelLogger;
extern t_kernel_config* kernelConfig;

static t_algoritmo elegir_pcb;

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

static void responder_no_hay_lugar_en_memoria(t_pcb* pcb) {
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

//////////////////////////////////////// Algorithms ////////////////////////////////////////
t_pcb* segun_fifo(t_estado* estado, double _) {
    return estado_desencolar_primer_pcb_atomic(estado);
}

t_pcb* menor_rafaga_restante(t_pcb* unPcb, t_pcb* otroPcb) {
    double unaRafagaRestante = pcb_get_estimacion_actual(unPcb) - pcb_get_reales_ejecutados_hasta_ahora(unPcb);
    double otraRafagaRestante = pcb_get_estimacion_actual(otroPcb) - pcb_get_reales_ejecutados_hasta_ahora(otroPcb);
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

t_pcb* segun_srt(t_estado* estado, double alfa) {
    t_pcb* pcbElecto = NULL;
    pthread_mutex_lock(estado_get_mutex(estado));
    int cantidadPcbsEnLista = list_size(estado_get_list(estado));
    if (cantidadPcbsEnLista == 1) {
        pcbElecto = estado_desencolar_primer_pcb(estado);
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
            // TODO: mem_adapter_avisar_suspension(pcbASuspender, kernelConfig, kernelLogger);
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
            pthread_mutex_lock(&hayQueDesalojarMutex);
            hayQueDesalojar = true;
            pthread_mutex_unlock(&hayQueDesalojarMutex);
            estado_encolar_pcb_atomic(estadoReady, pcbAEjecutarRafagasIO);
            pcb_set_tiempo_de_bloqueo(pcbAEjecutarRafagasIO, 0);
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
        // TODO: mem_adapter_finalizar_proceso(pcbALiberar, kernelConfig, kernelLogger);
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

            pthread_mutex_lock(&hayQueDesalojarMutex);
            hayQueDesalojar = true;
            pthread_mutex_unlock(&hayQueDesalojarMutex);

            if (nuevaTablaPagina == -1) {
                responder_no_hay_lugar_en_memoria(pcbQuePasaAReady);
            } else {
                estado_encolar_pcb_atomic(estadoReady, pcbQuePasaAReady);
                sem_post(estado_get_sem(estadoReady));
                log_transition("NEW", "READY", pcb_get_pid(pcbQuePasaAReady));
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

        int nuevaTablaPagina = mem_adapter_avisar_reactivacion(pcbQuePasaAReady, kernelConfig, kernelLogger);
        pcb_set_tabla_pagina_primer_nivel(pcbQuePasaAReady, nuevaTablaPagina);
        if (nuevaTablaPagina == -1) {
            responder_no_hay_lugar_en_memoria(pcbQuePasaAReady);
        } else {
            pcb_set_estado_actual(pcbQuePasaAReady, READY);
            pthread_mutex_lock(&hayQueDesalojarMutex);
            hayQueDesalojar = true;
            pthread_mutex_unlock(&hayQueDesalojarMutex);
            estado_encolar_pcb_atomic(estadoReady, pcbQuePasaAReady);
            sem_post(estado_get_sem(estadoReady));
            log_transition("SUSREADY", "READY", pcb_get_pid(pcbQuePasaAReady));
        }

        pcbQuePasaAReady = NULL;
    }
}

void actualizar_pcb_por_desalojo(t_pcb* pcb, double realEjecutado) {
    pcb_set_reales_ejecutados_hasta_ahora(pcb, pcb_get_reales_ejecutados_hasta_ahora(pcb) + realEjecutado);
}

void actualizar_pcb_por_bloqueo(t_pcb* pcb, double realEjecutado) {
    pcb_set_real_anterior(pcb, pcb_get_reales_ejecutados_hasta_ahora(pcb) + realEjecutado);  // Digo que el real anterior es la sumatoria de los reales ejecutados hasta ahora, antes de bloquearse
    pcb_set_reales_ejecutados_hasta_ahora(pcb, 0);                                           // Resetear reales ejecutados debido a que ya ejecutaron toda su ráfaga de CPU
    double siguienteEstimacion = calcular_siguiente_estimacion(pcb, kernel_config_get_alfa(kernelConfig));
    pcb_set_estimacion_actual(pcb, siguienteEstimacion);  // Establezco la nueva estimación para cuando me desbloquee (estimamos antes por conveniencia)
}

static void atender_bloqueo(t_pcb* pcb) {
    pcb_marcar_tiempo_inicial_bloqueado(pcb);
    estado_encolar_pcb_atomic(pcbsEsperandoParaIO, pcb);
    sem_post(estado_get_sem(pcbsEsperandoParaIO));
    pcb_set_estado_actual(pcb, BLOCKED);
    estado_encolar_pcb_atomic(estadoBlocked, pcb);
    log_transition("EXEC", "BLOCKED", pcb_get_pid(pcb));
    log_info(kernelLogger, "PCB <ID %d> ingresa a la cola de espera de I/O", pcb_get_pid(pcb));
    pthread_t* contadorASuspendedBlocked = malloc(sizeof(*contadorASuspendedBlocked));
    pthread_create(contadorASuspendedBlocked, NULL, (void*)iniciar_contador_blocked_a_suspended_blocked, (void*)pcb);
    pthread_detach(*contadorASuspendedBlocked);
}

/*
    struct timespec start, end;
    clock_gettime(CLOCK_REALTIME, &start);
    intervalo_de_pausa(10500);
    clock_gettime(CLOCK_REALTIME, &end);

    uint64_t delta_us = (end.tv_sec - start.tv_sec) * 1000 + (end.tv_nsec - start.tv_nsec) / 1000000;
    char* str_delta_us = string_itoa(delta_us);
    str_delta_us[strlen(str_delta_us) - 1] = '0';
    uint64_t new_delta_us = atoi(str_delta_us);

    printf("\nElapsed time: %ld\n", delta_us);
    printf("Elapsed time (new): %ld\n", new_delta_us);

    free(str_delta_us);
*/

static void noreturn atender_pcb(void) {
    for (;;) {
        sem_wait(estado_get_sem(estadoExec));

        struct timespec start;
        if (clock_gettime(CLOCK_REALTIME, &start) == -1) {
            perror("clock_gettime");
            exit(EXIT_FAILURE);
        }

        // time_t tiempoInicialEjecucion = time(NULL);
        pthread_mutex_lock(estado_get_mutex(estadoExec));
        t_pcb* pcb = list_get(estado_get_list(estadoExec), 0);
        log_transition("READY", "EXEC", pcb_get_pid(pcb));
        pthread_mutex_unlock(estado_get_mutex(estadoExec));

        cpu_adapter_enviar_pcb_a_cpu(pcb, kernelConfig, kernelLogger);
        uint8_t cpuResponse = stream_recv_header(kernel_config_get_socket_dispatch_cpu(kernelConfig));

        struct timespec end;
        if (clock_gettime(CLOCK_REALTIME, &end) == -1) {
            perror("clock_gettime");
            exit(EXIT_FAILURE);
        }
        // time_t tiempoFinalEjecucion = time(NULL);
        pthread_mutex_lock(estado_get_mutex(estadoExec));
        pcb = cpu_adapter_recibir_pcb_actualizado_de_cpu(pcb, cpuResponse, kernelConfig, kernelLogger);
        // pcb_set_real_anterior(pcb, difftime(tiempoFinalEjecucion, tiempoInicialEjecucion));  // TODO: Esto está mal, pues en caso de desalojar a un proceso no debe actualizar su real anterior ejecutado (solo actualizar realesEjecutadosHastaAhora)
        list_remove(estado_get_list(estadoExec), 0);
        pthread_mutex_unlock(estado_get_mutex(estadoExec));

        // double realEjecutado = difftime(tiempoFinalEjecucion, tiempoInicialEjecucion);
        uint64_t realEjecutado = (end.tv_sec - start.tv_sec) * 1000 + (end.tv_nsec - start.tv_nsec) / 1000000;
        log_debug(kernelLogger, "Elapsed time: %ld miliseconds", realEjecutado);
        /* char* strRealEjecutado = string_itoa(realEjecutado);
        strRealEjecutado[strlen(strRealEjecutado) - 1] = '0';
        uint64_t nuevoRealEjecutado = atoi(strRealEjecutado);
        log_debug(kernelLogger, "Elapsed time (new): %ld", nuevoRealEjecutado); */

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
                // TODO: mem_adapter_finalizar_proceso(pcb, kernelConfig, kernelLogger);
                pcb_responder_a_consola(pcb, HEADER_proceso_terminado);
                sem_post(estado_get_sem(estadoExit));
                break;
            case HEADER_proceso_bloqueado:
                actualizar_pcb_por_bloqueo(pcb, realEjecutado);
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

        if (kernel_config_es_algoritmo_srt(kernelConfig)) {
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
        pcb_set_socket(newPcb, *socketProceso);
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
    nextPid = 0;
    hayQueDesalojar = false;

    pthread_mutex_init(&nextPidMutex, NULL);
    pthread_mutex_init(&hayQueDesalojarMutex, NULL);

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

    if (kernel_config_es_algoritmo_srt(kernelConfig)) {
        elegir_pcb = segun_srt;
    } else if (kernel_config_es_algoritmo_fifo(kernelConfig)) {
        elegir_pcb = segun_fifo;
    } else {
        log_error(kernelLogger, "No se pudo inicializar el planificador, no se encontró un algoritmo de planificación válido");
        exit(-1);
    }

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
