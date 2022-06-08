#include <stdint.h>
#include <stdlib.h>

#include "common_flags.h"
#include "marcos.h"
#include "stream.h"
#include "tabla_nivel_1.h"
#include "archivo.h"

uint32_t __crear_nuevo_proceso(char* pathSwap, uint32_t tamanio, int entradasPorTabla, int tamanioPagina, int cantidadProcesosMax, int cantidadMarcosMax, int cantidadMarcosProceso, t_tabla_nivel_1* tablasDeNivel1, t_marcos* marcos);

extern t_tabla_nivel_1* tablasDeNivel1;
extern t_marcos* marcos;
extern int tamanioPagina;
extern int entradasPorTabla;
extern int cantidadProcesosMax;
extern int cantidadMarcosMax;
extern int cantidadMarcosProceso;
extern char* pathSwap;
extern int contadorTabla1;

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

                int procesoNuevo = __crear_nuevo_proceso(pathSwap, tamanio, entradasPorTabla, tamanioPagina, cantidadProcesosMax, cantidadMarcosMax, cantidadMarcosProceso, tablasDeNivel1, marcos);

                if (procesoNuevo == -1) {
                    // log_error(memoriaLogger, "No se pudo crear el proceso"); //TODO extern
                    stream_send_empty_buffer(socket, HEADER_error);
                } else {
                    uint32_t nroTablaNivel1 = procesoNuevo;
                    t_buffer* buffer_rta = buffer_create();
                    buffer_pack(buffer_rta, &nroTablaNivel1, sizeof(nroTablaNivel1));
                    stream_send_buffer(socket, HEADER_tabla_de_paginas, buffer_rta);
                }

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

uint32_t __crear_nuevo_proceso(char* pathSwap, uint32_t tamanio, int entradasPorTabla, int tamanioPagina, int cantidadProcesosMax, int cantidadMarcosMax, int cantidadMarcosProceso, t_tabla_nivel_1* tablasDeNivel1, t_marcos* marcos) {
    
    uint32_t indiceTablaNivel1 = obtener_tabla_libre_de_nivel_1(cantidadProcesosMax, tablasDeNivel1);
    uint32_t nroTablaNivel1 = asignar_tabla_nivel_1(indiceTablaNivel1, tamanio, tablasDeNivel1, contadorTabla1);
    crear_archivo_de_proceso(tamanio, pathSwap, nroTablaNivel1);

    //TODO cachear errores? Como que haya mas procesos en mulitprogramación y supere la cantidad admitida en memoria o que el tamanio del proceso sea mayor al que se le puede asginar 

    return nroTablaNivel1;
}