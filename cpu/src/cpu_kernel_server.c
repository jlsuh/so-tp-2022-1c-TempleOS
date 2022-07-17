#include "cpu_kernel_server.h"

#include <commons/string.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdnoreturn.h>

#include "common_utils.h"
#include "cpu_config.h"
#include "cpu_memoria_cliente.h"
#include "cpu_pcb.h"
#include "instruccion.h"
#include "stream.h"

extern t_cpu_config* cpuConfig;
extern t_log* cpuLogger;

t_tlb* tlb;

static bool hayInterrupcion;
static int pidProcesoEnExec;
static pthread_mutex_t mutexInterrupcion;

static t_instruccion* __cpu_fetch_instruction(t_cpu_pcb* pcb) {
    t_list* instructionsList = cpu_pcb_get_instrucciones(pcb);
    uint32_t programCounter = cpu_pcb_get_program_counter(pcb);
    t_instruccion* nextInstruction = list_get(instructionsList, programCounter);
    log_info(cpuLogger, "FETCH INSTRUCTION: PCB <ID %d>", cpu_pcb_get_pid(pcb));
    return nextInstruction;
}

static bool __cpu_decode_instruction(uint32_t pid, t_instruccion* instruction) {
    log_info(cpuLogger, "DECODE INSTRUCTION: PCB <ID %d> Decoded Instruction: %s", pid, instruccion_to_string(instruction));
    return instruccion_get_tipo_instruccion(instruction) == INSTRUCCION_copy;
}

static uint32_t __cpu_fetch_operands(t_instruccion* nextInstruction, t_cpu_pcb* pcb) {
    uint32_t direccionLogicaOrigen = instruccion_get_operando2(nextInstruction);
    printf("CPU tabla página primer nivel en CPU_fetch_operands: %d\n", cpu_pcb_get_tabla_pagina_primer_nivel(pcb));
    uint32_t fetchedValue = cpu_leer_en_memoria(tlb, cpu_config_get_socket_memoria(cpuConfig), direccionLogicaOrigen, cpu_pcb_get_tabla_pagina_primer_nivel(pcb));
    log_info(cpuLogger, "FETCH OPERANDS: PCB <ID %d> COPY <DL Destino: %d> <DL Origen: %d> => Fetched Value: %d", cpu_pcb_get_pid(pcb), instruccion_get_operando1(nextInstruction), direccionLogicaOrigen, fetchedValue);
    return fetchedValue;
}

