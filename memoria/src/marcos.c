#include "marcos.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

struct t_marcos {
    bool enUso;
    int pagina;
};

t_marcos* crear_marcos(t_memoria_data_holder memoriaData) {
    int cantTotalMarcos = memoriaData.cantidadMarcosMax;

    t_marcos* marcos = malloc(cantTotalMarcos * sizeof(*marcos));
    for (int i = 0; i < cantTotalMarcos; i++) {
        marcos[i].enUso = false;
        marcos[i].pagina = -1;
    }
    return marcos;
}

int obtener_pagina_de_un_marco(uint32_t direccionFisica, t_memoria_data_holder memoriaData) {
    int tamanioPagina = memoriaData.tamanioPagina;
    t_marcos* marcos = memoriaData.marcos;

    int marco = direccionFisica / tamanioPagina;
    return marcos[marco].pagina;
}

int obtener_marco_libre(int* marcos, t_memoria_data_holder memoriaData){
    for(int i = 0; i < memoriaData.cantidadMarcosProceso; i++){
        int indiceMarco = marcos[i];
        if(!memoriaData.marcos[indiceMarco].enUso){
            return marcos[i];
        }
    }
    return -1;
}