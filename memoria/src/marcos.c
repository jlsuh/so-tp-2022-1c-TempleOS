#include "marcos.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>


struct t_marcos
{
    bool enUso;
    bool reservado;
    int pagina;
};

t_marcos* crear_marcos(int cantTotalMarcos) {
    t_marcos* marcos = malloc(cantTotalMarcos * sizeof(*marcos));
    for(int i = 0; i < cantTotalMarcos; i++) {
        marcos[i].enUso = false;
        marcos[i].reservado = false;
        marcos[i].pagina = NULL;
    }
    return marcos;
}

int obtener_pagina_de_un_marco(uint32_t direccionFisica, int tamanioPagina, t_marcos* marcos) {
    int marco = direccionFisica / tamanioPagina;
    return marcos[marco].pagina;
}

int* reservar_marcos_libres(int cantidadMarcosMax, int cantMarcosAsignar, t_marcos* marcos) {
    int* marcosLibres = malloc(cantMarcosAsignar * sizeof(int));
    int cantMarcosAsignados = 0;
    for (int indiceMarco = 0; indiceMarco < cantidadMarcosMax && cantMarcosAsignados < cantMarcosAsignar; indiceMarco++) {
        if (!marcos[indiceMarco].reservado) {
            marcos[indiceMarco].reservado = true;
            marcosLibres[cantMarcosAsignados] = indiceMarco;
            cantMarcosAsignados++;
        }
    }

    if(cantMarcosAsignados == cantMarcosAsignar) {
        return marcosLibres;
    }
        return NULL;
}
