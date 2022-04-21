#include "scheduler.h"

#include <commons/collections/list.h>
#include <commons/log.h>
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
#include "stream.h"

extern t_log* kernelLogger;
extern t_kernel_config* kernelConfig;

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

uint32_t get_next_pid(void) {
    pthread_mutex_lock(&nextPidMutex);
    uint32_t newNextPid = nextPid++;
    pthread_mutex_unlock(&nextPidMutex);
    return newNextPid;
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
}

void* encolar_en_new_a_nuevo_pcb_entrante(void* socket) {
    int* socketProceso = (int*)socket;
    uint8_t response = stream_recv_header(*socketProceso);
    uint32_t tamanio = 0;
    if (response != HANDSHAKE_consola) {
        log_error(kernelLogger, "Error al intentar establecer conexión con proceso %d", *socketProceso);
    } else {
        t_buffer* bufferHandshakeInicial = buffer_create();
        stream_recv_buffer(*socketProceso, bufferHandshakeInicial);
        buffer_unpack(bufferHandshakeInicial, &tamanio, sizeof(tamanio));
        buffer_destroy(bufferHandshakeInicial);
        log_info(kernelLogger, "Se establece handshake con proceso %d de tamaño %d", *socketProceso, tamanio);
        stream_send_empty_buffer(*socketProceso, HANDSHAKE_ok_continue);
        stream_recv_header(*socketProceso);  // Header lista de instrucciones solamentet_buffer* instructionsBuffer = buffer_create();

        t_buffer* instructionsBuffer = buffer_create();
        stream_recv_buffer(*socketProceso, instructionsBuffer);

        uint32_t newPid = get_next_pid();
        t_pcb* newPcb = pcb_create(newPid, tamanio, kernel_config_get_est_inicial(kernelConfig));

        uint8_t instruction = -1;
        uint32_t op1 = -1;
        uint32_t op2 = -1;
        buffer_unpack(instructionsBuffer, &instruction, sizeof(instruction));
        for (; instruction != INSTRUCCION_exit;) {
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
                default:
                    log_error(kernelLogger, "Error al intentar desempaquetar una instrucción del proceso %d", *socketProceso);
                    pthread_exit(NULL);
            }

            t_instruccion* instruccionActual = instruccion_create(instruction, op1, op2);
            list_add(pcb_get_instrucciones(newPcb), instruccionActual);

            printf("Instrucción: %d con argumento/s %d %d\n", instruction, op1, op2);
            op1 = -1;
            op2 = -1;
            buffer_unpack(instructionsBuffer, &instruction, sizeof(instruction));
        }
        log_info(kernelLogger, "El tamanio de la instrucción es: %d", list_size(pcb_get_instrucciones(newPcb)));

        estado_encolar_pcb(estadoNew, newPcb);
        sem_post(&hayPcbsParaAgregarAlSistema);
    }
    return NULL;
}

static noreturn void planificador_largo_plazo(void) {
    t_pcb* pcbQuePasaAReady = NULL;
    for (;;) {
        sem_wait(&hayPcbsParaAgregarAlSistema);
        sem_wait(&gradoMultiprog);

        if (list_size(estado_get_list(estadoSuspendedReady)) > 0) {  // Da prioridad a los procesos en suspended ready por sobre los nuevos
            pcbQuePasaAReady = list_remove(estado_get_list(estadoSuspendedReady), 0);
        } else {
            pcbQuePasaAReady = list_remove(estado_get_list(estadoNew), 0);
            // TODO: avisar a memoria y pedir tabla
            log_info(kernelLogger, "List get de PCB de ID: %d", pcb_get_pid(pcbQuePasaAReady));
        }
        // TODO: pasar a ready
    }
}

/*static noreturn void planificador_mediano_plazo(void) {

}

static noreturn void planificador_corto_plazo(void) {

}*/

/*
        pthread_t planif_corto;
        pthread_create(&planif_corto, NULL, (void*) planificador_corto_plazo, NULL);
        pthread_detach((pthread_t) planif_corto);

        pthread_t planif_medio_suspender;
        pthread_create(&planif_medio_suspender, NULL, (void*) planificador_mediano_plazo_suspender, NULL);
        pthread_detach((pthread_t) planif_medio_suspender);
*/
/*
static noreturn void* __iniciar_largo_plazo(void* _) {
    pthread_t th;
    pthread_create(&th, NULL, __liberar_carpinchos_en_exit, NULL);
    pthread_detach(th);
    log_info(kernelLogger, "Largo Plazo: Inicialización exitosa");
    for (;;) {
        // tanto NEW como SUSREADY son parte del mismo conjunto: "el conjunto a pasar a READY"
        sem_wait(&hayPCBsParaAgregarAlSistema);
        log_info(kernelLogger, "Largo Plazo: Se toma una instancia de PCBs a agregar al sistema");

        sem_wait(&gradoMultiprog);
        log_info(kernelLogger, "Largo Plazo: Se toma una instancia de Grado Multiprogramación");

        if (!list_is_empty(cola_planificacion_get_list(pcbsSusReady))) {
            sem_post(&transicionarSusReadyAready);
        } else {
            t_pcb* pcbQuePasaAReady = __get_and_remove_first_pcb_from_queue(pcbsNew);

            pcb_algoritmo_init(pcbQuePasaAReady);
            log_info(kernelLogger, "Largo Plazo: Incialización de información del algoritmo correcta");

            pcb_transition_to_ready(pcbQuePasaAReady);
            __enqueue_pcb(pcbQuePasaAReady, pcbsReady);
            log_transition("Largo Plazo", "NEW", "READY", pcb_get_pid(pcbQuePasaAReady));

            sem_post(cola_planificacion_get_instancias_disponibles(pcbsReady));
        }
    }
}*/
