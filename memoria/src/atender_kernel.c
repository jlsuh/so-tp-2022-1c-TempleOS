#include <pthread.h>
#include <stdlib.h>

#include "archivo.h"
#include "common_flags.h"
#include "stream.h"
#include "tabla_nivel_1.h"
#include "tabla_nivel_2.h"
#include "tabla_suspendido.h"

extern pthread_mutex_t mutexMemoriaData;
extern t_memoria_data_holder* memoriaData;

static uint32_t __crear_nuevo_proceso(uint32_t tamanio, t_memoria_data_holder* memoriaData) {
    uint32_t indiceTablaNivel1 = obtener_tabla_libre_de_nivel_1(memoriaData);
    uint32_t nroTablaNivel1 = asignar_tabla_nivel_1(indiceTablaNivel1, tamanio, memoriaData);
    crear_archivo_de_proceso(tamanio, nroTablaNivel1, memoriaData);
    return nroTablaNivel1;
}

static void __eliminar_proceso(uint32_t nroTablaNivel1, t_memoria_data_holder* memoriaData) {
    eliminar_archivo_de_proceso(nroTablaNivel1, memoriaData);
    if (esta_suspendido(nroTablaNivel1, memoriaData)) {
        eliminar_de_tabla_suspendidos(nroTablaNivel1, memoriaData);
    } else {
        for (int i = 0; i < memoriaData->entradasPorTabla; i++) {
            int nroDeTabla2 = obtener_tabla_de_nivel_2(nroTablaNivel1, i, memoriaData);
            limpiar_tabla_nivel_2(nroDeTabla2, memoriaData);
        }
        limpiar_tabla_nivel_1(nroTablaNivel1, memoriaData);
    }
}

static bool __se_puede_crear_proceso(uint32_t tamanio, t_memoria_data_holder* memoriaData) {
    return tamanio <= memoriaData->tamanioMaxArchivo && hay_tabla_nivel_1_disponible(memoriaData);
}

void* escuchar_peticiones_kernel(void* socketKernel) {
    int socket = *(int*)socketKernel;
    uint32_t header, nroTablaNivel1;
    for (;;) {
        header = stream_recv_header(socket);
        pthread_mutex_lock(&mutexMemoriaData);
        t_buffer* buffer = buffer_create();
        stream_recv_buffer(socket, buffer);
        switch (header) {
            case HEADER_solicitud_tabla_paginas: {
                log_info(memoriaData->memoriaLogger, "\e[1;93mSe crea nuevo proceso\e[0m");
                uint32_t tamanio;
                buffer_unpack(buffer, &tamanio, sizeof(tamanio));
                if (__se_puede_crear_proceso(tamanio, memoriaData)) {
                    nroTablaNivel1 = __crear_nuevo_proceso(tamanio, memoriaData);
                    t_buffer* buffer_rta = buffer_create();
                    buffer_pack(buffer_rta, &nroTablaNivel1, sizeof(nroTablaNivel1));
                    stream_send_buffer(socket, HANDSHAKE_ok_continue, buffer_rta);
                    buffer_destroy(buffer_rta);
                    log_info(memoriaData->memoriaLogger, "Se asigno correctamente una tabla de nivel 1 con ID [%d] y tamaño [%d]", nroTablaNivel1, tamanio);
                } else {
                    stream_send_empty_buffer(socket, HEADER_error);
                    log_error(memoriaData->memoriaLogger, "No se pudo asignar tabla de nivel 1 con tamaño [%d]", tamanio);
                }
                buffer_destroy(buffer);
                break;
            }
            case HEADER_proceso_suspendido:
                log_info(memoriaData->memoriaLogger, "\e[1;93mSe suspende proceso\e[0m");
                buffer_unpack(buffer, &nroTablaNivel1, sizeof(nroTablaNivel1));
                if (esta_suspendido(nroTablaNivel1, memoriaData)) {
                    log_info(memoriaData->memoriaLogger, "\e[1;92mYa se encuentra suspendido el proceso [%d]\e[0m", nroTablaNivel1);
                } else {
                    log_info(memoriaData->memoriaLogger, "\e[1;92mSe suspendio el proceso [%d]\e[0m", nroTablaNivel1);
                    suspender_proceso(nroTablaNivel1, memoriaData);
                }
                stream_send_empty_buffer(socket, HANDSHAKE_ok_continue);
                buffer_destroy(buffer);
                break;
            case HEADER_proceso_terminado:
                log_info(memoriaData->memoriaLogger, "\e[1;93mSe finaliza proceso\e[0m");
                buffer_unpack(buffer, &nroTablaNivel1, sizeof(nroTablaNivel1));
                __eliminar_proceso(nroTablaNivel1, memoriaData);
                log_info(memoriaData->memoriaLogger, "Se finalizó tabla de nivel 1 con ID [%d]", nroTablaNivel1);
                stream_send_empty_buffer(socket, HANDSHAKE_ok_continue);
                buffer_destroy(buffer);
                break;
            default:
                exit(-1);
                break;
        }
        pthread_mutex_unlock(&mutexMemoriaData);
    }
    return NULL;
}
