#ifndef MARCOS_H_INCLUDED
#define MARCOS_H_INCLUDED

#include <stdint.h>
#include "memoria_data_holder.h"

typedef struct t_marcos t_marcos;

t_marcos* crear_marcos(t_memoria_data_holder* memoriaData);
int obtener_pagina_de_un_marco(int marco, t_memoria_data_holder* memoriaData);
int obtener_pagina_de_direccion_fisica(uint32_t direccionFisica, t_memoria_data_holder* memoriaData);
int obtener_marco_libre(int* marcos, t_memoria_data_holder* memoriaData);
void limpiar_marco(int indice, t_memoria_data_holder* memoriaData);
void asignar_pagina_a_marco(int pagina, int marco, t_memoria_data_holder* memoriaData);

#endif /* MARCOS_H_INCLUDED */
