#include "cpu_adapter.h"

#include <stdlib.h>

#include "buffer.h"
#include "common_flags.h"
#include "kernel_config.h"
#include "stream.h"

void cpu_adapter_enviar_pcb_a_cpu(t_pcb* pcbAEnviar, t_kernel_config* kernelConfig, t_log* kernelLogger) {
    uint32_t pidAEnviar = pcb_get_pid(pcbAEnviar);
    uint32_t pcAEnviar = pcb_get_program_counter(pcbAEnviar);
    uint32_t tablaPagsAEnviar = pcb_get_tabla_pagina_primer_nivel(pcbAEnviar);

    t_buffer* bufferPcbAEjecutar = buffer_create();
    buffer_pack(bufferPcbAEjecutar, &pidAEnviar, sizeof(pidAEnviar));
    buffer_pack(bufferPcbAEjecutar, &pcAEnviar, sizeof(pcAEnviar));
    buffer_pack(bufferPcbAEjecutar, &tablaPagsAEnviar, sizeof(tablaPagsAEnviar));

    stream_send_buffer(kernel_config_get_socket_dispatch_cpu(kernelConfig), HEADER_pcb_a_ejecutar, bufferPcbAEjecutar);
    stream_send_buffer(kernel_config_get_socket_dispatch_cpu(kernelConfig), HEADER_lista_instrucciones, pcb_get_instruction_buffer(pcbAEnviar));

    buffer_destroy(bufferPcbAEjecutar);
}

t_pcb* cpu_adapter_recibir_pcb_actualizado_de_cpu(t_pcb* pcbAActualizar, uint8_t cpuResponse, t_kernel_config* kernelConfig, t_log* kernelLogger) {
    uint32_t pidRecibido = 0;
    uint32_t programCounterActualizado = 0;
    uint32_t tablaPagsActualizada = 0;
    uint32_t tiempoDeBloqActualizado = 0;

    t_buffer* bufferPcb = buffer_create();
    stream_recv_buffer(kernel_config_get_socket_dispatch_cpu(kernelConfig), bufferPcb);
    buffer_unpack(bufferPcb, &pidRecibido, sizeof(pidRecibido));
    buffer_unpack(bufferPcb, &programCounterActualizado, sizeof(programCounterActualizado));
    buffer_unpack(bufferPcb, &tablaPagsActualizada, sizeof(tablaPagsActualizada));

    if (pidRecibido == pcb_get_pid(pcbAActualizar)) {
        if (cpuResponse == HEADER_proceso_bloqueado) {
            buffer_unpack(bufferPcb, &tiempoDeBloqActualizado, sizeof(tiempoDeBloqActualizado));
            pcb_set_tiempo_de_bloqueo(pcbAActualizar, tiempoDeBloqActualizado);
        }
        pcb_set_program_counter(pcbAActualizar, programCounterActualizado);
        pcb_set_tabla_pagina_primer_nivel(pcbAActualizar, tablaPagsActualizada);
    } else {
        log_error(kernelLogger, "Error al recibir PCB de CPU");
        exit(-1);
    }
    buffer_destroy(bufferPcb);
    return pcbAActualizar;
}

void cpu_adapter_interrumpir_cpu(t_kernel_config* kernelConfig, t_log* kernelLogger) {
    stream_send_empty_buffer(kernel_config_get_socket_interrupt_cpu(kernelConfig), INT_interrumpir_ejecucion);
    log_info(kernelLogger, "Se envía interrupción a CPU");
}
