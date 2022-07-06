#include "tabla_nivel_1.h"

#include <stdint.h>
#include <stdlib.h>

#include "marcos.h"
struct t_tabla_nivel_1 {
    uint32_t id;
    uint32_t tamanio;
    int* nroTablaNivel2;
    int* marcos;
    int puntero;
};

t_tabla_nivel_1* crear_tablas_de_nivel_1(t_memoria_data_holder* memoriaData) {
    int cantidadProcesosMax = memoriaData->cantidadProcesosMax;
    int cantidadMarcosProceso = memoriaData->cantidadMarcosProceso;
    int entradasPorTabla = memoriaData->entradasPorTabla;

    t_tabla_nivel_1* tablasDeNivel1 = malloc(cantidadProcesosMax * sizeof(t_tabla_nivel_1));
    int contadorTablaNivel2 = 0;
    int contadorMarcos = 0;

    log_info(memoriaData->memoriaLogger, "\e[1;93mSe crean las tablas de nivel 1\e[0m");
    for (int i = 0; i < cantidadProcesosMax; i++) {
        tablasDeNivel1[i].nroTablaNivel2 = malloc(entradasPorTabla * sizeof(int));
        tablasDeNivel1[i].marcos = malloc(cantidadMarcosProceso * sizeof(int));
        tablasDeNivel1[i].tamanio = 0;
        tablasDeNivel1[i].id = 0;
        tablasDeNivel1[i].puntero = 0;
        for (int j = 0; j < entradasPorTabla; j++) {
            tablasDeNivel1[i].nroTablaNivel2[j] = contadorTablaNivel2;
            contadorTablaNivel2++;
        }
        for (int j = 0; j < cantidadMarcosProceso; j++) {
            tablasDeNivel1[i].marcos[j] = contadorMarcos;
            contadorMarcos++;
        }
    }
    log_info(memoriaData->memoriaLogger, "Se creó %d tablas con %d marcos y %d entradas de paginas", cantidadProcesosMax, cantidadMarcosProceso, entradasPorTabla);

    return tablasDeNivel1;
}

int obtener_tabla_de_nivel_2_pagina(int nroPagina, t_memoria_data_holder* memoriaData) {
    t_tabla_nivel_1* tablasDeNivel1 = memoriaData->tablasDeNivel1;
    int entradasPorTabla = memoriaData->entradasPorTabla;

    int indiceNivel1 = nroPagina / (entradasPorTabla * entradasPorTabla);
    int indiceNivel2 = (nroPagina % (entradasPorTabla * entradasPorTabla)) / entradasPorTabla;
    return tablasDeNivel1[indiceNivel1].nroTablaNivel2[indiceNivel2];
}

int obtener_tabla_de_nivel_2(uint32_t nroDeTabla1, uint32_t entradaDeTabla1, t_memoria_data_holder* memoriaData) {
    int cantidadProcesosMax = memoriaData->cantidadProcesosMax;
    t_tabla_nivel_1* tablasDeNivel1 = memoriaData->tablasDeNivel1;

    for (int i = 0; i < cantidadProcesosMax; i++) {
        if (tablasDeNivel1[i].id == nroDeTabla1) {
            return tablasDeNivel1[i].nroTablaNivel2[entradaDeTabla1];
        }
    }
    return -1;
}

uint32_t obtener_tabla_de_nivel_1(uint32_t nroDeTabla2, t_memoria_data_holder* memoriaData) {
    int cantidadProcesosMax = memoriaData->cantidadProcesosMax;
    int entradasPorTabla = memoriaData->entradasPorTabla;
    t_tabla_nivel_1* tablasDeNivel1 = memoriaData->tablasDeNivel1;

    for (int i = 0; i < cantidadProcesosMax; i++) {
        for (int j = 0; j < entradasPorTabla; j++) {
            if (tablasDeNivel1[i].nroTablaNivel2[j] == nroDeTabla2) {
                return tablasDeNivel1[i].id;
            }
        }
    }
    return -1;
}

uint32_t obtener_tabla_libre_de_nivel_1(t_memoria_data_holder* memoriaData) {
    int cantidadProcesosMax = memoriaData->cantidadProcesosMax;
    t_tabla_nivel_1* tablasDeNivel1 = memoriaData->tablasDeNivel1;

    int i;
    for (i = 0; i < cantidadProcesosMax; i++) {
        if (tablasDeNivel1[i].id == 0) {
            break;
        }
    }
    return i;
}

