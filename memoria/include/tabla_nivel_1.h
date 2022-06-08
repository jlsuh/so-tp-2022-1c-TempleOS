#ifndef TABLA_NIVEL_1_H_INCLUDED
#define TABLA_NIVEL_1_H_INCLUDED

#include <stdint.h>
#include <memoria_data_holder.h>

typedef struct t_tabla_nivel_1 t_tabla_nivel_1;

int * obtener_marcos(uint32_t nroTablaNivel1,t_memoria_data_holder memoriaData);
int obtener_tabla_de_nivel_2(uint32_t nroDeTabla1,uint32_t entradaDeTabla1,t_memoria_data_holder memoriaData);
int obtener_tabla_de_nivel_2_pagina(int nroPagina,t_memoria_data_holder memoriaData);
t_tabla_nivel_1 * crear_tablas_de_nivel_1(t_memoria_data_holder memoriaData);
uint32_t asignar_tabla_nivel_1(int indiceTablaNivel1,uint32_t tamanio,t_memoria_data_holder memoriaData);
uint32_t obtener_indice_tabla_nivel_1(uint32_t nroTablaNivel1,t_memoria_data_holder memoriaData);
uint32_t obtener_tabla_de_nivel_1(uint32_t nroDeTabla2,t_memoria_data_holder memoriaData);
uint32_t obtener_tabla_libre_de_nivel_1(t_memoria_data_holder memoriaData);



#endif /* TABLA_NIVEL_1_H_INCLUDED */
