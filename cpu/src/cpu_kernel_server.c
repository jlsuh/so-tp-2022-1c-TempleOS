#include "cpu_kernel_server.h"

#include <commons/log.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdnoreturn.h>

#include "common_flags.h"
#include "cpu_config.h"
#include "instruccion.h"
#include "pcb_cpu.h"
#include "stream.h"

extern t_cpu_config* cpuConfig;
extern t_log* cpuLogger;

static void noreturn dispatch_peticiones_de_kernel(void) {
    uint32_t pidRecibido = 0, tablaPags = 0;
    uint64_t programCounter = 0;
    for (;;) {
        // Recibir PCB de Kernel
        uint8_t kernelResponse = stream_recv_header(cpu_config_get_socket_dispatch(cpuConfig));
        t_buffer* bufferPcb = NULL;
        if (kernelResponse == HEADER_pcb_a_ejecutar) {
            bufferPcb = buffer_create();
            stream_recv_buffer(cpu_config_get_socket_dispatch(cpuConfig), bufferPcb);
            buffer_unpack(bufferPcb, &pidRecibido, sizeof(pidRecibido));
            buffer_unpack(bufferPcb, &programCounter, sizeof(programCounter));
            buffer_unpack(bufferPcb, &tablaPags, sizeof(tablaPags));
            buffer_destroy(bufferPcb);

            t_pcb_cpu* newPcb = pcb_cpu_create(pidRecibido, programCounter, tablaPags);
            kernelResponse = stream_recv_header(cpu_config_get_socket_dispatch(cpuConfig));
            t_buffer* bufferInstrucciones = NULL;
            if (kernelResponse == HEADER_lista_instrucciones) {
                bufferInstrucciones = buffer_create();
                stream_recv_buffer(cpu_config_get_socket_dispatch(cpuConfig), bufferInstrucciones);

                // Start duplicación de lógica
                uint8_t instruction = -1;
                bool isExit = false;
                while (!isExit) {
                    buffer_unpack(bufferInstrucciones, &instruction, sizeof(instruction));
                    uint32_t op1 = -1;
                    uint32_t op2 = -1;
                    switch (instruction) {
                        case INSTRUCCION_no_op:
                            buffer_unpack(bufferInstrucciones, &op1, sizeof(op1));
                            break;
                        case INSTRUCCION_io:
                            buffer_unpack(bufferInstrucciones, &op1, sizeof(op1));
                            break;
                        case INSTRUCCION_read:
                            buffer_unpack(bufferInstrucciones, &op1, sizeof(op1));
                            break;
                        case INSTRUCCION_copy:
                            buffer_unpack(bufferInstrucciones, &op1, sizeof(op1));
                            buffer_unpack(bufferInstrucciones, &op2, sizeof(op2));
                            break;
                        case INSTRUCCION_write:
                            buffer_unpack(bufferInstrucciones, &op1, sizeof(op1));
                            buffer_unpack(bufferInstrucciones, &op2, sizeof(op2));
                            break;
                        case INSTRUCCION_exit:
                            isExit = true;
                            break;
                        default:
                            log_error(cpuLogger, "Error al intentar desempaquetar una instrucción");
                            exit(-1);
                    }
                    t_instruccion* instruccionActual = instruccion_create(instruction, op1, op2);
                    log_info(cpuLogger, "Se empaqueta instruccion: %d con operandos %d y %d", instruction, op1, op2);
                    list_add(pcb_cpu_get_instrucciones(newPcb), instruccionActual);
                }
                // End duplicación de lógica
            }
        } else {
            log_error(cpuLogger, "Error al intentar recibir el PCB de Kernel");
            exit(-1);
        }
        // TODO: Hacer ciclo instruccion
        // TODO: Chequear si hay interrupcion
    }
}

static void noreturn interrupt_peticiones_de_kernel(void) {
    for (;;) {
    }
}

void atender_peticiones_de_kernel(void) {
    pthread_t dispatchTh;
    pthread_create(&dispatchTh, NULL, (void*)dispatch_peticiones_de_kernel, NULL);

    pthread_t interruptTh;
    pthread_create(&interruptTh, NULL, (void*)interrupt_peticiones_de_kernel, NULL);

    log_info(cpuLogger, "Hilos de atención creados. Listo para atender peticiones de Kernel");

    pthread_join(dispatchTh, NULL);
    pthread_join(interruptTh, NULL);
}