static bool __cpu_exec_instruction(t_cpu_pcb* pcb, t_tipo_instruccion tipoInstruccion, uint32_t operando1, uint32_t operando2) {
    cpu_pcb_set_program_counter(pcb, cpu_pcb_get_program_counter(pcb) + 1);
    uint32_t programCounterActualizado = cpu_pcb_get_program_counter(pcb);
    bool shouldStopExec = false;
    bool shouldWrite = false;
    char* logMsg = NULL;
    if (tipoInstruccion == INSTRUCCION_no_op) {
        uint32_t retardoNoOp = cpu_config_get_retardo_no_op(cpuConfig);
        log_info(cpuLogger, "EXEC: PCB <ID %d> NO_OP <%d milisegundos>", cpu_pcb_get_pid(pcb), retardoNoOp);
        intervalo_de_pausa(retardoNoOp);
    } else if (tipoInstruccion == INSTRUCCION_io) {
        uint32_t tiempoDeBloqueo = operando1;
        uint32_t pid = cpu_pcb_get_pid(pcb);
        uint32_t tablaPaginaPrimerNivelActualizado = cpu_pcb_get_tabla_pagina_primer_nivel(pcb);
        log_info(cpuLogger, "EXEC: PCB <ID %d> I/O <%d milisegundos>", pid, tiempoDeBloqueo);
        t_buffer* bufferIO = buffer_create();
        buffer_pack(bufferIO, &pid, sizeof(pid));
        buffer_pack(bufferIO, &programCounterActualizado, sizeof(programCounterActualizado));
        buffer_pack(bufferIO, &tablaPaginaPrimerNivelActualizado, sizeof(tablaPaginaPrimerNivelActualizado));
        buffer_pack(bufferIO, &tiempoDeBloqueo, sizeof(tiempoDeBloqueo));
        stream_send_buffer(cpu_config_get_socket_dispatch(cpuConfig), HEADER_proceso_bloqueado, bufferIO);
        buffer_destroy(bufferIO);
        shouldStopExec = true;
    } else if (tipoInstruccion == INSTRUCCION_read) {
        printf("CPU tabla página primer nivel en INSTRUCCION_read: %d\n", cpu_pcb_get_tabla_pagina_primer_nivel(pcb));
        uint32_t readValue = cpu_leer_en_memoria(tlb, cpu_config_get_socket_memoria(cpuConfig), operando1, cpu_pcb_get_tabla_pagina_primer_nivel(pcb));
        log_info(cpuLogger, "EXEC: PCB <ID %d> READ <DL: %d> => Read Value: %d", cpu_pcb_get_pid(pcb), operando1, readValue);
    } else if (tipoInstruccion == INSTRUCCION_write) {
        shouldWrite = true;
        logMsg = string_from_format("EXEC: PCB <ID %d> WRITE <DL: %d> <Write Value: %d>", cpu_pcb_get_pid(pcb), operando1, operando2);
    } else if (tipoInstruccion == INSTRUCCION_copy) {
        shouldWrite = true;
        logMsg = string_from_format("EXEC: PCB <ID %d> COPY <DL: %d> <Copy Value: %d>", cpu_pcb_get_pid(pcb), operando1, operando2);
    } else if (tipoInstruccion == INSTRUCCION_exit) {
        log_info(cpuLogger, "EXEC: PCB <ID %d> EXIT", cpu_pcb_get_pid(pcb));
        uint32_t pid = cpu_pcb_get_pid(pcb);
        uint32_t tablaPaginaPrimerNivelActualizado = cpu_pcb_get_tabla_pagina_primer_nivel(pcb);
        t_buffer* bufferExit = buffer_create();
        buffer_pack(bufferExit, &pid, sizeof(pid));
        buffer_pack(bufferExit, &programCounterActualizado, sizeof(programCounterActualizado));
        buffer_pack(bufferExit, &tablaPaginaPrimerNivelActualizado, sizeof(tablaPaginaPrimerNivelActualizado));
        stream_send_buffer(cpu_config_get_socket_dispatch(cpuConfig), HEADER_proceso_terminado, bufferExit);
        buffer_destroy(bufferExit);
        shouldStopExec = true;
    }
    if (shouldWrite) {
        printf("CPU tabla página primer nivel en ESCRIBIR EN MEMORIA: %d\n", cpu_pcb_get_tabla_pagina_primer_nivel(pcb));
        cpu_escribir_en_memoria(tlb, cpu_config_get_socket_memoria(cpuConfig), operando1, cpu_pcb_get_tabla_pagina_primer_nivel(pcb), operando2);
        log_info(cpuLogger, "%s", logMsg);
        free(logMsg);
    }
    return shouldStopExec;
}

static bool __cpu_ejecutar_ciclos_de_instruccion(t_cpu_pcb* pcb) {
    t_instruccion* nextInstruction = __cpu_fetch_instruction(pcb);
    bool shouldFetchOperands = __cpu_decode_instruction(cpu_pcb_get_pid(pcb), nextInstruction);
    t_tipo_instruccion tipoInstruccion = instruccion_get_tipo_instruccion(nextInstruction);
    uint32_t operando1 = instruccion_get_operando1(nextInstruction);
    uint32_t operando2 = instruccion_get_operando2(nextInstruction);
    if (shouldFetchOperands) {
        operando2 = __cpu_fetch_operands(nextInstruction, pcb);
    }
    bool shouldStopExec = __cpu_exec_instruction(pcb, tipoInstruccion, operando1, operando2);
    return shouldStopExec;
}

static bool __cpu_atender_interrupcion(t_cpu_pcb* pcb) {
    pthread_mutex_lock(&mutexInterrupcion);
    bool shouldStopExec = false;
    if (hayInterrupcion) {
        uint32_t pid = cpu_pcb_get_pid(pcb);
        uint32_t programCounterActualizado = cpu_pcb_get_program_counter(pcb);
        uint32_t tablaPaginaPrimerNivelActualizado = cpu_pcb_get_tabla_pagina_primer_nivel(pcb);
        t_buffer* bufferInt = buffer_create();
        buffer_pack(bufferInt, &pid, sizeof(pid));
        buffer_pack(bufferInt, &programCounterActualizado, sizeof(programCounterActualizado));
        buffer_pack(bufferInt, &tablaPaginaPrimerNivelActualizado, sizeof(tablaPaginaPrimerNivelActualizado));
        stream_send_buffer(cpu_config_get_socket_dispatch(cpuConfig), HEADER_proceso_desalojado, bufferInt);
        buffer_destroy(bufferInt);
        hayInterrupcion = false;
        shouldStopExec = true;
        log_info(cpuLogger, "INT: Se envía a Kernel <PID %d> con <PC %d> <TP1erNivel %d>", pid, programCounterActualizado, tablaPaginaPrimerNivelActualizado);
    }
    pthread_mutex_unlock(&mutexInterrupcion);
    return shouldStopExec;
}

