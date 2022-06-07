#include "mem_adapter.h"

#include <stdlib.h>

#include "buffer.h"
#include "common_flags.h"
#include "kernel_config.h"
#include "stream.h"

int mem_adapter_obtener_tabla_pagina(t_pcb* pcbAIniciar, t_kernel_config* kernelConfig, t_log* kernelLogger) {
    /* uint32_t newTamanio = pcb_get_tamanio(pcbAIniciar);

    t_buffer* bufferNuevaTablaPagina = buffer_create();
    buffer_pack(bufferNuevaTablaPagina, &newTamanio, sizeof(newTamanio));

    // Enviamos la petición a Memoria con la info necesaria
    stream_send_buffer(kernel_config_get_socket_memoria(kernelConfig), HEADER_solicitud_tabla_paginas, bufferNuevaTablaPagina);
    buffer_destroy(bufferNuevaTablaPagina);

    // Esperamos que la Memoria nos devuelva lo necesario
    uint32_t nroTabla = 0;
    uint8_t memoriaResponse = stream_recv_header(kernel_config_get_socket_memoria(kernelConfig));
    if (memoriaResponse == HEADER_tabla_de_paginas) {
        t_buffer* bufferTabla = buffer_create();
        stream_recv_buffer(kernel_config_get_socket_memoria(kernelConfig), bufferTabla);

        buffer_unpack(bufferTabla, &nroTabla, sizeof(nroTabla));
        buffer_destroy(bufferTabla);

        log_info(kernelLogger, "Proceso: %d - Tabla de página de primer nivel: %d", pcb_get_pid(pcbAIniciar), nroTabla);
    } else if (memoriaResponse == HEADER_memoria_insuficiente) {
        return -1;
    } else {
        log_error(kernelLogger, "Error al recibir buffer tabla de páginas");
        exit(-1);
    }

    return nroTabla; */

    return 1;
}

void mem_adapter_finalizar_proceso(t_pcb* pcbAFinalizar, t_kernel_config* kernelConfig, t_log* kernelLogger) {
    /* uint32_t tablaATerminar = pcb_get_tabla_pagina_primer_nivel(pcbAFinalizar);

    t_buffer* bufferPcbAFinalizar = buffer_create();
    buffer_pack(bufferPcbAFinalizar, &tablaATerminar, sizeof(tablaATerminar));

    stream_send_buffer(kernel_config_get_socket_memoria(kernelConfig), HEADER_proceso_terminado, bufferPcbAFinalizar);
    buffer_destroy(bufferPcbAFinalizar);

    uint8_t memoriaResponse = stream_recv_header(kernel_config_get_socket_memoria(kernelConfig));
    if (memoriaResponse == HEADER_proceso_terminado) {
        log_info(kernelLogger, "Proceso %d finalizado correctamente en Memoria", pcb_get_pid(pcbAFinalizar));
    } else {
        log_error(kernelLogger, "Error al finalizar proceso en Memoria");
        exit(-1);
    } */
    return;
}

void mem_adapter_avisar_suspension(t_pcb* pcbASuspender, t_kernel_config* kernelConfig, t_log* kernelLogger) {
   /* uint32_t tablaASuspender = pcb_get_tabla_pagina_primer_nivel(pcbASuspender);
    uint32_t pidASuspender = pcb_get_pid(pcbASuspender);

    t_buffer* bufferPcbASuspender = buffer_create();
    buffer_pack(bufferPcbASuspender, &tablaASuspender, sizeof(tablaASuspender));
    buffer_pack(bufferPcbASuspender, &tablaASuspender, sizeof(pidASuspender));

    stream_send_buffer(kernel_config_get_socket_memoria(kernelConfig), HEADER_proceso_suspendido, bufferPcbASuspender);
    buffer_destroy(bufferPcbASuspender);

    uint8_t memoriaResponse = stream_recv_header(kernel_config_get_socket_memoria(kernelConfig));
    if (memoriaResponse == HEADER_proceso_suspendido) {
        log_info(kernelLogger, "Proceso %d suspendido correctamente en Memoria", pcb_get_pid(pcbASuspender));
    } else {
        log_error(kernelLogger, "Error al intentar suspender un proceso en Memoria");
        exit(-1);
    } */

    return;
}
