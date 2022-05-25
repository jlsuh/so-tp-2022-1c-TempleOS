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
#include "estados.h"
#include "instruccion.h"
#include "kernel_config.h"
#include "mem_adapter.h"
#include "pcb.h"
#include "stream.h"

extern t_log* kernelLogger;
extern t_kernel_config* kernelConfig;

static uint32_t nextPid;
static uint32_t tiempoInicioExec;
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

static void noreturn planificador_largo_plazo(void);
// static void noreturn planificador_mediano_plazo(void);
static void noreturn planificador_corto_plazo(void);
static void noreturn iniciar_dispositivo_io(void);
static void noreturn planificador_mediano_plazo(void);

static uint32_t get_next_pid(void) {
    pthread_mutex_lock(&nextPidMutex);
    uint32_t newNextPid = nextPid++;
    pthread_mutex_unlock(&nextPidMutex);
    return newNextPid;
}

static void log_transition(const char* prev, const char* post, int pid) {
    char* transicion = string_from_format("\e[1;93m%s->%s\e[0m", prev, post);
    log_info(kernelLogger, "Transición de %s PCB <ID %d>", transicion, pid);
    free(transicion);
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

    pthread_t largoPlazoTh;
    pthread_create(&largoPlazoTh, NULL, (void*)planificador_largo_plazo, NULL);
    pthread_detach(largoPlazoTh);

    pthread_t cortoPlazoTh;
    pthread_create(&cortoPlazoTh, NULL, (void*)planificador_corto_plazo, NULL);
    pthread_detach(cortoPlazoTh);

    pthread_t medianoPlazoTh;
    pthread_create(&medianoPlazoTh, NULL, (void*)planificador_mediano_plazo, NULL);
    pthread_detach(medianoPlazoTh);

    pthread_t dispositivoIOTh;
    pthread_create(&dispositivoIOTh, NULL, (void*)iniciar_dispositivo_io, NULL);
    pthread_detach(dispositivoIOTh);

    log_info(kernelLogger, "Se crean los hilos planificadores");
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

        uint32_t newPid = get_next_pid();
        t_pcb* newPcb = pcb_create(newPid, tamanio, kernel_config_get_est_inicial(kernelConfig));
        pcb_set_socket(newPcb, *socketProceso);
        pcb_set_instruction_buffer(newPcb, instructionsBufferCopy);

        log_info(kernelLogger, "Creación de nuevo proceso ID %d de tamaño %d mediante <socket %d>", pcb_get_pid(newPcb), tamanio, *socketProceso);

        t_list* listaInstrucciones = instruccion_list_create_from_buffer(instructionsBuffer, kernelLogger);
        pcb_set_instrucciones(newPcb, listaInstrucciones);

        estado_encolar_pcb(estadoNew, newPcb);
        log_transition("NULL", "NEW", pcb_get_pid(newPcb));
        sem_post(&hayPcbsParaAgregarAlSistema);
        buffer_destroy(instructionsBuffer);
    }
    return NULL;
}

static void noreturn liberar_pcbs_en_exit(void) {
    for (;;) {
        sem_wait(estado_get_sem(estadoExit));
        t_pcb* pcbALiberar = estado_desencolar_primer_pcb(estadoExit);
        // TODO: mem_adapter_finalizar_proceso(pcbALiberar, kernelConfig, kernelLogger);
        pcb_destroy(pcbALiberar);
        sem_post(&gradoMultiprog);
    }
}

void responder_no_hay_lugar_en_memoria(t_pcb* pcb) {
    estado_encolar_pcb(estadoExit, pcb);
    log_transition("NEW", "EXIT", pcb_get_pid(pcb));
    log_error(kernelLogger, "Memoria insuficiente para alojar el proceso %d", pcb_get_pid(pcb));
    pcb_responder_a_consola(pcb, HEADER_memoria_insuficiente);
}

