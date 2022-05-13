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
#include "cpu_adapter.h"
#include "estados.h"
#include "instruccion.h"
#include "kernel_config.h"
#include "mem_adapter.h"
#include "pcb.h"
#include "stream.h"

extern t_log* kernelLogger;
extern t_kernel_config* kernelConfig;
extern t_log* kernelLogger;

static uint32_t nextPid;
static uint32_t tiempoInicioExec;
static pthread_mutex_t nextPidMutex;

static sem_t gradoMultiprog;
static sem_t hayPcbsParaAgregarAlSistema;
static sem_t dispatchPermitido;
static sem_t sePuedeInterrumpirCPU;
static sem_t ejecutaMedianoPlazo;

static t_estado* estadoNew;
static t_estado* estadoReady;
static t_estado* estadoExec;
static t_estado* estadoExit;
static t_estado* estadoBlocked;
static t_estado* estadoSuspendedBlocked;
static t_estado* estadoSuspendedReady;

static bool hayQueDesalojar;
pthread_mutex_t hayQueDesalojarMutex;

static void noreturn planificador_largo_plazo(void);
static void noreturn planificador_corto_plazo(void);

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
    sem_init(&sePuedeInterrumpirCPU, 0, 1);
    sem_init(&ejecutaMedianoPlazo, 0, 1);
    log_info(kernelLogger, "Se inicializa el grado multiprogramación en %d", valorInicialGradoMultiprog);

    estadoNew = estado_create(NEW);
    estadoReady = estado_create(READY);
    estadoExec = estado_create(EXEC);
    estadoExit = estado_create(EXIT);
    estadoBlocked = estado_create(BLOCKED);
    estadoSuspendedBlocked = estado_create(SUSPENDED_BLOCKED);
    estadoSuspendedReady = estado_create(SUSPENDED_READY);

    pthread_t largoPlazoTh;
    pthread_create(&largoPlazoTh, NULL, (void*)planificador_largo_plazo, NULL);
    pthread_detach(largoPlazoTh);

    pthread_t cortoPlazoTh;
    pthread_create(&cortoPlazoTh, NULL, (void*)planificador_corto_plazo, NULL);
    pthread_detach(cortoPlazoTh);

    /*
    pthread_t medianoPlazoTh;
    pthread_create(&medianoPlazoTh, NULL, (void*)planificador_mediano_plazo, NULL);
    pthread_detach(medianoPlazoTh);
    */
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
        stream_send_empty_buffer(*socketProceso, HANDSHAKE_ok_continue);

        uint8_t consolaResponse = stream_recv_header(*socketProceso);  // Header lista de instrucciones solamentet_buffer* instructionsBuffer = buffer_create();
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

        log_info(kernelLogger, "Se establece Handshake inicial con proceso ID %d de tamaño %d mediante <socket %d>", pcb_get_pid(newPcb), tamanio, *socketProceso);

        t_list* listaInstrucciones = instruccion_list_create_from_buffer(instructionsBuffer, kernelLogger);
        printf(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>><\n");
        pcb_set_instrucciones(newPcb, listaInstrucciones);

        estado_encolar_pcb(estadoNew, newPcb);
        sem_post(&hayPcbsParaAgregarAlSistema);
        buffer_destroy(instructionsBuffer);
    }
    return NULL;
}

