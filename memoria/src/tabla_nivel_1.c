#include "tabla_nivel_1.h"
#include <stdint.h>

struct t_tabla_nivel_1
{
    uint32_t tamanio;
    int* nroTablaNivel2;
    int* marcos;
};

t_tabla_nivel_1* crear_tablas_de_nivel_1(int cantidadProcesosMax, int entradasPorTabla) {
    t_tabla_nivel_1* tablasDeNivel1 = malloc(cantidadProcesosMax * sizeof(t_tabla_nivel_1));
    int contadorTablaNivel2 = 0;
    for (int i = 0; i < cantidadProcesosMax; i++) {
        tablasDeNivel1[i].nroTablaNivel2 = malloc(entradasPorTabla * sizeof(int));
        tablasDeNivel1[i].tamanio = 0;
        for (int j = 0; j < entradasPorTabla; j++) {
            tablasDeNivel1[i].nroTablaNivel2[j] = contadorTablaNivel2;
            contadorTablaNivel2++;
        }
    }
    return tablasDeNivel1;
}

int obtener_tabla_de_nivel_2_pagina(int nroPagina, int entradasPorTabla, t_tabla_nivel_1* tablasDeNivel1) {
    int indiceNivel1 = nroPagina / (entradasPorTabla * entradasPorTabla);
    int indiceNivel2 = (nroPagina % (entradasPorTabla * entradasPorTabla)) / entradasPorTabla;
    return tablasDeNivel1[indiceNivel1].nroTablaNivel2[indiceNivel2];
}

uint32_t obtener_tabla_de_nivel_2(uint32_t nroDeTabla1, uint32_t entradaDeTabla1, t_tabla_nivel_1* tablasDeNivel1) {
    return tablasDeNivel1[nroDeTabla1].nroTablaNivel2[entradaDeTabla1];
}

int obtener_tabla_de_nivel_1(uint32_t nroDeTabla2, int cantidadProcesosMax, int entradasPorTabla, t_tabla_nivel_1* tablasDeNivel1) {
    for (int i = 0; i < cantidadProcesosMax; i++) {
        for (int j = 0; j < entradasPorTabla; j++) {
            if (tablasDeNivel1[i].nroTablaNivel2[j] == nroDeTabla2) {
                return i;
            }
        }
    }
    return -1;
}

int obtener_tabla_libre_de_nivel_1(int cantidadProcesosMax, t_tabla_nivel_1* tablasDeNivel1) {
    int i;
    for (i = 0; i < cantidadProcesosMax; i++) {
        if (tablasDeNivel1[i].tamanio == 0) {
            break;
        }
    }
    return i;
}

void asignar_tamanio_tabla_nivel_1(int nroTablaNivel1, uint32_t tamanio, t_tabla_nivel_1* tablasDeNivel1) {
    tablasDeNivel1[nroTablaNivel1].tamanio = tamanio;
}