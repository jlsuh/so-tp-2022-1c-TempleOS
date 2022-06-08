#include <stdint.h>
#include <stdlib.h>

#include "archivo.h"
#include "common_flags.h"
#include "marcos.h"
#include "memoria_data_holder.h"
#include "stream.h"
#include "tabla_nivel_1.h"

extern t_memoria_data_holder memoriaData;

uint32_t __crear_nuevo_proceso(uint32_t tamanio, t_memoria_data_holder memoriaData);

void* escuchar_peticiones_kernel(void* socketKernel) {
    int socket = *(int*)socketKernel;
    free(socketKernel);

    uint32_t header;
    for (;;) {
        header = stream_recv_header(socket);
        t_buffer* buffer = buffer_create();
        stream_recv_buffer(socket, buffer);

        switch (header) {
            case HEADER_solicitud_tabla_paginas: {
                uint32_t tamanio;
                buffer_unpack(buffer, &tamanio, sizeof(tamanio));

                int procesoNuevo = __crear_nuevo_proceso(tamanio, memoriaData);
                uint32_t nroTablaNivel1 = procesoNuevo;
                t_buffer* buffer_rta = buffer_create();
                buffer_pack(buffer_rta, &nroTablaNivel1, sizeof(nroTablaNivel1));
                stream_send_buffer(socket, HEADER_tabla_de_paginas, buffer_rta);
                // TODO cachear errores?
                break;
            }
            case HEADER_proceso_suspendido:
                // Liberar memoria del proceso con swap... //TODO
                stream_send_empty_buffer(socket, HANDSHAKE_ok_continue);
                break;
            case HEADER_proceso_terminado:
                // Liberar al proceso de memoria y de swap... //TODO
                stream_send_empty_buffer(socket, HANDSHAKE_ok_continue);
                break;
            default:
                break;
        }
    }

    return NULL;
}

uint32_t __crear_nuevo_proceso(uint32_t tamanio, t_memoria_data_holder memoriaData) {
    uint32_t indiceTablaNivel1 = obtener_tabla_libre_de_nivel_1(memoriaData);
    uint32_t nroTablaNivel1 = asignar_tabla_nivel_1(indiceTablaNivel1, tamanio, memoriaData);
    crear_archivo_de_proceso(tamanio, memoriaData.pathSwap, nroTablaNivel1);

    // TODO cachear errores? Como que haya mas procesos en mulitprogramación y supere la cantidad admitida en memoria o que el tamanio del proceso sea mayor al que se le puede asginar

    return nroTablaNivel1;
}