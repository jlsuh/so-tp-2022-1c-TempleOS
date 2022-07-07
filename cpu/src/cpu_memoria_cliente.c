#include "cpu_memoria_cliente.h"

#include <stdint.h>
#include <stdlib.h>

#include "buffer.h"
#include "common_flags.h"
#include "cpu_config.h"
#include "stream.h"

extern t_cpu_config *cpuConfig;
extern t_log *cpuLogger;

static uint32_t __solicitar(int toSocket, uint32_t numeroDeTabla, uint32_t entradaDeTabla, t_header requestHeader) {
    t_buffer *requestBuffer = buffer_create();
    buffer_pack(requestBuffer, &numeroDeTabla, sizeof(numeroDeTabla));
    buffer_pack(requestBuffer, &entradaDeTabla, sizeof(entradaDeTabla));
    stream_send_buffer(toSocket, requestHeader, requestBuffer);
    buffer_destroy(requestBuffer);

    uint32_t responseHeader = stream_recv_header(toSocket);
    if (responseHeader != requestHeader) {
        log_error(cpuLogger, "Error al realizar una solicitud de memoria");
        exit(-1);
    }

    t_buffer *responseBuffer = buffer_create();
    stream_recv_buffer(toSocket, responseBuffer);

    responseHeader = -1;
    buffer_unpack(responseBuffer, &responseHeader, sizeof(responseHeader));
    buffer_destroy(responseBuffer);

    return responseHeader;
}

static uint32_t __obtener_marco(t_tlb* tlb, int toSocket, uint32_t direccion, uint32_t numeroDeTabla1) {
    int tamanioPagina = cpu_config_get_tamanio_pagina(cpuConfig);
    int entradasPorTabla = cpu_config_get_entradas_por_tabla(cpuConfig);

    uint32_t numeroPagina = direccion / tamanioPagina;
    uint32_t entradaTablaNivel1 = numeroPagina / entradasPorTabla;
    uint32_t entradaTablaNivel2 = numeroPagina % entradasPorTabla;
    uint32_t offset = direccion - numeroPagina * tamanioPagina;

    int marco = tlb_get_marco(tlb, numeroPagina);
    if (marco < 0) {
        uint32_t numeroDeTabla2 = __solicitar(toSocket, numeroDeTabla1, entradaTablaNivel1, HEADER_tabla_nivel_2);
        marco = __solicitar(toSocket, numeroDeTabla2, entradaTablaNivel2, HEADER_marco);
        tlb_registrar_entrada_en_tlb(tlb, numeroPagina, marco);
    }
    return marco + offset;
}

void escribir_en_memoria(t_tlb* tlb, int toSocket, uint32_t direccionAEscribir, uint32_t numberoDeTabla1, uint32_t contenidoAEscribir) {
    int marco = __obtener_marco(tlb, toSocket, direccionAEscribir, numberoDeTabla1);

    t_buffer *buffer = buffer_create();
    uint32_t marcoSend = marco;
    buffer_pack(buffer, &marcoSend, sizeof(marcoSend));
    buffer_pack(buffer, &contenidoAEscribir, sizeof(contenidoAEscribir));
    stream_send_buffer(toSocket, HEADER_write, buffer);
    buffer_destroy(buffer);
}

uint32_t leer_en_memoria(t_tlb* tlb, int toSocket, uint32_t direccionALeer, uint32_t numberoDeTabla1) {
    int marco = __obtener_marco(tlb, toSocket, direccionALeer, numberoDeTabla1);

    t_buffer *requestBuffer = buffer_create();
    uint32_t marcoAEnviar = marco;
    buffer_pack(requestBuffer, &marcoAEnviar, sizeof(marcoAEnviar));
    stream_send_buffer(toSocket, HEADER_read, requestBuffer);
    buffer_destroy(requestBuffer);

    uint32_t responseHeader = stream_recv_header(toSocket);
    if (responseHeader != HEADER_read) {
        log_error(cpuLogger, "Error al leer en memoria");
        exit(-1);
    }
    t_buffer *responseBuffer = buffer_create();
    stream_recv_buffer(toSocket, responseBuffer);
    uint32_t contenidoLeido = -1;
    buffer_unpack(responseBuffer, &contenidoLeido, sizeof(contenidoLeido));
    buffer_destroy(responseBuffer);
    return contenidoLeido;
}
