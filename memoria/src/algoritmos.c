#include "algoritmos.h"

#include "marcos.h"
#include "tabla_nivel_1.h"
#include "tabla_nivel_2.h"

typedef struct
{
    int indicePagina;
    bool bitUso;
    bool bitModificado;
} t_entrada_nivel_2_algoritmo;

void __cargar_tabla(uint32_t nroTablaNivel1, t_entrada_nivel_2_algoritmo* tablaAlgoritmo, t_memoria_data_holder* memoriaData) {
    int* marcos = obtener_marcos(nroTablaNivel1, memoriaData);
    int tamanio = memoriaData->cantidadMarcosProceso;
    int puntero = obtener_puntero(nroTablaNivel1, memoriaData);
    printf("puntero: %d\n", puntero);
    int indicePuntero = -1;

    for (int i = 0; i < tamanio; i++) {
        int pagina = obtener_pagina_de_un_marco(marcos[i], memoriaData);
        tablaAlgoritmo[i].indicePagina = obtener_indice(pagina, memoriaData);
        tablaAlgoritmo[i].bitUso = obtener_bit_uso(pagina, memoriaData);
        tablaAlgoritmo[i].bitModificado = obtener_bit_modificado(pagina, memoriaData);
        if (tablaAlgoritmo[i].indicePagina == puntero) {
            indicePuntero = i;
        }
    }

    for (int i = 0; i < indicePuntero; i++) {
        t_entrada_nivel_2_algoritmo temp = tablaAlgoritmo[0];
        for (int j = 0; j < tamanio - 1; ++j) {
            tablaAlgoritmo[j] = tablaAlgoritmo[j + 1];
        }
        tablaAlgoritmo[tamanio - 1] = temp;
    }
}

void __descargar_tabla(uint32_t nroTablaNivel1, t_entrada_nivel_2_algoritmo* tablaAlgoritmo, t_memoria_data_holder* memoriaData) {
    int indiceTabla1 = (int)obtener_indice_tabla_nivel_1(nroTablaNivel1, memoriaData);
    int paginaInicialTabla1 = indiceTabla1 * memoriaData->entradasPorTabla * memoriaData->entradasPorTabla;
    for (int i = 0; i < memoriaData->cantidadMarcosProceso; i++) {
        int pagina = paginaInicialTabla1 + tablaAlgoritmo[i].indicePagina;
        setear_bit_uso(pagina, tablaAlgoritmo[i].bitUso, memoriaData);
    }
}

int seleccionar_victima_clock(uint32_t nroTablaNivel1, t_memoria_data_holder* memoriaData) {
    int tamanio = memoriaData->cantidadMarcosProceso;
    t_entrada_nivel_2_algoritmo tablaAlgoritmo[tamanio];
    __cargar_tabla(nroTablaNivel1, tablaAlgoritmo, memoriaData);

    for (int j = 0; j < 2; j++) {
        for (int i = 0; i < tamanio; i++) {
            if (tablaAlgoritmo[i].bitUso) {
                tablaAlgoritmo[i].bitUso = false;
            } else {
                if (i == tamanio - 1) {
                    actualizar_puntero(nroTablaNivel1, tablaAlgoritmo[0].indicePagina, memoriaData);
                } else {
                    actualizar_puntero(nroTablaNivel1, tablaAlgoritmo[i+1].indicePagina, memoriaData);
                }
                __descargar_tabla(nroTablaNivel1, tablaAlgoritmo, memoriaData);
                return tablaAlgoritmo[i].indicePagina;
            }
        }
    }
    return -1;
}

int seleccionar_victima_clock_modificado(uint32_t nroTablaNivel1, t_memoria_data_holder* memoriaData) {
    int tamanio = memoriaData->cantidadMarcosProceso;
    t_entrada_nivel_2_algoritmo tablaAlgoritmo[tamanio];
    __cargar_tabla(nroTablaNivel1, tablaAlgoritmo, memoriaData);

    for (int j = 0; j < 2; j++) {
        for (int i = 0; i < tamanio; i++) {
            if (!tablaAlgoritmo[i].bitUso && !tablaAlgoritmo[i].bitModificado) {
                if (i == tamanio - 1) {
                    actualizar_puntero(nroTablaNivel1, tablaAlgoritmo[0].indicePagina, memoriaData);
                } else {
                    actualizar_puntero(nroTablaNivel1, tablaAlgoritmo[i+1].indicePagina, memoriaData);
                }
                __descargar_tabla(nroTablaNivel1, tablaAlgoritmo, memoriaData);
                return tablaAlgoritmo[i].indicePagina;
            }
        }
        for (int i = 0; i < tamanio; i++) {
            if (!tablaAlgoritmo[i].bitUso && tablaAlgoritmo[i].bitModificado) {
                if (i == tamanio - 1) {
                    actualizar_puntero(nroTablaNivel1, tablaAlgoritmo[0].indicePagina, memoriaData);
                } else {
                    actualizar_puntero(nroTablaNivel1, tablaAlgoritmo[i+1].indicePagina, memoriaData);
                }
                __descargar_tabla(nroTablaNivel1, tablaAlgoritmo, memoriaData);
                return tablaAlgoritmo[i].indicePagina;
            } else if (tablaAlgoritmo[i].bitUso) {
                tablaAlgoritmo[i].bitUso = false;
            }
        }
    }
    return -1;
}