static void noreturn __dispatch_peticiones_de_kernel(void) {
    uint32_t pidRecibido = 0, tablaPags = 0;
    uint32_t programCounter = 0;
    for (;;) {
        uint8_t kernelResponse = stream_recv_header(cpu_config_get_socket_dispatch(cpuConfig));
        t_buffer* bufferPcb = NULL;
        t_cpu_pcb* pcb = NULL;
        if (kernelResponse == HEADER_pcb_a_ejecutar || kernelResponse == HEADER_pcb_a_ejecutar_ultimo_suspendido) {
            bufferPcb = buffer_create();
            stream_recv_buffer(cpu_config_get_socket_dispatch(cpuConfig), bufferPcb);
            buffer_unpack(bufferPcb, &pidRecibido, sizeof(pidRecibido));
            buffer_unpack(bufferPcb, &programCounter, sizeof(programCounter));
            buffer_unpack(bufferPcb, &tablaPags, sizeof(tablaPags));
            buffer_destroy(bufferPcb);
            if (kernelResponse == HEADER_pcb_a_ejecutar_ultimo_suspendido || pidRecibido != pidProcesoEnExec) {
                tlb_flush(tlb);
                pidProcesoEnExec = pidRecibido;
            }
            pcb = cpu_pcb_create(pidRecibido, programCounter, tablaPags);
            kernelResponse = stream_recv_header(cpu_config_get_socket_dispatch(cpuConfig));
            if (kernelResponse == HEADER_lista_instrucciones) {
                t_buffer* bufferInstrucciones = buffer_create();
                stream_recv_buffer(cpu_config_get_socket_dispatch(cpuConfig), bufferInstrucciones);
                t_list* listaInstrucciones = instruccion_list_create_from_buffer(bufferInstrucciones, cpuLogger);
                cpu_pcb_set_instrucciones(pcb, listaInstrucciones);
                buffer_destroy(bufferInstrucciones);
            } else {
                log_error(cpuLogger, "Error al intentar recibir las instrucciones de Kernel");
                exit(-1);
            }
            bool shouldStopExec = false;
            while (!shouldStopExec) {
                shouldStopExec = __cpu_ejecutar_ciclos_de_instruccion(pcb);
                if (!shouldStopExec) {
                    shouldStopExec = __cpu_atender_interrupcion(pcb);
                }
            }
            cpu_pcb_destroy(pcb);
        } else {
            log_error(cpuLogger, "Error al intentar recibir el PCB de Kernel");
            exit(-1);
        }
    }
}

static void noreturn __interrupt_peticiones_de_kernel(void) {
    for (;;) {
        uint8_t nuevaInterrupcion = stream_recv_header(cpu_config_get_socket_interrupt(cpuConfig));
        stream_recv_empty_buffer(cpu_config_get_socket_interrupt(cpuConfig));
        if (nuevaInterrupcion == HEADER_interrumpir_ejecucion) {
            pthread_mutex_lock(&mutexInterrupcion);
            hayInterrupcion = true;
            pthread_mutex_unlock(&mutexInterrupcion);
            log_info(cpuLogger, "Se recibe de Kernel una petición de interrupción");
        }
    }
}

static void __inicializar_kernel_server(void) {
    hayInterrupcion = false;
    pthread_mutex_init(&mutexInterrupcion, NULL);
    pidProcesoEnExec = -1;
}

void atender_peticiones_de_kernel(void) {
    __inicializar_kernel_server();
    pthread_t interruptTh;
    pthread_create(&interruptTh, NULL, (void*)__interrupt_peticiones_de_kernel, NULL);
    pthread_detach(interruptTh);
    log_info(cpuLogger, "Hilos de atención creados. Listo para atender peticiones de Kernel");
    __dispatch_peticiones_de_kernel();
}
