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

void __cargar_tabla(uint32_t nroTablaNivel1, t_entrada_nivel_2_algoritmo* tablaAlgoritmo, t_memoria_data_holder memoriaData) {
    int* marcos = obtener_marcos(nroTablaNivel1, memoriaData);
    int tamanio = memoriaData.cantidadMarcosProceso;
    int puntero = obtener_puntero(nroTablaNivel1, memoriaData);
    int indicePuntero;

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

int seleccionar_victima_clock(uint32_t nroTablaNivel1, t_memoria_data_holder memoriaData) {
    int tamanio = memoriaData.cantidadMarcosProceso;
    t_entrada_nivel_2_algoritmo tablaAlgoritmo[tamanio];
    __cargar_tabla(nroTablaNivel1, tablaAlgoritmo, memoriaData);

    for (int j = 0; j < 2; j++) {
        for (int i = 0; i < tamanio; i++) {
            if (tablaAlgoritmo[i].bitUso) {
                tablaAlgoritmo[i].bitUso = false;
            } else {
                actualizar_puntero(nroTablaNivel1, tablaAlgoritmo[i].indicePagina, memoriaData);
                return tablaAlgoritmo[i].indicePagina;
            }
        }
    }

    __descargar_tabla(nroTablaNivel1, tablaAlgoritmo, memoriaData);  // TODO
}

int seleccionar_victima_clock_modificado(uint32_t nroTablaNivel1, t_memoria_data_holder memoriaData) {
    int tamanio = memoriaData.cantidadMarcosProceso;
    t_entrada_nivel_2_algoritmo tablaAlgoritmo[tamanio];
    __cargar_tabla(nroTablaNivel1, tablaAlgoritmo, memoriaData);

    for (int j = 0; j < 2; j++) {
        for (int i = 0; i < tamanio; i++) {
            if (!(tablaAlgoritmo[i].bitUso || tablaAlgoritmo[i].bitModificado)) {
                actualizar_puntero(nroTablaNivel1, tablaAlgoritmo[i].indicePagina, memoriaData);
                return tablaAlgoritmo[i].indicePagina;
            }
        }
        for (int i = 0; i < tamanio; i++) {
            if (!tablaAlgoritmo[i].bitUso && tablaAlgoritmo[i].bitModificado) {
                actualizar_puntero(nroTablaNivel1, tablaAlgoritmo[i].indicePagina, memoriaData);
                return tablaAlgoritmo[i].indicePagina;
            } else if (tablaAlgoritmo[i].bitUso) {
                tablaAlgoritmo[i].bitUso = false;
            }
        }
    }

    __descargar_tabla(nroTablaNivel1, tablaAlgoritmo, memoriaData);  // TODO
}

// void __cargar_tabla(uint32_t nroTablaNivel1, int puntero, t_entrada_nivel_2_algoritmo* tablaAlgoritmo, t_memoria_data_holder memoriaData) {
//     int indiceAlgoritmo = 0;
//     int indicePuntero = 0;
//     int tamanio = memoriaData.cantidadMarcosProceso;

//     for (int i = 0; i < memoriaData.entradasPorTabla; i++) {
//         int nroDeTabla2 = obtener_tabla_de_nivel_2(nroTablaNivel1, i, memoriaData);
//         for (int j = 0; j < memoriaData.entradasPorTabla; j++) {
//             if (pagina_en_memoria(nroDeTabla2, j, memoriaData)) {
//                 tablaAlgoritmo[indiceAlgoritmo].indicePagina = i * memoriaData.entradasPorTabla + j;
//                 tablaAlgoritmo[indiceAlgoritmo].bitUso = obtener_bit_uso(nroDeTabla2, j, memoriaData);
//                 tablaAlgoritmo[indiceAlgoritmo].bitModificado = obtener_bit_modificado(nroDeTabla2, j, memoriaData);

//                 if (tablaAlgoritmo[indiceAlgoritmo].indicePagina == puntero) {
//                     indicePuntero = indiceAlgoritmo;
//                 }

//                 indiceAlgoritmo++;
//                 if (indiceAlgoritmo == tamanio) {
//                     break;
//                 }
//             }
//         }
//     }

//     for (int i = 0; i < indicePuntero; i++) {
//         t_entrada_nivel_2_algoritmo temp = tablaAlgoritmo[0];
//         for (int j = 0; j < tamanio - 1; ++j) {
//             tablaAlgoritmo[j] = tablaAlgoritmo[j + 1];
//         }
//         tablaAlgoritmo[tamanio - 1] = temp;
//     }
// }