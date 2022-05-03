#include "cpu_kernel_server.h"

#include <commons/log.h>
#include <commons/string.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdnoreturn.h>

#include "common_flags.h"
#include "common_utils.h"
#include "cpu_config.h"
// #include "cpu_memoria_cliente.h"
#include "instruccion.h"
#include "pcb_cpu.h"
#include "stream.h"

extern t_cpu_config* cpuConfig;
extern t_log* cpuLogger;

static bool hayInterrupcion;
static pthread_mutex_t mutexInterrupcion;

static t_instruccion* cpu_fetch_instruction(t_pcb_cpu* pcb) {
    t_list* instructionsList = pcb_cpu_get_instrucciones(pcb);
    uint64_t programCounter = pcb_cpu_get_program_counter(pcb);
    t_instruccion* nextInstruction = list_get(instructionsList, programCounter);
    return nextInstruction;
}

static bool cpu_decode_instruction(t_instruccion* instruction) {
    if (instruccion_get_tipo_instruccion(instruction) == INSTRUCCION_copy) {
        return true;
    }
    return false;
}

static uint32_t instruccion_fetch_operands(t_instruccion* nextInstruction, t_pcb_cpu* pcb) {
    // Llegamos acá solamente en caso de INSTRUCCION_copy
    uint32_t direccionLogicaOrigen = instruccion_get_operando2(nextInstruction);
    // uint32_t readValue = leer_en_memoria(direccionLogicaOrigen, pcb_cpu_get_tabla_pagina_primer_nivel(pcb)); Se abstrae de memoria por ahora
    uint32_t readValue = 2;
    log_info(cpuLogger, "INSTRUCCION_copy: Se lee el valor %d de memoria de la dirección lógica %d", readValue, direccionLogicaOrigen);
    return readValue;
}

