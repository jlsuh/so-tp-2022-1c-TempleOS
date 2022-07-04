#include "algoritmos.h"

#include "tabla_nivel_1.h"
#include "tabla_nivel_2.h"

typedef struct
{
    int indicePagina;
    bool bitUso;
    bool bitModificado;
} t_entrada_nivel_2_algoritmo;

int seleccionar_victima_clock(uint32_t nroTablaNivel1, t_memoria_data_holder memoriaData) {
    int tamanio = memoriaData.cantidadMarcosProceso;
    t_entrada_nivel_2_algoritmo tablasDeNivel2[tamanio];
    // TODO recuperar paginas en memoria, y luego analizarlo y por ultimo actualizar las modificaciones que se hayan hecho (bitDeUso)

    int puntero = obtener_puntero(nroTablaNivel1, memoriaData);
    int indiceTabla2 = puntero / memoriaData.entradasPorTabla;
    int indicePagina = puntero % memoriaData.entradasPorTabla;
    int nroDeTabla2;
    int victima;
    // Si no es pagina inicial
    if (!indicePagina) {
        nroDeTabla2 = obtener_tabla_de_nivel_2(nroTablaNivel1, indiceTabla2, memoriaData);
        victima = obtener_victima_clock_con_indice_inicial(nroDeTabla2, indicePagina, memoriaData);
    }

    for (int i = 0; i < memoriaData.entradasPorTabla; i++) {
        int nroDeTabla2 = obtener_tabla_de_nivel_2(nroTablaNivel1, indiceTabla2 + i, memoriaData);
    }
}

int seleccionar_victima_clock_modificado(uint32_t nroTablaNivel1, t_memoria_data_holder memoriaData){

}