#include "tabla_nivel_2.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
typedef struct
{
    int indiceMarco;
    bool bitPresencia;
    bool bitUso;
    bool bitModificado;
    bool bitPaginaEnSwap;
} t_entrada_nivel_2;

struct t_tabla_nivel_2 {
    t_entrada_nivel_2* entradaNivel2;
};

int __obtener_tabla2(int nroPagina, t_memoria_data_holder memoriaData) {
    return nroPagina / (memoriaData.entradasPorTabla * memoriaData.entradasPorTabla);
}

int __obtener_entrada(int nroPagina, t_memoria_data_holder memoriaData) {
    int entradasPorTabla = memoriaData.entradasPorTabla;

    return (nroPagina % (entradasPorTabla * entradasPorTabla)) % entradasPorTabla;
}

t_tabla_nivel_2* crear_tablas_de_nivel_2(t_memoria_data_holder memoriaData) {
    int cantidadProcesosMax = memoriaData.cantidadProcesosMax;
    int entradasPorTabla = memoriaData.entradasPorTabla;
    int cantidadTablasNivel2Max = cantidadProcesosMax * entradasPorTabla;

    t_tabla_nivel_2* tablasDeNivel2 = malloc(cantidadTablasNivel2Max * sizeof(*tablasDeNivel2));

    for (int i = 0; i < cantidadTablasNivel2Max; i++) {
        tablasDeNivel2[i].entradaNivel2 = malloc(entradasPorTabla * sizeof(t_entrada_nivel_2));
        for (int j = 0; j < entradasPorTabla; j++) {
            tablasDeNivel2[i].entradaNivel2[j].indiceMarco = -1;
            tablasDeNivel2[i].entradaNivel2[j].bitPresencia = false;
            tablasDeNivel2[i].entradaNivel2[j].bitUso = false;
            tablasDeNivel2[i].entradaNivel2[j].bitModificado = false;
            tablasDeNivel2[i].entradaNivel2[j].bitPaginaEnSwap = false;
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

void swap_out(int nroDeTabla2, int entradaDeTabla2, int marco, t_memoria_data_holder memoriaData) {
    memoriaData.tablasDeNivel2[nroDeTabla2].entradaNivel2[entradaDeTabla2].bitPresencia = 0;
    if (memoriaData.tablasDeNivel2[nroDeTabla2].entradaNivel2[entradaDeTabla2].bitModificado) {
        int pagina = nroDeTabla2 * memoriaData.entradasPorTabla + entradaDeTabla2;
        sleep(memoriaData.retardoSwap);
        memcpy(memoriaData.inicio_archivo + memoriaData.tamanioPagina * pagina, memoriaData.memoriaPrincipal + marco * memoriaData.tamanioPagina, memoriaData.tamanioPagina);
        memoriaData.tablasDeNivel2[nroDeTabla2].entradaNivel2[entradaDeTabla2].bitPaginaEnSwap = true;
    }
}

void swap_in(int nroDeTabla2, int entradaDeTabla2, int marco, t_memoria_data_holder memoriaData) {
    memoriaData.tablasDeNivel2[nroDeTabla2].entradaNivel2[entradaDeTabla2].indiceMarco = marco;
    memoriaData.tablasDeNivel2[nroDeTabla2].entradaNivel2[entradaDeTabla2].bitPresencia = 1;
    memoriaData.tablasDeNivel2[nroDeTabla2].entradaNivel2[entradaDeTabla2].bitUso = 0;
    memoriaData.tablasDeNivel2[nroDeTabla2].entradaNivel2[entradaDeTabla2].bitModificado = 0;
    if (memoriaData.tablasDeNivel2[nroDeTabla2].entradaNivel2[entradaDeTabla2].bitPaginaEnSwap) {
        int pagina = nroDeTabla2 * memoriaData.entradasPorTabla + entradaDeTabla2;
        sleep(memoriaData.retardoSwap);
        memcpy(memoriaData.memoriaPrincipal + marco * memoriaData.tamanioPagina, memoriaData.inicio_archivo + memoriaData.tamanioPagina * pagina, memoriaData.tamanioPagina);
    }
}

void limpiar_tabla_nivel_2(int nroDeTabla2, t_memoria_data_holder memoriaData) {
    int entradasPorTabla = memoriaData.entradasPorTabla;
    t_tabla_nivel_2* tablasDeNivel2 = memoriaData.tablasDeNivel2;

    for (int i = 0; i < entradasPorTabla; i++) {
        tablasDeNivel2[nroDeTabla2].entradaNivel2[i].indiceMarco = -1;
        tablasDeNivel2[nroDeTabla2].entradaNivel2[i].bitPresencia = false;
        tablasDeNivel2[nroDeTabla2].entradaNivel2[i].bitUso = false;
        tablasDeNivel2[nroDeTabla2].entradaNivel2[i].bitModificado = false;
        tablasDeNivel2[nroDeTabla2].entradaNivel2[i].bitPaginaEnSwap = false;
    }
}

int obtener_marco(uint32_t nroDeTabla2, uint32_t entradaDeTabla2, t_memoria_data_holder memoriaData) {
    return memoriaData.tablasDeNivel2[nroDeTabla2].entradaNivel2[entradaDeTabla2].indiceMarco;
}

bool pagina_en_memoria(uint32_t nroDeTabla2, uint32_t entradaDeTabla2, t_memoria_data_holder memoriaData) {
    return memoriaData.tablasDeNivel2[nroDeTabla2].entradaNivel2[entradaDeTabla2].bitPresencia;
}

int obtener_pagina(uint32_t nroDeTabla2, uint32_t entradaDeTabla2, t_memoria_data_holder memoriaData) {
    return nroDeTabla2 * memoriaData.entradasPorTabla * memoriaData.entradasPorTabla + entradaDeTabla2;
}

void asignar_marco_a_pagina(int marco, uint32_t nroDeTabla2, uint32_t entradaDeTabla2, t_memoria_data_holder memoriaData) {
    memoriaData.tablasDeNivel2[nroDeTabla2].entradaNivel2[entradaDeTabla2].indiceMarco = marco;
    memoriaData.tablasDeNivel2[nroDeTabla2].entradaNivel2[entradaDeTabla2].bitPresencia = true;
}

int obtener_indice(int nroPagina, t_memoria_data_holder memoriaData) {
    return __obtener_entrada(nroPagina, memoriaData);
}

bool obtener_bit_uso(int nroPagina, t_memoria_data_holder memoriaData) {
    int entrada = __obtener_entrada(nroPagina, memoriaData);
    int nroDeTabla2 = __obtener_tabla2(nroPagina, memoriaData);
    return memoriaData.tablasDeNivel2[nroDeTabla2].entradaNivel2[entrada].bitUso;
}

bool obtener_bit_modificado(int nroPagina, t_memoria_data_holder memoriaData) {
    int entrada = __obtener_entrada(nroPagina, memoriaData);
    int nroDeTabla2 = __obtener_tabla2(nroPagina, memoriaData);
    return memoriaData.tablasDeNivel2[nroDeTabla2].entradaNivel2[entrada].bitModificado;
}

bool obtener_bit_pagina_en_swap(int nroDeTabla2, int entradaDeTabla2, t_memoria_data_holder memoriaData) {
    return memoriaData.tablasDeNivel2[nroDeTabla2].entradaNivel2[entradaDeTabla2].bitPaginaEnSwap;
}

void setear_bit_pagina_en_swap(int nroDeTabla2, int entradaDeTabla2, bool bitPaginaEnSwap, t_memoria_data_holder memoriaData) {
    memoriaData.tablasDeNivel2[nroDeTabla2].entradaNivel2[entradaDeTabla2].bitPaginaEnSwap = bitPaginaEnSwap;
}


void setear_bit_uso(int nroPagina, bool bitUso, t_memoria_data_holder memoriaData) {
    int entrada = __obtener_entrada(nroPagina, memoriaData);
    int nroDeTabla2 = __obtener_tabla2(nroPagina, memoriaData);
    memoriaData.tablasDeNivel2[nroDeTabla2].entradaNivel2[entrada].bitUso = bitUso;
}

int obtener_victima_clock_con_indice_inicial(uint32_t nroDeTabla2, int indicePagina, t_memoria_data_holder memoriaData) {
    for (; indicePagina < memoriaData.entradasPorTabla; indicePagina++) {
        bool presencia = memoriaData.tablasDeNivel2[nroDeTabla2].entradaNivel2[indicePagina].bitPresencia;
        if (!presencia) {
            continue;
        }
        bool uso = memoriaData.tablasDeNivel2[nroDeTabla2].entradaNivel2[indicePagina].bitUso;
        if (uso) {
            memoriaData.tablasDeNivel2[nroDeTabla2].entradaNivel2[indicePagina].bitUso = false;
        } else {
            return indicePagina;
        }
    }
    return -1;
}