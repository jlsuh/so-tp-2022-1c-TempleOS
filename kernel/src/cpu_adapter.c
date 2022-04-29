#include "cpu_adapter.h"

#include "buffer.h"
#include "common_flags.h"
#include "kernel_config.h"
#include "stream.h"

void cpu_adapter_enviar_pcb_a_cpu(t_pcb* pcbAEnviar, t_kernel_config* kernelConfig, t_log* kernelLogger) {
    uint32_t pidAEnviar = pcb_get_pid(pcbAEnviar);
    uint64_t pcAEnviar = pcb_get_program_counter(pcbAEnviar);
    uint32_t tablaPagsAEnviar = pcb_get_tabla_pagina_primer_nivel(pcbAEnviar);

    t_buffer* bufferPcbAEjecutar = buffer_create();
    buffer_pack(bufferPcbAEjecutar, &pidAEnviar, sizeof(pidAEnviar));
    buffer_pack(bufferPcbAEjecutar, &pcAEnviar, sizeof(pcAEnviar));
    buffer_pack(bufferPcbAEjecutar, &tablaPagsAEnviar, sizeof(tablaPagsAEnviar));

    stream_send_buffer(kernel_config_get_socket_dispatch_cpu(kernelConfig), HEADER_pcb_a_ejecutar, bufferPcbAEjecutar);
    stream_send_buffer(kernel_config_get_socket_dispatch_cpu(kernelConfig), HEADER_lista_instrucciones, pcb_get_instruction_buffer(pcbAEnviar));

    buffer_destroy(bufferPcbAEjecutar);
}

t_pcb* cpu_adapter_recibir_pcb_de_cpu(t_pcb* pcbARecibir, t_kernel_config* kernelConfig, t_log* kernelLogger) {
    uint32_t pidRecibido = 0, tiempoDeBloqActualizado = 0, pcActualizado = 0, tablaPagsActualizada = 0;

    t_buffer* bufferPcb = buffer_create();
    stream_recv_buffer(kernel_config_get_socket_dispatch_cpu(kernelConfig), bufferPcb);
    buffer_unpack(bufferPcb, &pidRecibido, sizeof(pidRecibido));
    buffer_unpack(bufferPcb, &tiempoDeBloqActualizado, sizeof(tiempoDeBloqActualizado));
    buffer_unpack(bufferPcb, &pcActualizado, sizeof(pcActualizado));
    buffer_unpack(bufferPcb, &tablaPagsActualizada, sizeof(tablaPagsActualizada));
    buffer_destroy(bufferPcb);

    if (pidRecibido == pcb_get_pid(pcbARecibir)) {
        pcb_set_tiempo_de_bloq(pcbARecibir, tiempoDeBloqActualizado);
        pcb_set_program_counter(pcbARecibir, pcActualizado);
        pcb_set_tabla_pagina_primer_nivel(pcbARecibir, tablaPagsActualizada);
    } else {
        log_error(kernelLogger, "Error al recibir PCB de CPU");
    }
    return pcbARecibir;
}

void cpu_adapter_interrumpir_cpu(t_kernel_config* kernelConfig, t_log* kernelLogger) {
    stream_send_empty_buffer(kernel_config_get_socket_interrupt_cpu(kernelConfig), INT_interrumpir_ejecucion);
    uint8_t cpuResponse = stream_recv_header(kernel_config_get_socket_dispatch_cpu(kernelConfig));
    stream_recv_empty_buffer(kernel_config_get_socket_dispatch_cpu(kernelConfig));
    if (cpuResponse != INT_cpu_interrumpida) {
        log_error(kernelLogger, "Error al interrumpir CPU");
    }
    log_info(kernelLogger, "CPU interrumpida");
}
