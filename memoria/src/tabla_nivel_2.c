#include "tabla_nivel_2.h"

#include <stdlib.h>
#include <string.h>

#include "common_utils.h"
#include "marcos.h"
#include "tabla_nivel_1.h"

typedef struct {
    int indiceMarco;
    bool bitPresencia;
    bool bitUso;
    bool bitModificado;
    bool bitPaginaEnSwap;
} t_entrada_nivel_2;

struct t_tabla_nivel_2 {
    t_entrada_nivel_2* entradaNivel2;
};

static int __obtener_tabla2(int nroPagina, t_memoria_data_holder* memoriaData) {
    return nroPagina / memoriaData->entradasPorTabla;
}

static int __obtener_entrada(int nroPagina, t_memoria_data_holder* memoriaData) {
    int entradasPorTabla = memoriaData->entradasPorTabla;

    return (nroPagina % (entradasPorTabla * entradasPorTabla)) % entradasPorTabla;
}

t_tabla_nivel_2* crear_tablas_de_nivel_2(t_memoria_data_holder* memoriaData) {
    int cantidadProcesosMax = memoriaData->cantidadProcesosMax;
    int entradasPorTabla = memoriaData->entradasPorTabla;
    int cantidadTablasNivel2Max = cantidadProcesosMax * entradasPorTabla;
    t_tabla_nivel_2* tablasDeNivel2 = malloc(cantidadTablasNivel2Max * sizeof(*tablasDeNivel2));
    log_info(memoriaData->memoriaLogger, "\e[1;93mSe crean las tablas de nivel 2\e[0m");
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
    log_info(memoriaData->memoriaLogger, "Se creó %d tablas con %d entradas de paginas", cantidadTablasNivel2Max, entradasPorTabla);
    return tablasDeNivel2;
}

void actualizar_escritura_pagina(int nroPagina, int nroTablaNivel2, t_memoria_data_holder* memoriaData) {
    t_tabla_nivel_2* tablasDeNivel2 = memoriaData->tablasDeNivel2;
    int entrada = __obtener_entrada(nroPagina, memoriaData);
    tablasDeNivel2[nroTablaNivel2].entradaNivel2[entrada].bitUso = true;
    tablasDeNivel2[nroTablaNivel2].entradaNivel2[entrada].bitModificado = true;
}

void actualizar_lectura_pagina(int nroPagina, int nroTablaNivel2, t_memoria_data_holder* memoriaData) {
    t_tabla_nivel_2* tablasDeNivel2 = memoriaData->tablasDeNivel2;
    int entrada = __obtener_entrada(nroPagina, memoriaData);
    tablasDeNivel2[nroTablaNivel2].entradaNivel2[entrada].bitUso = true;
}

void swap_out(int nroDeTabla2, int entradaDeTabla2, int marco, t_memoria_data_holder* memoriaData) {
    log_info(memoriaData->memoriaLogger, "\e[1;93mSe realiza un swap out\e[0m");
    memoriaData->tablasDeNivel2[nroDeTabla2].entradaNivel2[entradaDeTabla2].bitPresencia = false;
    int pagina = nroDeTabla2 * memoriaData->entradasPorTabla + entradaDeTabla2;
    int tablaNivel1 = obtener_tabla_de_nivel_1(nroDeTabla2, memoriaData);
    if (memoriaData->tablasDeNivel2[nroDeTabla2].entradaNivel2[entradaDeTabla2].bitModificado) {
        int paginaLocal = obtener_indice(pagina, memoriaData);
        intervalo_de_pausa(memoriaData->retardoSwap);
        memcpy((void*)(memoriaData->inicio_archivo + memoriaData->tamanioPagina * paginaLocal), (void*)(memoriaData->memoriaPrincipal + marco * memoriaData->tamanioPagina), memoriaData->tamanioPagina);
        memoriaData->tablasDeNivel2[nroDeTabla2].entradaNivel2[entradaDeTabla2].bitPaginaEnSwap = true;
        log_info(memoriaData->memoriaLogger, "<\e[0;96m(Acceso a disco)\e[0m> Se escribió en disco la <\e[1;95mpágina [%d] del proceso [%d]\e[0m> con <\e[1;95mmarco [%d]\e[0m> <Bit Modificado = 1>", paginaLocal, tablaNivel1, marco);
    } else {
        log_info(memoriaData->memoriaLogger, "La <\e[1;95mpágina [%d] del proceso [%d]\e[0m> con <\e[1;95mmarco [%d]\e[0m> no se encuentra modificada <Bit Modificado = 0>", pagina, tablaNivel1, marco);
    }
    asignar_pagina_a_marco(-1, marco, memoriaData);
}

