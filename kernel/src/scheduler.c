#include "scheduler.h"

#include <commons/collections/list.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdint.h>
#include <stdio.h>
#include <stdnoreturn.h>

#include "buffer.h"
#include "common_flags.h"

typedef enum estado {
    NEW,
    READY,
    EXEC,
    EXIT,
    BLOCKED,
    SUSPENDED_READY,
    SUSPENDED_BLOCKED,
} t_nombre_estado;

typedef struct {
    t_nombre_estado nombre;
    t_list* listaProcesos;
    sem_t semaforoEstado;
    pthread_mutex_t mutexEstado;
} t_estado;

// static t_estado pcbsNew;

void* encolar_en_new_a_nuevo_pcb_entrante(void* socket) {
    int* socketProceso = (int*)socket;
    uint8_t response = stream_recv_header(*socketProceso);
    uint32_t tamanio = 0;
    if (response != HANDSHAKE_consola) {
        log_error(kernelLogger, "Error al intentar establecer conexión con proceso %d", *socketProceso);
    } else {
        t_buffer* buffer = buffer_create();
        stream_recv_buffer(*socketProceso, buffer);
        buffer_unpack(buffer, &tamanio, sizeof(tamanio));
        buffer_destroy(buffer);
        log_info(kernelLogger, "Se establece handshake con proceso %d de tamaño %d", *socketProceso, tamanio);
        stream_send_empty_buffer(*socketProceso, HANDSHAKE_ok_continue);
        stream_recv_header(*socketProceso);  // Header lista de instrucciones solamentet_buffer* instructionsBuffer = buffer_create();
        stream_recv_buffer(*socketProceso, instructionsBuffer);

        t_buffer* instructionsBuffer = buffer_create();
        stream_recv_buffer(*socketProceso, instructionsBuffer);

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
            printf("Instrucción: %d con argumento/s %d %d\n", instruction, op1, op2);
            op1 = -1;
            op2 = -1;
            buffer_unpack(instructionsBuffer, &instruction, sizeof(instruction));
        }  // TODO: crear instrucción y agregar a la lista
    }
    // TODO: crear PCB con lista instrucciones y tamaño y pasar a NEW
    return NULL;
}

static noreturn void planificador_largo_plazo(void) {
    for (;;) {
    }
}