static void noreturn planificador_largo_plazo(void) {
    pthread_t liberarPcbsEnExitTh;
    pthread_create(&liberarPcbsEnExitTh, NULL, (void*)liberar_pcbs_en_exit, NULL);
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
                estado_encolar_pcb(estadoReady, pcbQuePasaAReady);
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
            estado_encolar_pcb(estadoReady, pcbQuePasaAReady);
            sem_post(estado_get_sem(estadoReady));
            log_transition("SUSREADY", "READY", pcb_get_pid(pcbQuePasaAReady));
        }

        pthread_mutex_lock(&hayQueDesalojarMutex);
        hayQueDesalojar = true;
        pthread_mutex_unlock(&hayQueDesalojarMutex);
        pcbQuePasaAReady = NULL;
    }
}

// TODO: Abstraerlo en un puntero a la función del pcb con la idea de: t_pcb* pcb = scheduler_elegir_segun_algoritmo(estadoReady)
/*
Declaración: t_pcb* (*scheduler_elegir_segun_algoritmo)(t_estado* estadoReady)

En inicializar_estructuras se pondría un if leyendo si es SJF o FIFO:
if(kernel_config_get_algoritmo_planificacion(kernelConfig) == SJF){
    scheduler_elegir_segun_algoritmo = scheduler_elegir_segun_sjf;
} else if(kernel_config_get_algoritmo_planificacion(kernelConfig) == FIFO){
    scheduler_elegir_segun_algoritmo = scheduler_elegir_segun_fifo;
} else {
    log_error(kernelLogger, "Algoritmo de planificación no reconocido");
    exit(-1);
}

Tanto scheduler_elegir_segun_sjf y scheduler_elegir_segun_fifo deben tener la misma declaración que scheduler_elejir_segun_algoritmo:
t_pcb* (*scheduler_elegir_segun_algoritmo)(t_estado* estadoReady);
t_pcb* (*scheduler_elegir_segun_sjf)(t_estado* estadoReady);
*/

t_pcb* elegir_segun_algoritmo(void) {
    t_pcb* seleccionado = NULL;
    int alfa = kernel_config_get_alfa(kernelConfig);

    void* t_planificar_srt(t_pcb * p1, t_pcb * p2) {
        return (pcb_estimar_srt(p1, alfa) <= pcb_estimar_srt(p2, alfa)) ? p1 : p2;
    }
    bool t_encontrar_seleccionado(t_pcb * p1, t_pcb * p2) {
        return (pcb_get_pid((t_pcb*)seleccionado) == pcb_get_pid(seleccionado));
    }

    if (strcmp(kernel_config_get_algoritmo(kernelConfig), "SRT") == 0) {
        if (list_size(estado_get_list(estadoReady)) == 1) {
            seleccionado = list_remove(estado_get_list(estadoReady), 0);
            pcb_estimar_srt(seleccionado, alfa);  // Si la lista tiene 1 solo elemento rompe el get_minimum asi que lo busco por lugar 0 y estimo el próximo srt
            return seleccionado;
        }
        seleccionado = list_get_minimum(estado_get_list(estadoReady), (void*)t_planificar_srt);
        seleccionado = list_remove_by_condition(estado_get_list(estadoReady), (void*)t_encontrar_seleccionado);  // Tengo que hacer el remove porque lo anterior es un get
        return seleccionado;
    }
    return list_remove(estado_get_list(estadoReady), 0);  // FIFO
}

double timestamp(void) {
    struct timespec tiempo;
    clock_gettime(CLOCK_MONOTONIC, &tiempo);
    double tiempo_en_ms = tiempo.tv_sec * 1000 + tiempo.tv_nsec / 1000000;
    return tiempo_en_ms;
}