uint32_t asignar_tabla_nivel_1(int indiceTablaNivel1, uint32_t tamanio, t_memoria_data_holder* memoriaData) {
    t_tabla_nivel_1* tablasDeNivel1 = memoriaData->tablasDeNivel1;

    tablasDeNivel1[indiceTablaNivel1].tamanio = tamanio;
    tablasDeNivel1[indiceTablaNivel1].id = memoriaData->contadorTabla1++;  // TODO mutex?
    return tablasDeNivel1[indiceTablaNivel1].id;
}

void asignar_tabla_nivel_1_with_id(int indiceTablaNivel1, uint32_t nroTablaNivel1, uint32_t tamanio, t_memoria_data_holder* memoriaData) {
    t_tabla_nivel_1* tablasDeNivel1 = memoriaData->tablasDeNivel1;

    tablasDeNivel1[indiceTablaNivel1].tamanio = tamanio;
    tablasDeNivel1[indiceTablaNivel1].id = nroTablaNivel1;
}

int* obtener_marcos(uint32_t nroTablaNivel1, t_memoria_data_holder* memoriaData) {
    t_tabla_nivel_1* tablasDeNivel1 = memoriaData->tablasDeNivel1;

    int indiceTablaNivel1 = obtener_indice_tabla_nivel_1(nroTablaNivel1, memoriaData);
    return tablasDeNivel1[indiceTablaNivel1].marcos;
}

uint32_t obtener_indice_tabla_nivel_1(uint32_t nroTablaNivel1, t_memoria_data_holder* memoriaData) {
    int cantidadProcesosMax = memoriaData->cantidadProcesosMax;
    t_tabla_nivel_1* tablasDeNivel1 = memoriaData->tablasDeNivel1;
    uint32_t indice;

    for (int i = 0; i < cantidadProcesosMax; i++) {
        if (tablasDeNivel1[i].id == nroTablaNivel1) {
            indice = i;
        }
    }

    return indice;
}

uint32_t obtener_tamanio(uint32_t nroTablaNivel1, t_memoria_data_holder* memoriaData) {
    t_tabla_nivel_1* tablasDeNivel1 = memoriaData->tablasDeNivel1;

    int indiceTablaNivel1 = obtener_indice_tabla_nivel_1(nroTablaNivel1, memoriaData);
    return tablasDeNivel1[indiceTablaNivel1].tamanio;
}

bool hay_tabla_nivel_1_disponible(t_memoria_data_holder* memoriaData) {
    int cantidadProcesosMax = memoriaData->cantidadProcesosMax;
    t_tabla_nivel_1* tablasDeNivel1 = memoriaData->tablasDeNivel1;

    int i;
    for (i = 0; i < cantidadProcesosMax; i++) {
        if (tablasDeNivel1[i].id == 0) {
            return true;
        }
    }
    return false;
}

void limpiar_tabla_nivel_1(uint32_t nroTablaNivel1, t_memoria_data_holder* memoriaData) {
    int cantidadProcesosMax = memoriaData->cantidadProcesosMax;
    int cantidadMarcosPorProceso = memoriaData->cantidadMarcosProceso;
    t_tabla_nivel_1* tablasDeNivel1 = memoriaData->tablasDeNivel1;

    for (int i = 0; i < cantidadProcesosMax; i++) {
        if (tablasDeNivel1[i].id == nroTablaNivel1) {
            tablasDeNivel1[i].id = 0;
            tablasDeNivel1[i].tamanio = 0;
            tablasDeNivel1[i].puntero = 0;
            int* marcos = tablasDeNivel1[i].marcos;
            for (int r = 0; r < cantidadMarcosPorProceso; r++) {
                limpiar_marco(marcos[r], memoriaData);
            }
            break;
        }
    }
}

int obtener_puntero(uint32_t nroTablaNivel1, t_memoria_data_holder* memoriaData) {
    int puntero = -1;
    for (int i = 0; i < memoriaData->cantidadProcesosMax; i++) {
        if (memoriaData->tablasDeNivel1[i].id == nroTablaNivel1) {
            puntero = memoriaData->tablasDeNivel1[i].puntero;
        }
    }
    return puntero;
}

void actualizar_puntero(uint32_t nroTablaNivel1, int puntero, t_memoria_data_holder* memoriaData) {
    for (int i = 0; i < memoriaData->cantidadProcesosMax; i++) {
        if (memoriaData->tablasDeNivel1[i].id == nroTablaNivel1) {
            memoriaData->tablasDeNivel1[i].puntero = puntero;
        }
    }
}