static void noreturn liberar_pcbs_en_exit(void) {
    for (;;) {
        sem_wait(estado_get_sem(estadoExit));
        t_pcb* pcbALiberar = estado_desencolar_primer_pcb(estadoExit);
        // Acá invocar algo para comunicarse con memoria para que desasigne recursos del proceso en exit
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

    int nuevaTablaPagina;
    t_pcb* pcbQuePasaAReady = NULL;
    char* prevStatus = NULL;
    for (;;) {
        sem_wait(&hayPcbsParaAgregarAlSistema);
        sem_wait(&gradoMultiprog);
        if (list_size(estado_get_list(estadoSuspendedReady)) > 0) {  // Prioridad a procesos en SUSREADY por sobre NEW
            // TODO: Transicionar de SUSREADY a READY no debería ser una responsabilidad del planificador a largo plazo, sino del mediano plazo
            pthread_mutex_lock(estado_get_mutex(estadoSuspendedReady));
            pcbQuePasaAReady = list_remove(estado_get_list(estadoSuspendedReady), 0);
            pthread_mutex_unlock(estado_get_mutex(estadoSuspendedReady));
            prevStatus = string_from_format("SUSREADY");

            pthread_mutex_lock(&hayQueDesalojarMutex);
            hayQueDesalojar = true;
            pthread_mutex_unlock(&hayQueDesalojarMutex);
        } else {
            pthread_mutex_lock(estado_get_mutex(estadoNew));
            pcbQuePasaAReady = list_remove(estado_get_list(estadoNew), 0);
            pthread_mutex_unlock(estado_get_mutex(estadoNew));
            nuevaTablaPagina = mem_adapter_obtener_tabla_pagina(pcbQuePasaAReady, kernelConfig, kernelLogger);
            pcb_set_tabla_pagina_primer_nivel(pcbQuePasaAReady, nuevaTablaPagina);
            prevStatus = string_from_format("NEW");

            pthread_mutex_lock(&hayQueDesalojarMutex);
            hayQueDesalojar = true;
            pthread_mutex_unlock(&hayQueDesalojarMutex);
        }
        if (nuevaTablaPagina == -1) {
            responder_no_hay_lugar_en_memoria(pcbQuePasaAReady);
        } else {
            estado_encolar_pcb(estadoReady, pcbQuePasaAReady);
            sem_post(estado_get_sem(estadoReady));
            log_transition(prevStatus, "READY", pcb_get_pid(pcbQuePasaAReady));
        }
        free(prevStatus);
        prevStatus = NULL;
        pcbQuePasaAReady = NULL;
    }
}

static void noreturn atender_suspended_ready_a_ready(void) {
    int nuevaTablaPagina;
    t_pcb* pcbQuePasaAReady = NULL;

    for (;;) {
        sem_wait(estado_get_sem(estadoSuspendedReady));
        sem_wait(&gradoMultiprog);
        if (list_size(estado_get_list(estadoSuspendedReady)) > 0) {  // Prioridad a procesos en SUSREADY por sobre NEW
            pthread_mutex_lock(estado_get_mutex(estadoSuspendedReady));
            pcbQuePasaAReady = list_remove(estado_get_list(estadoSuspendedReady), 0);
            pthread_mutex_unlock(estado_get_mutex(estadoSuspendedReady));

           /* nuevaTablaPagina = mem_adapter_avisar_reactivacion(pcbQuePasaAReady, kernelConfig, kernelLogger);
            pcb_set_tabla_pagina_primer_nivel(pcbQuePasaAReady, nuevaTablaPagina);*/
            log_transition("SUSPENDED_READY", "READY", pcb_get_pid(pcbQuePasaAReady));

            pthread_mutex_lock(&hayQueDesalojarMutex);
            hayQueDesalojar = true;
            pthread_mutex_unlock(&hayQueDesalojarMutex);
        } 
        //sem_post(&ejecutaMedianoPlazo); TODO: definir como dar prioridad
        pcbQuePasaAReady = NULL;
    }
}

static void noreturn planificador_mediano_plazo(void) {
    bool supera_limite_block(void* pcb_en_lista) {
        return (pcb_get_tiempo_de_bloqueo((t_pcb*)pcb_en_lista) > kernel_config_get_maximo_bloq(kernelConfig));
    }

    pthread_t atenderTransicionSuspendedReady;
    pthread_create(&atenderTransicionSuspendedReady, NULL, (void*)atender_suspended_ready_a_ready, NULL);
    pthread_detach(atenderTransicionSuspendedReady);

    t_pcb* pcbASuspender = NULL;

    for (;;) { // TRANSICION BLOCKED -> SUSPENDED BLOCKED
        sem_wait(estado_get_sem(estadoBlocked));
        if(list_size(estado_get_list(estadoBlocked)) > 0 && list_any_satisfy(estado_get_list(estadoBlocked),(void*)supera_limite_block)){
            pcbASuspender = list_remove_by_condition(estado_get_list(estadoBlocked),(void*)supera_limite_block);
            estado_encolar_pcb(estadoSuspendedBlocked, pcbASuspender);
            //mem_adapter_avisar_suspension(pcbASuspender, kernelConfig, kernelLogger);

            pcb_set_estado_actual(pcbASuspender, SUSPENDED_BLOCKED);
            log_transition("BLOCKED", "SUSPENDED_BLOCKED", pcb_get_pid(pcbASuspender));
        }
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

static void atender_bloqueo(void* args){
	t_pcb* pcb = (t_pcb*) args; // TODO: chequear esto si anda

    estado_encolar_pcb(estadoBlocked, pcb);
    pcb_set_estado_actual(pcb, BLOCKED);
    log_transition("EXEC", "BLOCKED", pcb_get_pid(pcb));
    sem_post(estado_get_sem(estadoBlocked));
    usleep(pcb_get_tiempo_de_bloqueo(pcb)*1000); 
    /* Supongo acá que si el t.espera es muy corto no puede causar que el sleep se haga entero antes que el mediano plazo lo pase
    suspenda, pero debido a que si el tiempo es corto como para que pase eso entonces es mas corto que el límite de suspensión
    por eso no meto ningún semáforo */
    pcb_set_tiempo_de_bloqueo(pcb, 0);

    if(pcb_get_estado_actual(pcb) == SUSPENDED_BLOCKED){
        pthread_mutex_lock(estado_get_mutex(estadoSuspendedReady));
        estado_encolar_pcb(estadoSuspendedReady, pcb);
        pthread_mutex_unlock(estado_get_mutex(estadoSuspendedReady));
        pcb_set_estado_actual(pcb, SUSPENDED_READY);
        log_transition("SUSPENDED_BLOCKED", "SUSPENDED_READY", pcb_get_pid(pcb));
        sem_post(estado_get_sem(estadoSuspendedReady));
    }
}

static void noreturn atender_pcb(void) {  // TEMPORALMENTE ACÁ, QUIZÁS SE MUEVA A OTRO ARCHIVO
    pthread_t hiloAtenderBloqueo; // TODO: Esto no rompe nada acá? 

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
        // Quizás meter un log acá de que cierto pcb volvió de CPU
        pthread_mutex_unlock(estado_get_mutex(estadoExec));

        switch (cpuResponse) {
            case HEADER_proceso_desalojado:  // SALIDA INTERRUPCION
                estado_encolar_pcb(estadoReady, pcb);
                pcb_set_estado_actual(pcb, READY);
                log_transition("EXEC", "READY", pcb_get_pid(pcb));
                sem_post(estado_get_sem(estadoReady));
                break;
            case HEADER_proceso_terminado:
                estado_encolar_pcb(estadoExit, pcb);
                pcb_set_estado_actual(pcb, EXIT);
                log_transition("EXEC", "EXIT", pcb_get_pid(pcb));
                sem_post(&gradoMultiprog);
                // mem_adapter_finalizar_proceso(pcb, kernelConfig, kernelLogger);
                pcb_responder_a_consola(pcb, HEADER_proceso_terminado);
                sem_post(estado_get_sem(estadoExit));
                break;
            case HEADER_proceso_bloqueado:
                pthread_create(&hiloAtenderBloqueo, NULL, (void*) atender_bloqueo, pcb); 
                pthread_detach(hiloAtenderBloqueo);
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
        log_info(kernelLogger, "Se toma una intancia de READY");

        if (kernel_config_es_algoritmo_srt(kernelConfig)) {
            log_info(kernelLogger, ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> ES SRT <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<");
            pthread_mutex_lock(&hayQueDesalojarMutex);
            if (hayQueDesalojar) {
                log_info(kernelLogger, "Se ingresa a hayQueDesalojar");
                hayQueDesalojar = false;
                pthread_mutex_lock(estado_get_mutex(estadoExec));
                if (list_size(estado_get_list(estadoExec)) == 1) {
                    log_info(kernelLogger, "Se ingresa a list_size(estado_get_list(estadoExec)) == 1");
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
