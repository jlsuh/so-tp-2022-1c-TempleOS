#include <stdint.h>
#include <stdlib.h>

#include "archivo.h"
#include "common_flags.h"
#include "marcos.h"
#include "memoria_data_holder.h"
#include "stream.h"
#include "tabla_nivel_1.h"
#include "tabla_nivel_2.h"
#include "tabla_suspendido.h"

extern t_memoria_data_holder* memoriaData;

static uint32_t __crear_nuevo_proceso(uint32_t tamanio, t_memoria_data_holder* memoriaData) {
    uint32_t indiceTablaNivel1 = obtener_tabla_libre_de_nivel_1(memoriaData);
    uint32_t nroTablaNivel1 = asignar_tabla_nivel_1(indiceTablaNivel1, tamanio, memoriaData);
    crear_archivo_de_proceso(tamanio, nroTablaNivel1, memoriaData);

    return nroTablaNivel1;
}

static void __eliminar_proceso(uint32_t nroTablaNivel1, t_memoria_data_holder* memoriaData){
    eliminar_archivo_de_proceso(nroTablaNivel1, memoriaData);
    for(int i = 0; i < memoriaData->entradasPorTabla; i++){
        int nroDeTabla2 = obtener_tabla_de_nivel_2(nroTablaNivel1, i, memoriaData);
        limpiar_tabla_nivel_2(nroDeTabla2, memoriaData);
    }
    limpiar_tabla_nivel_1(nroTablaNivel1, memoriaData);
}

static bool __se_puede_crear_proceso(uint32_t tamanio, t_memoria_data_holder* memoriaData) {
    return tamanio <= memoriaData->tamanioMaxArchivo && hay_tabla_nivel_1_disponible(memoriaData);
}

void* escuchar_peticiones_kernel(void* socketKernel) {
    int socket = *(int*)socketKernel;
    // free(socketKernel);

    uint32_t header, tablaNivel1;
    for (;;) {
        header = stream_recv_header(socket);  // NEW -> READY // BLOCKED -> SUSBLOCKED
        t_buffer* buffer = buffer_create();
        stream_recv_buffer(socket, buffer);

        switch (header) {
            case HEADER_solicitud_tabla_paginas: {
                uint32_t tamanio;
                buffer_unpack(buffer, &tamanio, sizeof(tamanio));

                // FIX: Mock
                // uint32_t nroTablaNivel1 = tamanio / 5;
                // t_buffer* buffer_rta = buffer_create();
                // buffer_pack(buffer_rta, &nroTablaNivel1, sizeof(nroTablaNivel1));
                // stream_send_buffer(socket, HANDSHAKE_ok_continue, buffer_rta);
                // buffer_destroy(buffer_rta);

                if (__se_puede_crear_proceso(tamanio, memoriaData)) {
                    uint32_t nroTablaNivel1 = __crear_nuevo_proceso(tamanio, memoriaData);
                    t_buffer* buffer_rta = buffer_create();
                    buffer_pack(buffer_rta, &nroTablaNivel1, sizeof(nroTablaNivel1));
                    stream_send_buffer(socket, HANDSHAKE_ok_continue, buffer_rta);
                    buffer_destroy(buffer_rta);
                } else {
                    stream_send_empty_buffer(socket, HEADER_error);
                }

                buffer_destroy(buffer);

                break;
            }
            case HEADER_proceso_suspendido:
                buffer_unpack(buffer, &tablaNivel1, sizeof(tablaNivel1));

                suspender_proceso(tablaNivel1, memoriaData);

                stream_send_empty_buffer(socket, HANDSHAKE_ok_continue);
                buffer_destroy(buffer);
                break;
            case HEADER_proceso_terminado:
                buffer_unpack(buffer, &tablaNivel1, sizeof(tablaNivel1));

                __eliminar_proceso(tablaNivel1, memoriaData);

                stream_send_empty_buffer(socket, HANDSHAKE_ok_continue);
                buffer_destroy(buffer);
                break;
            default:
                exit(-1);
                break;
        }
    }

    return NULL;
}