static void noreturn iniciar_dispositivo_io(void) {
    for (;;) {
        sem_wait(estado_get_sem(pcbsEsperandoParaIO));
        t_pcb* pcbAEjecutarRafagasIO = estado_desencolar_primer_pcb(pcbsEsperandoParaIO);
        log_info(kernelLogger, "Ejecutando ráfagas I/O de PCB <ID %d> por %d milisegundos", pcb_get_pid(pcbAEjecutarRafagasIO), pcb_get_tiempo_de_bloqueo(pcbAEjecutarRafagasIO));
        pthread_mutex_lock(pcb_get_mutex(pcbAEjecutarRafagasIO));
        if (!pcb_get_tiempo_final_bloqueado_setteado(pcbAEjecutarRafagasIO)) {
            time_t tiempoFinal;
            time(&tiempoFinal);
            pcb_set_tiempo_final_bloqueado(pcbAEjecutarRafagasIO, tiempoFinal);
            pcb_set_tiempo_final_bloqueado_setteado(pcbAEjecutarRafagasIO, true);
        }
        pthread_mutex_unlock(pcb_get_mutex(pcbAEjecutarRafagasIO));

        intervalo_de_pausa(pcb_get_tiempo_de_bloqueo(pcbAEjecutarRafagasIO));

        pthread_mutex_lock(pcb_get_mutex(pcbAEjecutarRafagasIO));
        if (pcb_get_estado_actual(pcbAEjecutarRafagasIO) == BLOCKED) {
            estado_remover_pcb_de_cola(estadoBlocked, pcbAEjecutarRafagasIO);
            pcb_set_estado_actual(pcbAEjecutarRafagasIO, READY);
            estado_encolar_pcb(estadoReady, pcbAEjecutarRafagasIO);
            pcb_set_tiempo_de_bloqueo(pcbAEjecutarRafagasIO, 0);
            log_transition("BLOCKED", "READY", pcb_get_pid(pcbAEjecutarRafagasIO));
            sem_post(estado_get_sem(estadoReady));
        } else if (pcb_get_estado_actual(pcbAEjecutarRafagasIO) == SUSPENDED_BLOCKED) {
            estado_remover_pcb_de_cola(estadoSuspendedBlocked, pcbAEjecutarRafagasIO);
            pcb_set_estado_actual(pcbAEjecutarRafagasIO, SUSPENDED_READY);
            estado_encolar_pcb(estadoSuspendedReady, pcbAEjecutarRafagasIO);
            pcb_set_tiempo_de_bloqueo(pcbAEjecutarRafagasIO, 0);
            log_transition("SUSBLOCKED", "SUSREADY", pcb_get_pid(pcbAEjecutarRafagasIO));
            sem_post(&hayPcbsParaAgregarAlSistema);
        }
        pcb_set_tiempo_final_bloqueado_setteado(pcbAEjecutarRafagasIO, false);
        pthread_mutex_unlock(pcb_get_mutex(pcbAEjecutarRafagasIO));
    }
}

bool es_este_pcb_por_pid(void* unPcb, void* otroPcb) {
    return pcb_get_pid((t_pcb*)unPcb) == pcb_get_pid((t_pcb*)otroPcb);
}

static void iniciar_contador_blocked_a_suspended_blocked(void* pcbVoid) {
    t_pcb* pcb = (t_pcb*)pcbVoid;
    log_error(kernelLogger, "Iniciando contador de blocked a suspended blocked de PCB <ID %d>", pcb_get_pid(pcb));
    t_pcb* dummyPcb = pcb_create(pcb_get_pid(pcb), pcb_get_tamanio(pcb), pcb_get_est_actual(pcb));

    intervalo_de_pausa(kernel_config_get_tiempo_maximo_bloqueado(kernelConfig));

    if (estado_remover_pcb_de_cola(estadoBlocked, dummyPcb) != NULL) {
        pthread_mutex_lock(pcb_get_mutex(pcb));
        if (!pcb_get_tiempo_final_bloqueado_setteado(pcb)) {
            time_t tiempoFinal;
            time(&tiempoFinal);
            pcb_set_tiempo_final_bloqueado(pcb, tiempoFinal);
            pcb_set_tiempo_final_bloqueado_setteado(pcb, true);
        }
        double tiempoEsperandoEnBlocked = difftime(pcb_get_tiempo_final_bloqueado(pcb), pcb_get_tiempo_inicial_bloqueado(pcb));
        if (pcb_get_tiempo_de_bloqueo(pcb) <= kernel_config_get_tiempo_maximo_bloqueado(kernelConfig)) {
            if (pcb_get_tiempo_de_bloqueo(pcb) / 1000.0 + tiempoEsperandoEnBlocked <= kernel_config_get_tiempo_maximo_bloqueado(kernelConfig) / 1000.0) {
                // Como el tiempo de bloqueo total es menor ó igual al tiempo máximo tolerable de bloqueo, NO se suspende el proceso y se transiciona de BLOCKED a READY
                pcb_destroy(dummyPcb);
                pthread_mutex_unlock(pcb_get_mutex(pcb));
                return;
            }
        }

        pcb_set_estado_actual(pcb, SUSPENDED_BLOCKED);
        // TODO: mem_adapter_avisar_suspension(pcbASuspender, kernelConfig, kernelLogger);
        estado_encolar_pcb(estadoSuspendedBlocked, pcb);
        log_error(kernelLogger, "Entra en suspensión PCB <ID %d>", pcb_get_pid(pcb));
        log_transition("BLOCKED", "SUSBLOCKED", pcb_get_pid(pcb));
        sem_post(&gradoMultiprog);

        pthread_mutex_unlock(pcb_get_mutex(pcb));
    }

    pcb_destroy(dummyPcb);
}

