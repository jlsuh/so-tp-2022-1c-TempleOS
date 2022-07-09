#include "tabla_suspendido.h"

#include <commons/collections/dictionary.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "archivo.h"
#include "tabla_nivel_1.h"
#include "tabla_nivel_2.h"

t_dictionary* crear_tabla_de_suspendidos(void) {
    return dictionary_create();
}

void __aniadir_a_tabla(uint32_t nroDeTabla1, uint32_t* punteroValue, t_memoria_data_holder* memoriaData) {
    int length = snprintf(NULL, 0, "%d", nroDeTabla1);
    char* nroDeTabla1Str = malloc(length + 1);
    snprintf(nroDeTabla1Str, length + 1, "%d", nroDeTabla1);
    dictionary_put(memoriaData->tablaSuspendidos, nroDeTabla1Str, (void*)punteroValue);
    free(nroDeTabla1Str);
}

void suspender_proceso(uint32_t nroDeTabla1, t_memoria_data_holder* memoriaData) {
    uint32_t tamanioNroDeTabla1 = obtener_tamanio(nroDeTabla1, memoriaData);
    int cantPaginas = memoriaData->entradasPorTabla * memoriaData->entradasPorTabla;
    uint32_t value[cantPaginas];
    int indiceValue = 0;

    abrir_archivo(tamanioNroDeTabla1, nroDeTabla1, memoriaData);
    int entradasPorTabla = memoriaData->entradasPorTabla;
    for (int i = 0; i < entradasPorTabla; i++) {
        int nroDeTabla2 = obtener_tabla_de_nivel_2(nroDeTabla1, i, memoriaData);
        for (int j = 0; j < entradasPorTabla; j++) {
            if (pagina_en_memoria(nroDeTabla2, j, memoriaData)) {
                int marco = obtener_marco(nroDeTabla2, j, memoriaData);
                swap_out(nroDeTabla2, j, marco, memoriaData);
                log_trace(memoriaData->memoriaLogger, "La pagina %d esta en memoria", nroDeTabla2 * entradasPorTabla + j);
                memset(memoriaData->memoriaPrincipal + marco * memoriaData->tamanioPagina, 0, memoriaData->tamanioPagina);
            }
            if (obtener_bit_pagina_en_swap(nroDeTabla2, j, memoriaData)) {
                value[indiceValue++] = i * entradasPorTabla + j;
            }
        }
        limpiar_tabla_nivel_2(nroDeTabla2, memoriaData);
    }
    limpiar_tabla_nivel_1(nroDeTabla1, memoriaData);
    cerrar_archivo(tamanioNroDeTabla1, memoriaData);

    uint32_t* punteroValue = malloc((indiceValue + 2) * sizeof(*punteroValue));
    punteroValue[0] = tamanioNroDeTabla1;
    punteroValue[1] = indiceValue;
    for (int i = 0; i < indiceValue; i++) {
        punteroValue[i + 2] = value[i];
    }
    __aniadir_a_tabla(nroDeTabla1, punteroValue, memoriaData);
}

void despertar_proceso(uint32_t nroDeTabla1, t_memoria_data_holder* memoriaData) {
    int length = snprintf(NULL, 0, "%d", nroDeTabla1);
    char* nroDeTabla1Str = malloc(length + 1);
    snprintf(nroDeTabla1Str, length + 1, "%d", nroDeTabla1);
    uint32_t* valueRecuperado = (uint32_t*)dictionary_remove(memoriaData->tablaSuspendidos, nroDeTabla1Str);
    uint32_t tamanioNroDeTabla1 = valueRecuperado[0];
    free(nroDeTabla1Str);

    uint32_t indiceTablaNivel1 = obtener_tabla_libre_de_nivel_1(memoriaData);
    asignar_tabla_nivel_1_with_id(indiceTablaNivel1, nroDeTabla1, tamanioNroDeTabla1, memoriaData);

    uint32_t indiceValue = valueRecuperado[1];
    for (int i = 0; i < indiceValue; i++) {
        int puntero = valueRecuperado[i + 2];
        int entradaDeTabla2 = puntero / memoriaData->entradasPorTabla;
        int nroTabla2 = obtener_tabla_de_nivel_2(nroDeTabla1, entradaDeTabla2, memoriaData);
        int entradaTabla2 = puntero % memoriaData->entradasPorTabla;
        setear_bit_pagina_en_swap(nroTabla2, entradaTabla2, true, memoriaData);
    }
    free(valueRecuperado);
}

bool esta_suspendido(uint32_t nroTablaNivel1, t_memoria_data_holder* memoriaData) {
    int length = snprintf(NULL, 0, "%d", nroTablaNivel1);
    char* nroDeTabla1Str = malloc(length + 1);
    snprintf(nroDeTabla1Str, length + 1, "%d", nroTablaNivel1);
    bool estaSuspendido = dictionary_has_key(memoriaData->tablaSuspendidos, nroDeTabla1Str);
    free(nroDeTabla1Str);

    return estaSuspendido;
}

void eliminar_de_tabla_suspendidos(uint32_t nroTablaNivel1, t_memoria_data_holder* memoriaData) {
    int length = snprintf(NULL, 0, "%d", nroTablaNivel1);
    char* nroDeTabla1Str = malloc(length + 1);
    snprintf(nroDeTabla1Str, length + 1, "%d", nroTablaNivel1);
    uint32_t* valueRecuperado = dictionary_remove(memoriaData->tablaSuspendidos, nroDeTabla1Str);
    free(valueRecuperado);
    free(nroDeTabla1Str);
}