void swap_in(int nroDeTabla2, int entradaDeTabla2, int marco, t_memoria_data_holder* memoriaData) {
    log_info(memoriaData->memoriaLogger, "\e[1;93mSe realiza un swap in\e[0m");
    memoriaData->tablasDeNivel2[nroDeTabla2].entradaNivel2[entradaDeTabla2].indiceMarco = marco;
    memoriaData->tablasDeNivel2[nroDeTabla2].entradaNivel2[entradaDeTabla2].bitPresencia = true;
    memoriaData->tablasDeNivel2[nroDeTabla2].entradaNivel2[entradaDeTabla2].bitUso = false;
    memoriaData->tablasDeNivel2[nroDeTabla2].entradaNivel2[entradaDeTabla2].bitModificado = false;
    int pagina = nroDeTabla2 * memoriaData->entradasPorTabla + entradaDeTabla2;
    int tablaNivel1 = obtener_tabla_de_nivel_1(nroDeTabla2, memoriaData);
    if (memoriaData->tablasDeNivel2[nroDeTabla2].entradaNivel2[entradaDeTabla2].bitPaginaEnSwap) {
        int paginaLocal = obtener_indice(pagina, memoriaData);
        intervalo_de_pausa(memoriaData->retardoSwap);
        memcpy((void*)(memoriaData->memoriaPrincipal + marco * memoriaData->tamanioPagina), (void*)(memoriaData->inicio_archivo + memoriaData->tamanioPagina * paginaLocal), memoriaData->tamanioPagina);
        log_info(memoriaData->memoriaLogger, "<\e[0;96m(Acceso a disco)\e[0m> Se trae de disco la <\e[1;95mpágina [%d] del proceso [%d]\e[0m> al <\e[1;95mmarco [%d]\e[0m> <Bit Página en Swap = 1>", pagina, tablaNivel1, marco);
    } else {
        log_info(memoriaData->memoriaLogger, "La <\e[1;95mpágina [%d] del proceso [%d]\e[0m> no tiene datos en disco para escribir en <\e[1;95mmarco [%d]\e[0m> <Bit Página en Swap = 0>", pagina, tablaNivel1, marco);
    }
    asignar_pagina_a_marco(pagina, marco, memoriaData);
}

void limpiar_tabla_nivel_2(int nroDeTabla2, t_memoria_data_holder* memoriaData) {
    int entradasPorTabla = memoriaData->entradasPorTabla;
    t_tabla_nivel_2* tablasDeNivel2 = memoriaData->tablasDeNivel2;
    for (int i = 0; i < entradasPorTabla; i++) {
        tablasDeNivel2[nroDeTabla2].entradaNivel2[i].indiceMarco = -1;
        tablasDeNivel2[nroDeTabla2].entradaNivel2[i].bitPresencia = false;
        tablasDeNivel2[nroDeTabla2].entradaNivel2[i].bitUso = false;
        tablasDeNivel2[nroDeTabla2].entradaNivel2[i].bitModificado = false;
        tablasDeNivel2[nroDeTabla2].entradaNivel2[i].bitPaginaEnSwap = false;
    }
}

int obtener_marco(uint32_t nroDeTabla2, uint32_t entradaDeTabla2, t_memoria_data_holder* memoriaData) {
    return memoriaData->tablasDeNivel2[nroDeTabla2].entradaNivel2[entradaDeTabla2].indiceMarco;
}

bool pagina_en_memoria(uint32_t nroDeTabla2, uint32_t entradaDeTabla2, t_memoria_data_holder* memoriaData) {
    return memoriaData->tablasDeNivel2[nroDeTabla2].entradaNivel2[entradaDeTabla2].bitPresencia;
}

int obtener_pagina(uint32_t nroDeTabla2, uint32_t entradaDeTabla2, t_memoria_data_holder* memoriaData) {
    return nroDeTabla2 * memoriaData->entradasPorTabla + entradaDeTabla2;
}

int obtener_indice(int nroPagina, t_memoria_data_holder* memoriaData) {
    return nroPagina % (memoriaData->entradasPorTabla * memoriaData->entradasPorTabla);
}

bool obtener_bit_uso(int nroPagina, t_memoria_data_holder* memoriaData) {
    int entrada = __obtener_entrada(nroPagina, memoriaData);
    int nroDeTabla2 = __obtener_tabla2(nroPagina, memoriaData);
    return memoriaData->tablasDeNivel2[nroDeTabla2].entradaNivel2[entrada].bitUso;
}

bool obtener_bit_modificado(int nroPagina, t_memoria_data_holder* memoriaData) {
    int entrada = __obtener_entrada(nroPagina, memoriaData);
    int nroDeTabla2 = __obtener_tabla2(nroPagina, memoriaData);
    return memoriaData->tablasDeNivel2[nroDeTabla2].entradaNivel2[entrada].bitModificado;
}

bool obtener_bit_pagina_en_swap(int nroDeTabla2, int entradaDeTabla2, t_memoria_data_holder* memoriaData) {
    return memoriaData->tablasDeNivel2[nroDeTabla2].entradaNivel2[entradaDeTabla2].bitPaginaEnSwap;
}

void setear_bit_pagina_en_swap(int nroDeTabla2, int entradaDeTabla2, bool bitPaginaEnSwap, t_memoria_data_holder* memoriaData) {
    memoriaData->tablasDeNivel2[nroDeTabla2].entradaNivel2[entradaDeTabla2].bitPaginaEnSwap = bitPaginaEnSwap;
}

void setear_bit_uso(int nroPagina, bool bitUso, t_memoria_data_holder* memoriaData) {
    int entrada = __obtener_entrada(nroPagina, memoriaData);
    int nroDeTabla2 = __obtener_tabla2(nroPagina, memoriaData);
    memoriaData->tablasDeNivel2[nroDeTabla2].entradaNivel2[entrada].bitUso = bitUso;
}
