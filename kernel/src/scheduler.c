#include "scheduler.h"

#include <commons/collections/list.h>
#include <commons/log.h>
#include <commons/string.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdint.h>
#include <stdio.h>
#include <stdnoreturn.h>

#include "buffer.h"
#include "common_flags.h"
#include "estados.h"
#include "instruccion.h"
#include "kernel_config.h"
#include "mem_adapter.h"
#include "stream.h"

extern t_log* kernelLogger;
extern t_kernel_config* kernelConfig;
extern t_log* kernelLogger;

static uint32_t nextPid;
static pthread_mutex_t nextPidMutex;

static sem_t gradoMultiprog;
static sem_t hayPcbsParaAgregarAlSistema;

static t_estado* estadoNew;
static t_estado* estadoReady;
static t_estado* estadoExec;
static t_estado* estadoExit;
static t_estado* estadoBlocked;
static t_estado* estadoSuspendedBlocked;
static t_estado* estadoSuspendedReady;

static noreturn void planificador_largo_plazo(void);

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

    pthread_mutex_init(&nextPidMutex, NULL);

    int valorInicialGradoMultiprog = kernel_config_get_grado_multiprogramacion(kernelConfig);

    sem_init(&hayPcbsParaAgregarAlSistema, 0, 0);
    sem_init(&gradoMultiprog, 0, valorInicialGradoMultiprog);
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

    /* pthread_t medianoPlazoTh;
      pthread_create(&medianoPlazoTh, NULL, (void*)planificador_mediano_plazo, NULL);
      pthread_detach(medianoPlazoTh);

      pthread_t cortoPlazoTh;
      pthread_create(&cortoPlazoTh, NULL, (void*)planificador_corto_plazo, NULL);
      pthread_detach(cortoPlazoTh); */
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
        log_info(kernelLogger, "Se establece Handshake inicial con proceso de tamaño %d mediante <socket %d>", tamanio, *socketProceso);

        uint8_t consolaResponse = stream_recv_header(*socketProceso);  // Header lista de instrucciones solamentet_buffer* instructionsBuffer = buffer_create();
        if (consolaResponse != HEADER_lista_instrucciones) {
            log_error(kernelLogger, "Error al intentar recibir lista de instrucciones del proceso mediante <socket %d>", *socketProceso);
            return NULL;
        }

        t_buffer* instructionsBuffer = buffer_create();
        stream_recv_buffer(*socketProceso, instructionsBuffer);

        uint32_t newPid = get_next_pid();
        t_pcb* newPcb = pcb_create(newPid, tamanio, kernel_config_get_est_inicial(kernelConfig));

        uint8_t instruction = -1;
        bool isExit = false;
        while (!isExit) {
            buffer_unpack(instructionsBuffer, &instruction, sizeof(instruction));
            uint32_t op1 = -1;
            uint32_t op2 = -1;
            switch (instruction) {
                case INSTRUCCION_no_op:
                    buffer_unpack(instructionsBuffer, &op1, sizeof(op1));
                    break;
                case INSTRUCCION_io:
                    buffer_unpack(instructionsBuffer, &op1, sizeof(op1));
                    break;
                case INSTRUCCION_read:
                    buffer_unpack(instructionsBuffer, &op1, sizeof(op1));
                    break;
                case INSTRUCCION_copy:
                    buffer_unpack(instructionsBuffer, &op1, sizeof(op1));
                    buffer_unpack(instructionsBuffer, &op2, sizeof(op2));
                    break;
                case INSTRUCCION_write:
                    buffer_unpack(instructionsBuffer, &op1, sizeof(op1));
                    buffer_unpack(instructionsBuffer, &op2, sizeof(op2));
                    break;
                case INSTRUCCION_exit:
                    isExit = true;
                    break;
                default:
                    log_error(kernelLogger, "Se recibe instrucción desconocida de PCB <ID %d>", pcb_get_pid(newPcb));
                    return NULL;
            }
            t_instruccion* instruccionActual = instruccion_create(instruction, op1, op2);
            list_add(pcb_get_instrucciones(newPcb), instruccionActual);
        }
        estado_encolar_pcb(estadoNew, newPcb);
        sem_post(&hayPcbsParaAgregarAlSistema);
        log_transition("NULL", "NEW", pcb_get_pid(newPcb));
        buffer_destroy(instructionsBuffer);
    }
    return NULL;
}

