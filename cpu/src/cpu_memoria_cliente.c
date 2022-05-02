#include <stdint.h>

#include "buffer.h"
#include "common_flags.h"
#include "cpu_config.h"
#include "stream.h"

extern t_cpu_config *cpuConfig;
int socket;
uint32_t obtener_marco(uint32_t direccion, uint32_t nroDeTabla1);

void escribir_en_memoria(uint32_t direccion, uint32_t nroDeTabla1, uint32_t valor) {
    int marco = obtener_marco(direccion, nroDeTabla1);

    t_buffer *buffer = buffer_create();
    uint32_t marcoSend = marco;
    buffer_pack(buffer, &marcoSend, sizeof(marcoSend));
    buffer_pack(buffer, &valor, sizeof(valor));
    stream_send_buffer(socket, HEADER_write, buffer);
    buffer_destroy(buffer);
}

uint32_t leer_en_memoria(uint32_t direccion, uint32_t nroDeTabla1) {
    int marco = obtener_marco(direccion, nroDeTabla1);

    t_buffer *buffer = buffer_create();
    uint32_t marcoSend = marco;
    buffer_pack(buffer, &marcoSend, sizeof(marcoSend));
    stream_send_buffer(socket, HEADER_write, buffer);
    buffer_destroy(buffer);

    uint32_t header = stream_recv_header(socket);
    if (header != HEADER_read) {
        printf("Error al leer en memoria\n"); // TODO que hacer?
        return -1;
    }
    t_buffer *bufferRecv = buffer_create();
    uint32_t valor;
    buffer_unpack(bufferRecv, &valor, sizeof(valor));
    buffer_destroy(bufferRecv);
    return valor;
}

void copiar_en_memoria(uint32_t direccionDestino, uint32_t direccionOrigen, uint32_t nroDeTabla1) {
    int marcoDestino = obtener_marco(direccionDestino, nroDeTabla1);
    int marcoOrigen = obtener_marco(direccionOrigen, nroDeTabla1);

    t_buffer *buffer = buffer_create();
    uint32_t marcoDestSend = marcoDestino;
    uint32_t marcoOrigenSend = marcoOrigen;
    buffer_pack(buffer, &marcoDestSend, sizeof(marcoDestSend));
    buffer_pack(buffer, &marcoOrigenSend, sizeof(marcoOrigenSend));
    stream_send_buffer(socket, HEADER_copy, buffer);
    buffer_destroy(buffer);
}

uint32_t solicitar(uint32_t nroDeTabla, uint32_t entradaDeTabla, t_header header){
    t_buffer *bufferSolicitud = buffer_create();
    buffer_pack(bufferSolicitud, &nroDeTabla, sizeof(nroDeTabla));
    buffer_pack(bufferSolicitud, &entradaDeTabla, sizeof(entradaDeTabla));
    stream_send_buffer(socket, header, bufferSolicitud);
    buffer_destroy(bufferSolicitud);

    uint32_t headerRta = stream_recv_header(socket);
    t_buffer *bufferRta = buffer_create();
    stream_recv_buffer(socket, bufferRta);
    uint32_t rta;
    if (headerRta != header) {
        return -1; // TODO: que hacer?
    }
    buffer_unpack(bufferRta, &rta, sizeof(rta));
    buffer_destroy(bufferRta);
    return rta;
}

uint32_t obtener_marco(uint32_t direccion, uint32_t nroDeTabla1) {
    socket = cpu_config_get_socket_memoria(cpuConfig);
    int tamPagina = cpu_config_get_tamanio_pagina(cpuConfig);
    int entradasPorTabla = cpu_config_get_entradas_por_tabla(cpuConfig);
    uint32_t header;

    uint32_t nroPag = direccion / tamPagina;
    uint32_t entradaTablaNivel1 = nroPag / entradasPorTabla;
    uint32_t entradaTablaNivel2 = nroPag % entradasPorTabla;
    uint32_t offset = direccion - nroPag * tamPagina;

    uint32_t nroDeTabla2 = solicitar(nroDeTabla1, entradaTablaNivel1, HEADER_tabla_nivel_2);
    uint32_t marco = solicitar(nroDeTabla2, entradaTablaNivel2, HEADER_marco);

    return marco + offset;
}