#include "tabla_nivel_2.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

int __obtener_entrada(int nroPagina, t_memoria_data_holder memoriaData);

typedef struct
{
    int indiceMarco;
    bool bitPresencia;
    bool bitUso;
    bool bitModificado;
} t_entrada_nivel_2;

struct t_tabla_nivel_2 {
    t_entrada_nivel_2* entradaNivel2;
};

t_tabla_nivel_2* crear_tablas_de_nivel_2(t_memoria_data_holder memoriaData) {
    int cantidadProcesosMax = memoriaData.cantidadProcesosMax;
    int entradasPorTabla = memoriaData.entradasPorTabla;

    t_tabla_nivel_2* tablasDeNivel2 = malloc(cantidadProcesosMax * entradasPorTabla * sizeof(t_entrada_nivel_2));  // TODO Revisar que este correcta la reserva
    for (int i = 0; i < cantidadProcesosMax * entradasPorTabla; i++) {
        for (int j = 0; j < entradasPorTabla; j++) {
            tablasDeNivel2[i].entradaNivel2[j].indiceMarco = -1;
            tablasDeNivel2[i].entradaNivel2[j].bitPresencia = false;
            tablasDeNivel2[i].entradaNivel2[j].bitUso = false;
            tablasDeNivel2[i].entradaNivel2[j].bitModificado = false;
        }
    }
    return tablasDeNivel2;
}

void actualizar_escritura_pagina(int nroPagina, int nroTablaNivel2, t_memoria_data_holder memoriaData) {
    t_tabla_nivel_2* tablasDeNivel2 = memoriaData.tablasDeNivel2;

    int entrada = __obtener_entrada(nroPagina, memoriaData);
    tablasDeNivel2[nroTablaNivel2].entradaNivel2[entrada].bitUso = true;
    tablasDeNivel2[nroTablaNivel2].entradaNivel2[entrada].bitModificado = true;
}

void actualizar_lectura_pagina(int nroPagina, int nroTablaNivel2, t_memoria_data_holder memoriaData) {
    t_tabla_nivel_2* tablasDeNivel2 = memoriaData.tablasDeNivel2;

    int entrada = __obtener_entrada(nroPagina, memoriaData);
    tablasDeNivel2[nroTablaNivel2].entradaNivel2[entrada].bitUso = true;
}

void swap_out(int nroDeTabla2, int entradaDeTabla2, t_memoria_data_holder memoriaData) {
    memoriaData.tablasDeNivel2[nroDeTabla2].entradaNivel2[entradaDeTabla2].bitPresencia = 0;
}

void swap_in(int nroDeTabla2, int entradaDeTabla2, int marco, t_memoria_data_holder memoriaData) {
    memoriaData.tablasDeNivel2[nroDeTabla2].entradaNivel2[entradaDeTabla2].indiceMarco = marco;
    memoriaData.tablasDeNivel2[nroDeTabla2].entradaNivel2[entradaDeTabla2].bitPresencia = 1;
}

void limpiar_tabla_nivel_2(int nroDeTabla2, t_memoria_data_holder memoriaData){
    int entradasPorTabla = memoriaData.entradasPorTabla;
    t_tabla_nivel_2* tablasDeNivel2 = memoriaData.tablasDeNivel2;

    for (int i = 0; i < entradasPorTabla; i++) {
        tablasDeNivel2[nroDeTabla2].entradaNivel2[i].indiceMarco = -1;
        tablasDeNivel2[nroDeTabla2].entradaNivel2[i].bitPresencia = false;
        tablasDeNivel2[nroDeTabla2].entradaNivel2[i].bitUso = false;
        tablasDeNivel2[nroDeTabla2].entradaNivel2[i].bitModificado = false;
    }
}

int obtener_marco(uint32_t nroDeTabla2, uint32_t entradaDeTabla2, t_memoria_data_holder memoriaData) {
    t_tabla_nivel_2* tablasDeNivel2 = memoriaData.tablasDeNivel2;
    
    return tablasDeNivel2[nroDeTabla2].entradaNivel2[entradaDeTabla2].indiceMarco;
}

bool pagina_en_memoria(uint32_t nroDeTabla2, uint32_t entradaDeTabla2, t_memoria_data_holder memoriaData){
    t_tabla_nivel_2* tablasDeNivel2 = memoriaData.tablasDeNivel2;
    
    return tablasDeNivel2[nroDeTabla2].entradaNivel2[entradaDeTabla2].bitPresencia;
}

int __obtener_entrada(int nroPagina,  t_memoria_data_holder memoriaData) {
    int entradasPorTabla = memoriaData.entradasPorTabla;
    
    return (nroPagina % (entradasPorTabla * entradasPorTabla)) % entradasPorTabla;
}