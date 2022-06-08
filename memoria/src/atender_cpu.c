#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "common_flags.h"
#include "marcos.h"
#include "stream.h"
#include "tabla_nivel_1.h"
#include "tabla_nivel_2.h"

extern void* memoriaPrincipal;
extern t_tabla_nivel_1* tablasDeNivel1;
extern t_tabla_nivel_2* tablasDeNivel2;
extern t_marcos* marcos;
extern int tamanioPagina;
extern int entradasPorTabla;
extern int cantidadProcesosMax;

void __actualizar_pagina(uint32_t direccionFisica, bool esEscritura, int tamanioPagina, int entradasPorTabla, t_marcos* marcos, t_tabla_nivel_1* tablasDeNivel1, t_tabla_nivel_2* tablasDeNivel2);
int __obtener_marco(uint32_t nroDeTabla2, uint32_t entradaDeTabla2, t_tabla_nivel_2* tablasDeNivel2, int cantidadProcesosMax, int entradasPorTabla);

void* escuchar_peticiones_cpu(void* socketCpu) {
    int socket = *(int*)socketCpu;
    free(socketCpu);

    uint32_t header, direccionFisica, direccionFisicaOrigen, valor;
    t_buffer* buffer;

    for (;;) {
        header = stream_recv_header(socket);
        buffer = buffer_create();
        stream_recv_buffer(socket, buffer);

        switch (header) {
            case HEADER_read:
                buffer_unpack(buffer, &direccionFisica, sizeof(direccionFisica));

                memcpy(&valor, memoriaPrincipal + direccionFisica, sizeof(valor));

                t_buffer* buffer_rta = buffer_create();
                buffer_pack(buffer_rta, &valor, sizeof(valor));
                stream_send_buffer(socket, HEADER_read, buffer_rta);
                buffer_destroy(buffer_rta);

                __actualizar_pagina(direccionFisica, false, tamanioPagina, entradasPorTabla, marcos, tablasDeNivel1, tablasDeNivel2);

                break;
            case HEADER_write:
                buffer_unpack(buffer, &direccionFisica, sizeof(direccionFisica));
                buffer_unpack(buffer, &valor, sizeof(valor));

                memcpy(memoriaPrincipal + direccionFisica, &valor, sizeof(valor));

                __actualizar_pagina(direccionFisica, true, tamanioPagina, entradasPorTabla, marcos, tablasDeNivel1, tablasDeNivel2);

                // TODO send ok?
                break;
            case HEADER_copy:
                buffer_unpack(buffer, &direccionFisica, sizeof(direccionFisica));
                buffer_unpack(buffer, &direccionFisicaOrigen, sizeof(direccionFisicaOrigen));

                memcpy(memoriaPrincipal + direccionFisica, memoriaPrincipal + direccionFisicaOrigen, sizeof(uint32_t));

                __actualizar_pagina(direccionFisica, true, tamanioPagina, entradasPorTabla, marcos, tablasDeNivel1, tablasDeNivel2);
                __actualizar_pagina(direccionFisicaOrigen, false, tamanioPagina, entradasPorTabla, marcos, tablasDeNivel1, tablasDeNivel2);

                // TODO send ok?
                break;
            case HEADER_tabla_nivel_2: {
                uint32_t nroDeTabla1, entradaDeTabla1;
                buffer_unpack(buffer, &nroDeTabla1, sizeof(nroDeTabla1));
                buffer_unpack(buffer, &entradaDeTabla1, sizeof(entradaDeTabla1));

                int nroDeTabla2 = obtener_tabla_de_nivel_2(nroDeTabla1, entradaDeTabla1, cantidadProcesosMax, tablasDeNivel1);
                if (nroDeTabla2 == -1) {
                    traer_de_suspendidos(nroDeTabla1);  // TODO
                    nroDeTabla2 = obtener_tabla_de_nivel_2(nroDeTabla1, entradaDeTabla1, cantidadProcesosMax, tablasDeNivel1);
                }

                uint32_t nroDeTabla2send = nroDeTabla2;

                t_buffer* buffer_rta = buffer_create();
                buffer_pack(buffer_rta, &nroDeTabla2send, sizeof(nroDeTabla2send));
                stream_send_buffer(socket, HEADER_tabla_nivel_2, buffer_rta);
                buffer_destroy(buffer_rta);

                break;
            }
            case HEADER_marco: {  // TODO
                uint32_t nroDeTabla2, entradaDeTabla2;
                buffer_unpack(buffer, &nroDeTabla2, sizeof(nroDeTabla2));
                buffer_unpack(buffer, &entradaDeTabla2, sizeof(entradaDeTabla2));

                int indiceMarco = __obtener_marco(nroDeTabla2, entradaDeTabla2, tablasDeNivel2, cantidadProcesosMax, entradasPorTabla);
                uint32_t marco = indiceMarco * tamanioPagina;

                t_buffer* buffer_rta = buffer_create();
                buffer_pack(buffer_rta, &marco, sizeof(marco));
                stream_send_buffer(socket, HEADER_marco, buffer_rta);
                buffer_destroy(buffer_rta);

                break;
            }
            default:
                break;
        }
        buffer_destroy(buffer);
    }

    return NULL;
}