static bool cpu_exec_instruction(t_pcb_cpu* pcb, t_tipo_instruccion tipoInstruccion, uint32_t operando1, uint32_t operando2) {
    pcb_cpu_set_program_counter(pcb, pcb_cpu_get_program_counter(pcb) + 1);
    uint64_t programCounterActualizado = pcb_cpu_get_program_counter(pcb);

    bool stopExec = false;
    bool shouldWrite = false;
    char* logMsg = NULL;

    if (tipoInstruccion == INSTRUCCION_no_op) {
        uint32_t retardoNoOpTotal = operando1 * cpu_config_get_retardo_no_op(cpuConfig);
        log_info(cpuLogger, "INSTRUCCION_no_op: Ejecución NO_OP durante %d milisegundos", retardoNoOpTotal);
        intervalo_de_pausa(retardoNoOpTotal);
        log_info(cpuLogger, "Fin de la ejecución de la instrucción NO_OP");
    } else if (tipoInstruccion == INSTRUCCION_io) {
        uint32_t tiempoDeBloqueo = operando1;
        log_info(cpuLogger, "INSTRUCCION_io: Tiempo de bloqueo de %d milisegundos", tiempoDeBloqueo);
        uint32_t pid = pcb_cpu_get_pid(pcb);
        uint32_t tablaPaginaPrimerNivelActualizado = pcb_cpu_get_tabla_pagina_primer_nivel(pcb);

        t_buffer* bufferIO = buffer_create();
        buffer_pack(bufferIO, &pid, sizeof(pid));
        buffer_pack(bufferIO, &programCounterActualizado, sizeof(programCounterActualizado));
        buffer_pack(bufferIO, &tablaPaginaPrimerNivelActualizado, sizeof(tablaPaginaPrimerNivelActualizado));
        buffer_pack(bufferIO, &tiempoDeBloqueo, sizeof(tiempoDeBloqueo));  // Hace falta que en el struct tengamos tiempoBloqueo como miembro?
        stream_send_buffer(cpu_config_get_socket_dispatch(cpuConfig), HEADER_proceso_bloqueado, bufferIO);
        buffer_destroy(bufferIO);

        log_info(cpuLogger, "INSTRUCCION_io: Se envía a Kernel <PID %d> con <PC %ld> con <TP1erNivel %d> con <Tiempo de bloqueo %d>", pid, programCounterActualizado, tablaPaginaPrimerNivelActualizado, tiempoDeBloqueo);

        stopExec = true;
    } else if (tipoInstruccion == INSTRUCCION_read) {
        // uint32_t readValue = leer_en_memoria(operando1, pcb_cpu_get_tabla_pagina_primer_nivel(pcb)); Se abstrae de memoria por ahora
        uint32_t readValue = 3;
        // TODO: Esto debe ser loggeado en consola también o solamente en CPU?
        log_info(cpuLogger, "INSTRUCCION_read: Se lee %d de la dirección lógica %d", readValue, operando1);
    } else if (tipoInstruccion == INSTRUCCION_write) {
        shouldWrite = true;
        logMsg = string_from_format("INSTRUCCION_write: Se escribe %d en la dirección lógica %d", operando1, operando2);
    } else if (tipoInstruccion == INSTRUCCION_copy) {
        shouldWrite = true;
        logMsg = string_from_format("INSTRUCCION_copy: Se escribe %d en la dirección lógica %d", operando1, operando2);
    } else if (tipoInstruccion == INSTRUCCION_exit) {
        log_info(cpuLogger, "INSTRUCCION_exit: Se termina la ejecución del programa");
        uint32_t pid = pcb_cpu_get_pid(pcb);
        uint32_t tablaPaginaPrimerNivelActualizado = pcb_cpu_get_tabla_pagina_primer_nivel(pcb);
        t_buffer* bufferExit = buffer_create();
        buffer_pack(bufferExit, &pid, sizeof(pid));
        buffer_pack(bufferExit, &programCounterActualizado, sizeof(programCounterActualizado));
        buffer_pack(bufferExit, &tablaPaginaPrimerNivelActualizado, sizeof(tablaPaginaPrimerNivelActualizado));
        stream_send_buffer(cpu_config_get_socket_dispatch(cpuConfig), HEADER_proceso_terminado, bufferExit);
        buffer_destroy(bufferExit);

        log_info(cpuLogger, "INSTRUCCION_exit: Se envía a Kernel <PID %d> con <PC %ld> con <TP1erNivel %d>", pid, programCounterActualizado, tablaPaginaPrimerNivelActualizado);

        stopExec = true;
    }

    if (shouldWrite) {
        // escribir_en_memoria(operando1, pcb_cpu_get_tabla_pagina_primer_nivel(pcb), operando2); Se abstrae de memoria por ahora
        log_info(cpuLogger, "%s", logMsg);
        free(logMsg);
    }

    return stopExec;
}

static bool cpu_ejecutar_ciclos_de_instruccion(t_pcb_cpu* pcb) {
    // Fetch Instruction
    t_instruccion* nextInstruction = cpu_fetch_instruction(pcb);
    log_info(cpuLogger, "Fetch Instruction: Se obtiene la siguiente instrucción");

    // Decode
    bool shouldFetchOperands = cpu_decode_instruction(nextInstruction);
    log_info(cpuLogger, "Decode: Se decodifica la siguiente instrucción");

    // Fetch Operands
    t_tipo_instruccion tipoInstruccion = instruccion_get_tipo_instruccion(nextInstruction);
    uint32_t operando1 = instruccion_get_operando1(nextInstruction);
    uint32_t operando2 = instruccion_get_operando2(nextInstruction);
    if (shouldFetchOperands) {  // Hace que la instrucción copy o write sean lo mismo
        operando2 = instruccion_fetch_operands(nextInstruction, pcb);
        log_info(cpuLogger, "Fetch Operands: Se obtienen los operandos de la siguiente instrucción");
    }

    // Exec Instruction
    bool stopExec = cpu_exec_instruction(pcb, tipoInstruccion, operando1, operando2);
    log_info(cpuLogger, "Exec Instruction: Se ejecuta la siguiente instrucción");

    return stopExec;
}

