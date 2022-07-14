#include <pthread.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "archivo.h"
#include "common_flags.h"
#include "common_utils.h"
#include "marcos.h"
#include "memoria_data_holder.h"
#include "stream.h"
#include "tabla_nivel_1.h"
#include "tabla_nivel_2.h"
#include "tabla_suspendido.h"

extern t_memoria_data_holder* memoriaData;
extern pthread_mutex_t mutexMemoriaData;

static void __loggear_paginas_en_memoria_del_proceso(uint32_t nroTablaNivel2, t_memoria_data_holder* memoriaData) {
    int nroTablaNivel1 = obtener_tabla_de_nivel_1(nroTablaNivel2, memoriaData);
    int* marcos = obtener_marcos(nroTablaNivel1, memoriaData);
    int tamanio = memoriaData->cantidadMarcosProceso;
    int puntero = obtener_puntero(nroTablaNivel1, memoriaData);
    printf("--------------------\n");
    printf("Proceso [%d] con puntero [%d]\n", nroTablaNivel1, puntero);
    printf("Marco\t\tPagina\t\tBitUso\t\tBitModificado\n");
    for (int i = 0; i < tamanio; i++) {
        int pagina = obtener_pagina_de_un_marco(marcos[i], memoriaData);
        if (pagina != -1) {
            int paginaLocal = obtener_indice(pagina, memoriaData);
            bool bitUso = obtener_bit_uso(pagina, memoriaData);
            bool bitModificado = obtener_bit_modificado(pagina, memoriaData);
            printf("%d\t\t%d\t\t%d\t\t%d\n", marcos[i], paginaLocal, bitUso, bitModificado);
        } else {
            printf("%d\t\t%d\t\t%d\t\t%d\n", marcos[i], -1, 0, 0);
        }
    }
    printf("--------------------\n");
}

static void __actualizar_pagina(uint32_t direccionFisica, bool esEscritura, t_memoria_data_holder* memoriaData) {
    int nroPagina = obtener_pagina_de_direccion_fisica(direccionFisica, memoriaData);
    int nroTablaNivel2 = obtener_tabla_de_nivel_2_pagina(nroPagina, memoriaData);
    if (esEscritura)
        actualizar_escritura_pagina(nroPagina, nroTablaNivel2, memoriaData);
    else
        actualizar_lectura_pagina(nroPagina, nroTablaNivel2, memoriaData);

    // No es logica de dominio
    __loggear_paginas_en_memoria_del_proceso(nroTablaNivel2, memoriaData);
}

static int __swap_marco(uint32_t nroDeTabla2Victima, uint32_t entradaDeTabla2Victima, uint32_t nroDeTabla2, uint32_t entradaDeTabla2, t_memoria_data_holder* memoriaData) {
    int marco = obtener_marco(nroDeTabla2Victima, entradaDeTabla2Victima, memoriaData);
    swap_out(nroDeTabla2Victima, entradaDeTabla2Victima, marco, memoriaData);
    swap_in(nroDeTabla2, entradaDeTabla2, marco, memoriaData);
    return marco;
}

int __obtener_marco(uint32_t nroDeTabla2, uint32_t entradaDeTabla2, t_memoria_data_holder* memoriaData) {
    int marco = -1;
    if (pagina_en_memoria(nroDeTabla2, entradaDeTabla2, memoriaData)) {
        marco = obtener_marco(nroDeTabla2, entradaDeTabla2, memoriaData);
        return marco;
    }

    uint32_t nroTablaNivel1 = obtener_tabla_de_nivel_1(nroDeTabla2, memoriaData);
    int* marcos = obtener_marcos(nroTablaNivel1, memoriaData);
    marco = obtener_marco_libre(marcos, memoriaData);

    uint32_t tamanio = obtener_tamanio(nroTablaNivel1, memoriaData);
    abrir_archivo(tamanio, nroTablaNivel1, memoriaData);
    if (marco == -1) {
        int punteroVictima = memoriaData->seleccionar_victima(nroTablaNivel1, memoriaData);
        uint32_t indiceTabla2 = punteroVictima / memoriaData->entradasPorTabla;
        uint32_t nroDeTabla2Victima = obtener_tabla_de_nivel_2(nroTablaNivel1, indiceTabla2, memoriaData);
        uint32_t entradaDeTabla2Victima = punteroVictima % memoriaData->entradasPorTabla;
        marco = __swap_marco(nroDeTabla2Victima, entradaDeTabla2Victima, nroDeTabla2, entradaDeTabla2, memoriaData);
    } else {
        int pagina = obtener_pagina(nroDeTabla2, entradaDeTabla2, memoriaData);
        if (marco == marcos[0]) {
            int puntero = obtener_indice(pagina, memoriaData);
            actualizar_puntero(nroTablaNivel1, puntero, memoriaData);
        }
        asignar_pagina_a_marco(pagina, marco, memoriaData);
        swap_in(nroDeTabla2, entradaDeTabla2, marco, memoriaData);
    }
    cerrar_archivo(tamanio, memoriaData);

    return marco;
}