static void atender_bloqueo(t_pcb* pcb) {
    time_t tiempoInicial;
    time(&tiempoInicial);
    pcb_set_tiempo_inicial_bloqueado(pcb, tiempoInicial);
    estado_encolar_pcb(pcbsEsperandoParaIO, pcb);
    sem_post(estado_get_sem(pcbsEsperandoParaIO));
    pcb_set_estado_actual(pcb, BLOCKED);
    estado_encolar_pcb(estadoBlocked, pcb);
    log_transition("EXEC", "BLOCKED", pcb_get_pid(pcb));
    log_info(kernelLogger, "PCB <ID %d> ingresa a la cola de espera de I/O", pcb_get_pid(pcb));
    pthread_t* contadorASuspendedBlocked = malloc(sizeof(*contadorASuspendedBlocked));
    pthread_create(contadorASuspendedBlocked, NULL, (void*)iniciar_contador_blocked_a_suspended_blocked, (void*)pcb);
    pthread_detach(*contadorASuspendedBlocked);
    pcb_set_hilo_contador(pcb, contadorASuspendedBlocked);
}

static void noreturn atender_pcb(void) {
    for (;;) {
        sem_wait(estado_get_sem(estadoExec));

        pthread_mutex_lock(estado_get_mutex(estadoExec));
        t_pcb* pcb = list_get(estado_get_list(estadoExec), 0);
        pthread_mutex_unlock(estado_get_mutex(estadoExec));

        cpu_adapter_enviar_pcb_a_cpu(pcb, kernelConfig, kernelLogger);
        uint8_t cpuResponse = stream_recv_header(kernel_config_get_socket_dispatch_cpu(kernelConfig));

        pthread_mutex_lock(estado_get_mutex(estadoExec));
        pcb = cpu_adapter_recibir_pcb_actualizado_de_cpu(pcb, cpuResponse, kernelConfig, kernelLogger);
        pcb_set_ultima_ejecucion(pcb, timestamp() - tiempoInicioExec);
        list_remove(estado_get_list(estadoExec), 0);
        pthread_mutex_unlock(estado_get_mutex(estadoExec));

        switch (cpuResponse) {
            case HEADER_proceso_desalojado:
                pcb_set_estado_actual(pcb, READY);
                estado_encolar_pcb(estadoReady, pcb);
                log_transition("EXEC", "READY", pcb_get_pid(pcb));
                sem_post(estado_get_sem(estadoReady));
                break;
            case HEADER_proceso_terminado:
                pcb_set_estado_actual(pcb, EXIT);
                estado_encolar_pcb(estadoExit, pcb);
                log_transition("EXEC", "EXIT", pcb_get_pid(pcb));
                // TODO: mem_adapter_finalizar_proceso(pcb, kernelConfig, kernelLogger);
                pcb_responder_a_consola(pcb, HEADER_proceso_terminado);
                sem_post(estado_get_sem(estadoExit));
                break;
            case HEADER_proceso_bloqueado:
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

        pthread_mutex_lock(estado_get_mutex(estadoReady));
        t_pcb* pcbToDispatch = elegir_segun_algoritmo();
        pthread_mutex_unlock(estado_get_mutex(estadoReady));

        estado_encolar_pcb(estadoExec, pcbToDispatch);
        tiempoInicioExec = timestamp();
        log_transition("READY", "EXEC", pcb_get_pid(pcbToDispatch));

        sem_post(estado_get_sem(estadoExec));
    }
}