static bool cpu_hay_interrupcion(t_pcb_cpu* pcb) {
    pthread_mutex_lock(&mutexInterrupcion);
    bool stopExec = false;
    if (hayInterrupcion) {
        uint32_t pid = pcb_cpu_get_pid(pcb);
        uint64_t programCounterActualizado = pcb_cpu_get_program_counter(pcb);
        uint32_t tablaPaginaPrimerNivelActualizado = pcb_cpu_get_tabla_pagina_primer_nivel(pcb);

        t_buffer* bufferInt = buffer_create();
        buffer_pack(bufferInt, &pid, sizeof(pid));
        buffer_pack(bufferInt, &programCounterActualizado, sizeof(programCounterActualizado));
        buffer_pack(bufferInt, &tablaPaginaPrimerNivelActualizado, sizeof(tablaPaginaPrimerNivelActualizado));
        stream_send_buffer(cpu_config_get_socket_dispatch(cpuConfig), HEADER_proceso_desalojado, bufferInt);
        buffer_destroy(bufferInt);

        hayInterrupcion = false;
        stopExec = true;

        log_info(cpuLogger, "INT: Se interrumpe la ejecución del programa");
        log_info(cpuLogger, "INT: Se envía a Kernel <PID %d> con <PC %ld> con <TP1erNivel %d>", pid, programCounterActualizado, tablaPaginaPrimerNivelActualizado);
    }
    pthread_mutex_unlock(&mutexInterrupcion);
    return stopExec;
}

static void noreturn dispatch_peticiones_de_kernel(void) {
    uint32_t pidRecibido = 0, tablaPags = 0;
    uint64_t programCounter = 0;
    for (;;) {
        // Recibir PCB de Kernel
        uint8_t kernelResponse = stream_recv_header(cpu_config_get_socket_dispatch(cpuConfig));
        t_buffer* bufferPcb = NULL;
        t_pcb_cpu* newPcb = NULL;
        if (kernelResponse == HEADER_pcb_a_ejecutar) {
            bufferPcb = buffer_create();
            stream_recv_buffer(cpu_config_get_socket_dispatch(cpuConfig), bufferPcb);
            buffer_unpack(bufferPcb, &pidRecibido, sizeof(pidRecibido));
            buffer_unpack(bufferPcb, &programCounter, sizeof(programCounter));
            buffer_unpack(bufferPcb, &tablaPags, sizeof(tablaPags));
            buffer_destroy(bufferPcb);

            newPcb = pcb_cpu_create(pidRecibido, programCounter, tablaPags);
            kernelResponse = stream_recv_header(cpu_config_get_socket_dispatch(cpuConfig));
            t_buffer* bufferInstrucciones = NULL;
            if (kernelResponse == HEADER_lista_instrucciones) {
                bufferInstrucciones = buffer_create();
                stream_recv_buffer(cpu_config_get_socket_dispatch(cpuConfig), bufferInstrucciones);

                t_list* listaInstrucciones = instruccion_list_create_from_buffer(bufferInstrucciones, cpuLogger);
                pcb_cpu_set_instrucciones(newPcb, listaInstrucciones);
            } else {
                log_error(cpuLogger, "Error al intentar recibir las instrucciones de Kernel");
                exit(-1);
            }

            bool stopExec = false;
            while (!stopExec) {
                // Hacer ciclo instruccion
                stopExec = cpu_ejecutar_ciclos_de_instruccion(newPcb);

                if(!stopExec) {
                    // TODO: Chequear si hay interrupcion
                    stopExec = cpu_hay_interrupcion(newPcb);
                }
            }
        } else {
            log_error(cpuLogger, "Error al intentar recibir el PCB de Kernel");
            exit(-1);
        }
    }
}

static void noreturn interrupt_peticiones_de_kernel(void) {
    for (;;) {
        uint8_t nuevaInterrupcion = stream_recv_header(cpu_config_get_socket_interrupt(cpuConfig));
        if(nuevaInterrupcion == INT_interrumpir_ejecucion) {
            pthread_mutex_lock(&mutexInterrupcion);
            hayInterrupcion = true;
            pthread_mutex_unlock(&mutexInterrupcion);
            log_info(cpuLogger, "INT: Se recibe de Kernel una petición de interrupción");
        }
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
