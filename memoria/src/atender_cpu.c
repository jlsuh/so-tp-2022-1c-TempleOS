#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "archivo.h"
#include "common_flags.h"
#include "marcos.h"
#include "memoria_data_holder.h"
#include "stream.h"
#include "tabla_nivel_1.h"
#include "tabla_nivel_2.h"
#include "tabla_suspendido.h"

extern t_memoria_data_holder memoriaData;

static void __actualizar_pagina(uint32_t direccionFisica, bool esEscritura, t_memoria_data_holder memoriaData) {
    int nroPagina = obtener_pagina_de_un_marco(direccionFisica, memoriaData);
    int nroTablaNivel2 = obtener_tabla_de_nivel_2_pagina(nroPagina, memoriaData);
    if (esEscritura)
        actualizar_escritura_pagina(nroPagina, nroTablaNivel2, memoriaData);
    else
        actualizar_lectura_pagina(nroPagina, nroTablaNivel2, memoriaData);
}

static int __swap_marco(uint32_t nroDeTabla2Victima, uint32_t entradaDeTabla2Victima, uint32_t nroDeTabla2, uint32_t entradaDeTabla2, t_memoria_data_holder memoriaData) {
    int marco = obtener_marco(nroDeTabla2Victima, entradaDeTabla2Victima, memoriaData);
    // actualizar_swap_out(nroDeTabla2Victima, entradaDeTabla2Victima, tablasDeNivel2); //TODO que es?
    swap_out(nroDeTabla2Victima, entradaDeTabla2Victima, memoriaData);

    int paginaVictima = nroDeTabla2Victima * memoriaData.entradasPorTabla + entradaDeTabla2Victima;
    int pagina = nroDeTabla2 * memoriaData.entradasPorTabla + entradaDeTabla2;
    int tiempoDeEspera = memoriaData.retardoSwap;

    // Escribir en archivo
    sleep(tiempoDeEspera);
    memcpy(memoriaData.inicio_archivo + memoriaData.tamanioPagina * paginaVictima, memoriaData.memoriaPrincipal + marco * memoriaData.tamanioPagina, memoriaData.tamanioPagina);
    // Leer de archivo
    sleep(tiempoDeEspera);
    memcpy(memoriaData.memoriaPrincipal + marco * memoriaData.tamanioPagina, memoriaData.inicio_archivo + memoriaData.tamanioPagina * pagina, memoriaData.tamanioPagina);

    swap_in(nroDeTabla2, entradaDeTabla2, marco, memoriaData);

    return marco;
}

int __obtener_marco(uint32_t nroDeTabla2, uint32_t entradaDeTabla2, t_memoria_data_holder memoriaData) {
    int marco = -1;
    if (pagina_en_memoria(nroDeTabla2, entradaDeTabla2, memoriaData)) {
        marco = obtener_marco(nroDeTabla2, entradaDeTabla2, memoriaData);
        return marco;
    }

    uint32_t nroTablaNivel1 = obtener_tabla_de_nivel_1(nroDeTabla2, memoriaData);
    int* marcos = obtener_marcos(nroTablaNivel1, memoriaData);
    marco = obtener_marco_libre(marcos, memoriaData);

    if (marco == -1) {  // TODO swap - Esta mal actualmente
        uint32_t nroDeTabla2Victima = 0, entradaDeTabla2Victima = 0;
        uint32_t tamanio = obtener_tamanio(nroTablaNivel1, memoriaData);
        nroDeTabla2Victima = memoriaData.seleccionar_victima(nroTablaNivel1, memoriaData);
        abrir_archivo(tamanio, nroTablaNivel1, memoriaData);
        marco = __swap_marco(nroDeTabla2Victima, entradaDeTabla2Victima, nroDeTabla2, entradaDeTabla2, memoriaData);
        marco = obtener_marco(nroDeTabla2, entradaDeTabla2, memoriaData);
        cerrar_archivo(memoriaData);
    } else{
        int pagina = obtener_pagina(nroDeTabla2, entradaDeTabla2, memoriaData);
        asignar_pagina_a_marco(pagina, marco, memoriaData);
        asignar_marco_a_pagina(marco, nroDeTabla2, entradaDeTabla2, memoriaData); //TODO poner bit de uso en 1?
    }

    return marco;
}

void* escuchar_peticiones_cpu(void* socketCpu) {
    void* memoriaPrincipal = memoriaData.memoriaPrincipal;
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

                __actualizar_pagina(direccionFisica, false, memoriaData);

                break;
            case HEADER_write:
                buffer_unpack(buffer, &direccionFisica, sizeof(direccionFisica));
                buffer_unpack(buffer, &valor, sizeof(valor));

                memcpy(memoriaPrincipal + direccionFisica, &valor, sizeof(valor));

                __actualizar_pagina(direccionFisica, true, memoriaData);
                break;
            case HEADER_copy:
                buffer_unpack(buffer, &direccionFisica, sizeof(direccionFisica));
                buffer_unpack(buffer, &direccionFisicaOrigen, sizeof(direccionFisicaOrigen));

                memcpy(memoriaPrincipal + direccionFisica, memoriaPrincipal + direccionFisicaOrigen, sizeof(uint32_t));

                __actualizar_pagina(direccionFisica, true, memoriaData);
                __actualizar_pagina(direccionFisicaOrigen, false, memoriaData);
                break;
            case HEADER_tabla_nivel_2: {
                uint32_t nroDeTabla1, entradaDeTabla1;
                buffer_unpack(buffer, &nroDeTabla1, sizeof(nroDeTabla1));
                buffer_unpack(buffer, &entradaDeTabla1, sizeof(entradaDeTabla1));

                int nroDeTabla2 = obtener_tabla_de_nivel_2(nroDeTabla1, entradaDeTabla1, memoriaData);
                if (nroDeTabla2 == -1) {
                    despertar_proceso(nroDeTabla1, memoriaData);
                    nroDeTabla2 = obtener_tabla_de_nivel_2(nroDeTabla1, entradaDeTabla1, memoriaData);
                }

                uint32_t nroDeTabla2send = nroDeTabla2;

                t_buffer* buffer_rta = buffer_create();
                buffer_pack(buffer_rta, &nroDeTabla2send, sizeof(nroDeTabla2send));
                stream_send_buffer(socket, HEADER_tabla_nivel_2, buffer_rta);
                buffer_destroy(buffer_rta);

                break;
            }
            case HEADER_marco: {
                uint32_t nroDeTabla2, entradaDeTabla2;
                buffer_unpack(buffer, &nroDeTabla2, sizeof(nroDeTabla2));
                buffer_unpack(buffer, &entradaDeTabla2, sizeof(entradaDeTabla2));

                int indiceMarco = __obtener_marco(nroDeTabla2, entradaDeTabla2, memoriaData);
                uint32_t marco = indiceMarco * memoriaData.tamanioPagina;

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