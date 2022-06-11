#include <stdint.h>
#include <stdlib.h>

#include "archivo.h"
#include "common_flags.h"
#include "marcos.h"
#include "memoria_data_holder.h"
#include "stream.h"
#include "tabla_nivel_1.h"

extern t_memoria_data_holder memoriaData;

static uint32_t __crear_nuevo_proceso(uint32_t tamanio, t_memoria_data_holder memoriaData) {
    uint32_t indiceTablaNivel1 = obtener_tabla_libre_de_nivel_1(memoriaData);
    uint32_t nroTablaNivel1 = asignar_tabla_nivel_1(indiceTablaNivel1, tamanio, memoriaData);
    crear_archivo_de_proceso(tamanio, nroTablaNivel1, memoriaData);

    return nroTablaNivel1;
}

void* escuchar_peticiones_kernel(void* socketKernel) {
    int socket = *(int*)socketKernel;
    free(socketKernel);

    uint32_t header, tablaNivel1;
    for (;;) {
        header = stream_recv_header(socket);
        t_buffer* buffer = buffer_create();
        stream_recv_buffer(socket, buffer);

        switch (header) {
            case HEADER_solicitud_tabla_paginas: {
                uint32_t tamanio;
                buffer_unpack(buffer, &tamanio, sizeof(tamanio));

                // FIX: Mock
                uint32_t nroTablaNivel1 = tamanio / 5;
                t_buffer* buffer_rta = buffer_create();
                buffer_pack(buffer_rta, &nroTablaNivel1, sizeof(nroTablaNivel1));
                stream_send_buffer(socket, HANDSHAKE_ok_continue, buffer_rta);
                buffer_destroy(buffer_rta);
                log_info(memoriaData.memoriaLogger, "Kernel: Se ha solicitado una tabla de páginas de %d bytes, asignando la tabla %d", tamanio, nroTablaNivel1);

                // if (__se_puede_crear_proceso(tamanio, memoriaData)) {  // TODO funcion que analize si por tamaño entra y si hay tabla libre de lvl1
                //     uint32_t nroTablaNivel1 = __crear_nuevo_proceso(tamanio, memoriaData);
                //     t_buffer* buffer_rta = buffer_create();
                //     buffer_pack(buffer_rta, &nroTablaNivel1, sizeof(nroTablaNivel1));
                //     stream_send_buffer(socket, HANDSHAKE_ok_continue, buffer_rta);
                //     buffer_destroy(buffer_rta);
                // } else {
                //     stream_send_empty_buffer(socket, HEADER_error);
                // }

                buffer_destroy(buffer);

                break;
            }
            case HEADER_proceso_suspendido:
                buffer_unpack(buffer, &tablaNivel1, sizeof(tablaNivel1));

                // Liberar memoria del proceso con swap... //TODO

                stream_send_empty_buffer(socket, HANDSHAKE_ok_continue);
                break;
            case HEADER_proceso_terminado:
                buffer_unpack(buffer, &tablaNivel1, sizeof(tablaNivel1));

                // Liberar al proceso de memoria y de swap... //TODO

                stream_send_empty_buffer(socket, HANDSHAKE_ok_continue);
                break;
            default:
                break;
        }
    }

    return NULL;
}