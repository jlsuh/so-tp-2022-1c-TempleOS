#include <stdint.h>

#include "buffer.h"
#include "common_flags.h"
#include "cpu_config.h"
#include "stream.h"

extern t_cpu_config *cpuConfig;
int socket;
int obtener_marco(uint32_t direccion, uint32_t nroDeTabla1);

void escribir_en_memoria(uint32_t direccion, uint32_t nroDeTabla1, uint32_t valor) {
    int marco = obtener_marco(direccion, nroDeTabla1);
    if (marco == -1) {  // TODO
        printf("No se pudo escribir en memoria\n");
        return;
    }

    t_buffer *buffer = buffer_create();
    uint32_t marcoSend = marco;
    buffer_pack(buffer, &marcoSend, sizeof(marcoSend));
    buffer_pack(buffer, &valor, sizeof(valor));
    stream_send_buffer(socket, HEADER_write, buffer);
}

int obtener_marco(uint32_t direccion, uint32_t nroDeTabla1) {
    socket = cpu_config_get_socket_memoria(cpuConfig);
    int tamPagina = cpu_config_get_tamanio_pagina(cpuConfig);
    int entradasPorTabla = cpu_config_get_entradas_por_tabla(cpuConfig);
    int marcoFinal = -1;
    uint32_t header;

    uint32_t nroPag = direccion / tamPagina;
    uint32_t entradaTablaNivel1 = nroPag / entradasPorTabla;
    uint32_t entradaTablaNivel2 = nroPag % entradasPorTabla;
    uint32_t offset = direccion - nroPag * tamPagina;

    
    t_buffer *bufferSolicitud = buffer_create();
    buffer_pack(bufferSolicitud, &nroDeTabla1, sizeof(nroDeTabla1));
    buffer_pack(bufferSolicitud, &entradaTablaNivel1, sizeof(entradaTablaNivel1));
    stream_send_buffer(socket, HEADER_solicitud_tabla_segundo_nivel, bufferSolicitud);
    buffer_destroy(bufferSolicitud);

    header = stream_recv_header(socket);
    t_buffer *bufferRta = buffer_create();
    uint32_t nroDeTabla2;
    if (header == HEADER_rta_tabla_segundo_nivel) {
        stream_recv_buffer(socket, bufferRta);
        buffer_unpack(bufferRta, &nroDeTabla2, sizeof(nroDeTabla2));
        buffer_destroy(bufferRta);
    } else {
        buffer_destroy(bufferRta);
        return -1;
    }

    bufferSolicitud = buffer_create();
    buffer_pack(bufferSolicitud, &nroDeTabla2, sizeof(nroDeTabla2));
    buffer_pack(bufferSolicitud, &entradaTablaNivel2, sizeof(entradaTablaNivel2));
    stream_send_buffer(socket, HEADER_solicitud_marco, bufferSolicitud);
    buffer_destroy(bufferSolicitud);

    header = stream_recv_header(socket);
    t_buffer *bufferRta = buffer_create();
    uint32_t marco;
    if (header == HEADER_rta_marco) {
        stream_recv_buffer(socket, bufferRta);
        buffer_unpack(bufferRta, &marco, sizeof(marco));
        buffer_destroy(bufferRta);
    } else {
        buffer_destroy(bufferRta);
        return -1;
    }

    return marco + offset;
}