static noreturn void planificador_largo_plazo(void) {
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
        } else {
            pthread_mutex_lock(estado_get_mutex(estadoNew));
            pcbQuePasaAReady = list_remove(estado_get_list(estadoNew), 0);
            pthread_mutex_unlock(estado_get_mutex(estadoNew));
            uint32_t nuevaTablaPagina = mem_adapter_obtener_tabla_pagina(pcbQuePasaAReady, kernelConfig, kernelLogger);
            pcb_set_tabla_paginas(pcbQuePasaAReady, nuevaTablaPagina);
            prevStatus = string_from_format("NEW");
        }
        estado_encolar_pcb(estadoReady, pcbQuePasaAReady);
        sem_post(estado_get_sem(estadoReady));
        log_transition(prevStatus, "READY", pcb_get_pid(pcbQuePasaAReady));
        free(prevStatus);
        prevStatus = NULL;
        pcbQuePasaAReady = NULL;
    }
}

/*
void interrumpir_cpu(void) {
    stream_send_empty_buffer(kernel_config_get_socket_interrupt_cpu(kernelConfig), INT_interrumpir_ejecucion);
    uint8_t cpuInterruptResponse = stream_recv_header(kernel_config_get_socket_interrupt_cpu(kernelConfig));
    stream_recv_empty_buffer(kernel_config_get_socket_interrupt_cpu(kernelConfig));
    if (cpuInterruptResponse != INT_cpu_interrumpida) {
        log_error(kernelLogger, "Error al intentar interrumpir CPU");
        // TODO: Manejar este caso
        exit(-1);
    }
    log_info(kernelLogger, "CPU interrumpida");
}

double srt(t_pcb* proceso){
        double alfa = kernel_config_get_alfa(kernelConfig);

        if(pcb_get_ultima_ejecucion(proceso) == -1){
                return pcb_get_est_actual(proceso);
        }

        pcb_set_est_actual(proceso,(alfa*pcb_get_ultima_ejecucion(proceso) + (1-alfa)*pcb_get_est_actual(proceso)));
        return pcb_get_est_actual(proceso);
}

t_pcb* planificar(void){
    void* t_planificar_srt(t_pcb* p1, t_pcb* p2){
        return (srt(p1) <= srt(p2)) ? p1 : p2;
    }
    t_pcb* seleccionado = NULL;

    if(strcmp(kernel_config_get_algoritmo(kernelConfig), "SRT") == 0){
        if(list_size(estado_get_list(estadoReady)) == 1){
            seleccionado = list_get(estado_get_list(estadoReady),0);
                        srt(seleccionado);
                        return seleccionado;
                }
            seleccionado = list_get_minimum(estado_get_list(estadoReady),(void*)t_planificar_srt);
        return seleccionado;
    }
    return list_get(estado_get_list(estadoReady),0); //FIFO
}

static noreturn void planificador_mediano_plazo(void) {
    bool supera_limite_block(void* tcb_en_lista) {
                return (pcb_get_tiempo_de_bloq((t_pcb*)tcb_en_lista) > kernel_config_get_maximo_bloq(kernelConfig)); // TODO: los dos gets de esta linea
        }

    t_pcb* pcbASuspender = NULL;

    for (;;) {
        //SEMAFORO HAY UNO BLOQUEADO
        if(list_size(estado_get_list(estadoBlocked)) > 0 && list_any_satisfy(estado_get_list(estadoBlocked),(void*)supera_limite_block)){
            pcbASuspender = list_remove_by_condition(estado_get_list(estadoBlocked),(void*)supera_limite_block);
            estado_encolar_pcb(estadoSuspendedBlocked, pcbASuspender);
            //SACAR TIEMPO BLOQUEO
            //SEMAFORO BLOQUEADOS --
        }
    }
}

static noreturn void planificador_corto_plazo(void) {
    t_pcb* pcbQuePasaAExec = NULL;
    for (;;) {
        sem_wait(estado_get_semaforo(estadoReady));
        // if(hay_uno_planificando()){  // TODO: enviar interrupcion a CPU si ya hay uno planificando y es SRT
            interrumpir_cpu();
        // }
        //MUTEX CPU DESOCUPADA (INTERRUPCION O SALIDA)
        pcbQuePasaAExec = planificar();
        //PASAR A EXEC
        //MARCAR TIEMPO DE INICIO DE RAFAGA
        //CREAR HILO DE ATENCION - POSTEAR SEMAFOROS EN SALIDA
    }
}

//ENVIAR PCB A CPU (datos q necesite)
//SWITCHEAR LOS RETORNOS
*/