void __actualizar_pagina(uint32_t direccionFisica, bool esEscritura, int tamanioPagina, int entradasPorTabla, t_marcos* marcos, t_tabla_nivel_1* tablasDeNivel1, t_tabla_nivel_2* tablasDeNivel2) {
    int nroPagina = obtener_pagina_de_un_marco(direccionFisica, tamanioPagina, marcos);
    int nroTablaNivel2 = obtener_tabla_de_nivel_2_pagina(nroPagina, entradasPorTabla, tablasDeNivel1);
    if (esEscritura)
        actualizar_escritura_pagina(nroPagina, nroTablaNivel2, entradasPorTabla, tablasDeNivel2);
    else
        actualizar_lectura_pagina(nroPagina, nroTablaNivel2, entradasPorTabla, tablasDeNivel2);
}

int __obtener_marco(uint32_t nroDeTabla2, uint32_t entradaDeTabla2, t_tabla_nivel_2* tablasDeNivel2, int cantidadProcesosMax, int entradasPorTabla) {
    uint32_t nroTablaNivel1 = obtener_tabla_de_nivel_1(nroDeTabla2, cantidadProcesosMax, entradasPorTabla, tablasDeNivel1);
    int marco = obtener_marco_libre(nroTablaNivel1, tablasDeNivel1);

    marco = obtener_marco(nroDeTabla2, entradaDeTabla2, tablasDeNivel2);  // TODO capaz usar el bit de presencia, para darle el sentido que corresponde
    if (marco == -1) {
        // TODO disponible? asignación directo sin swap sino swap, con la tabla de nivel 1 ver si hay algun marco con bool enUso en false.

        // TODO ver con cami el algoritmo para seleccionar la victima
        int nroDeTabla2Victima = 0, entradaDeTabla2Victima = 0;

        int nroDeTabla1 = obtener_tabla_de_nivel_1(nroDeTabla2, cantidadProcesosMax, entradasPorTabla, tablasDeNivel2);
        abrir_archivo(nroDeTabla1);
        marco = __swap_marco(nroDeTabla2Victima, entradaDeTabla2Victima, nroDeTabla2, entradaDeTabla2);
        cerrar_archivo();
    }

    return marco;
}
int __swap_marco(int nroDeTabla2Victima, int entradaDeTabla2Victima, uint32_t nroDeTabla2, uint32_t entradaDeTabla2) {
    int marco = obtener_marco(nroDeTabla2Victima, entradaDeTabla2Victima, tablasDeNivel2);
    actualizar_swap_out(nroDeTabla2Victima, entradaDeTabla2Victima, tablasDeNivel2);
    tablasDeNivel2[nroDeTabla2ToSwap].entradaNivel2[entradaDeTabla2ToSwap].bitPresencia = 0;

    int paginaToSwap = nroDeTabla2ToSwap * entradasPorTabla + entradaDeTabla2ToSwap;
    int pagina = nroDeTabla2 * entradasPorTabla + entradaDeTabla2;
    int tiempoDeEspera = memoria_config_get_retardo_swap(memoriaConfig);

    // Escribir en archivo
    sleep(tiempoDeEspera);
    memcpy(inicio_archivo + tamanioPagina * paginaToSwap, memoriaPrincipal + marco * tamanioPagina, tamanioPagina);
    // Leer de archivo
    sleep(tiempoDeEspera);
    memcpy(memoriaPrincipal + marco * tamanioPagina, inicio_archivo + tamanioPagina * pagina, tamanioPagina);

    tablasDeNivel2[nroDeTabla2].entradaNivel2[entradaDeTabla2].indiceMarco = marco;
    tablasDeNivel2[nroDeTabla2].entradaNivel2[entradaDeTabla2].bitPresencia = 1;

    return marco;
}