void* escuchar_peticiones_cpu(void* socketCpu) {
    void* memoriaPrincipal = memoriaData->memoriaPrincipal;
    int socket = *(int*)socketCpu;
    // free(socketCpu);

    uint32_t header, direccionFisica, valor;
    t_buffer* buffer;

    for (;;) {
        header = stream_recv_header(socket);
        pthread_mutex_lock(&mutexMemoriaData);
        buffer = buffer_create();
        stream_recv_buffer(socket, buffer);
        intervalo_de_pausa(memoriaData->retardoMemoria);
        switch (header) {
            case HEADER_read:
                log_info(memoriaData->memoriaLogger, "\e[1;93mPetición de lectura\e[0m");
                buffer_unpack(buffer, &direccionFisica, sizeof(direccionFisica));

                memcpy((void*)&valor, (void*)(memoriaPrincipal + direccionFisica), sizeof(valor));

                t_buffer* buffer_rta = buffer_create();
                buffer_pack(buffer_rta, &valor, sizeof(valor));
                stream_send_buffer(socket, HEADER_read, buffer_rta);
                buffer_destroy(buffer_rta);

                __actualizar_pagina(direccionFisica, false, memoriaData);

                log_info(memoriaData->memoriaLogger, "Se envió el valor [%d] de la dirección física [%d]", valor, direccionFisica);
                break;
            case HEADER_write:
                log_info(memoriaData->memoriaLogger, "\e[1;93mPetición de escritura\e[0m");
                buffer_unpack(buffer, &direccionFisica, sizeof(direccionFisica));
                buffer_unpack(buffer, &valor, sizeof(valor));

                memcpy((void*)(memoriaPrincipal + direccionFisica), (void*)&valor, sizeof(valor));

                __actualizar_pagina(direccionFisica, true, memoriaData);

                log_info(memoriaData->memoriaLogger, "Se escribio el valor [%d] en la dirección física [%d]", *((uint32_t*)(memoriaPrincipal + direccionFisica)), direccionFisica);
                break;
            case HEADER_tabla_nivel_2: {
                log_info(memoriaData->memoriaLogger, "\e[1;93mPetición de tabla nivel 2\e[0m");
                uint32_t nroDeTabla1, entradaDeTabla1;
                buffer_unpack(buffer, &nroDeTabla1, sizeof(nroDeTabla1));
                buffer_unpack(buffer, &entradaDeTabla1, sizeof(entradaDeTabla1));
                log_info(memoriaData->memoriaLogger, "Se recibe ID [%d] de tabla 1 con la entrada [%d]", nroDeTabla1, entradaDeTabla1);

                int nroDeTabla2 = obtener_tabla_de_nivel_2(nroDeTabla1, entradaDeTabla1, memoriaData);
                if (nroDeTabla2 == -1) {
                    log_info(memoriaData->memoriaLogger, "\e[1;92mProceso suspendido, se procede a despertar\e[0m");
                    despertar_proceso(nroDeTabla1, memoriaData);
                    nroDeTabla2 = obtener_tabla_de_nivel_2(nroDeTabla1, entradaDeTabla1, memoriaData);
                }

                uint32_t nroDeTabla2send = nroDeTabla2;

                t_buffer* buffer_rta = buffer_create();
                buffer_pack(buffer_rta, &nroDeTabla2send, sizeof(nroDeTabla2send));
                stream_send_buffer(socket, HEADER_tabla_nivel_2, buffer_rta);
                buffer_destroy(buffer_rta);

                log_info(memoriaData->memoriaLogger, "Se enviá la tabla nivel 2 con ID [%d]", nroDeTabla2send);
                break;
            }
            case HEADER_marco: {
                log_info(memoriaData->memoriaLogger, "\e[1;93mPetición de marco\e[0m");
                uint32_t nroDeTabla2, entradaDeTabla2;
                buffer_unpack(buffer, &nroDeTabla2, sizeof(nroDeTabla2));
                buffer_unpack(buffer, &entradaDeTabla2, sizeof(entradaDeTabla2));

                log_info(memoriaData->memoriaLogger, "Se quiere la dirección física de la tabla 2 [%d] con entrada [%d]", nroDeTabla2, entradaDeTabla2);

                int indiceMarco = __obtener_marco(nroDeTabla2, entradaDeTabla2, memoriaData);
                uint32_t marco = indiceMarco * memoriaData->tamanioPagina;

                t_buffer* buffer_rta = buffer_create();
                buffer_pack(buffer_rta, &marco, sizeof(marco));
                stream_send_buffer(socket, HEADER_marco, buffer_rta);
                buffer_destroy(buffer_rta);

                log_info(memoriaData->memoriaLogger, "Se enviá la dirección física [%d]", marco);
                break;
            }
            default:
                break;
        }
        buffer_destroy(buffer);
        pthread_mutex_unlock(&mutexMemoriaData);
    }

    return NULL;
}