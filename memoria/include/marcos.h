#ifndef MARCOS_H_INCLUDED
#define MARCOS_H_INCLUDED

typedef struct t_marcos t_marcos;

t_marcos* crear_marcos(int cantTotalMarcos);
int obtener_pagina_de_un_marco(uint32_t direccionFisica, int tamanioPagina, t_marcos* marcos);

#endif /* MARCOS_H_INCLUDED */
