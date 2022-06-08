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
        marcos[i].pagina = -1;
    }
    return marcos;
}

int obtener_pagina_de_un_marco(uint32_t direccionFisica, int tamanioPagina, t_marcos* marcos) {
    int marco = direccionFisica / tamanioPagina;
    return marcos[marco].pagina;
}