#ifndef TABLA_NIVEL_2_H_INCLUDED
#define TABLA_NIVEL_2_H_INCLUDED

#include <stdint.h>
#include "memoria_data_holder.h"

typedef struct t_tabla_nivel_2 t_tabla_nivel_2;

t_tabla_nivel_2* crear_tablas_de_nivel_2(t_memoria_data_holder* memoriaData);
void actualizar_lectura_pagina(int nroPagina, int nroTablaNivel2, t_memoria_data_holder* memoriaData);
void actualizar_escritura_pagina(int nroPagina, int nroTablaNivel2, t_memoria_data_holder* memoriaData);
int obtener_marco(uint32_t nroDeTabla2, uint32_t entradaDeTabla2, t_memoria_data_holder* memoriaData);
void swap_out(int nroDeTabla2, int entradaDeTabla2, int marco, t_memoria_data_holder* memoriaData);
void swap_in(int nroDeTabla2, int entradaDeTabla2, int marco, t_memoria_data_holder* memoriaData);
bool pagina_en_memoria(uint32_t nroDeTabla2, uint32_t entradaDeTabla2, t_memoria_data_holder* memoriaData);
void limpiar_tabla_nivel_2(int nroDeTabla2, t_memoria_data_holder* memoriaData);
int obtener_pagina(uint32_t nroDeTabla2, uint32_t entradaDeTabla2, t_memoria_data_holder* memoriaData);
void asignar_marco_a_pagina(int marco, uint32_t nroDeTabla2, uint32_t entradaDeTabla2, t_memoria_data_holder* memoriaData);
int obtener_victima_clock_con_indice_inicial(uint32_t nroDeTabla2, int indicePagina, t_memoria_data_holder* memoriaData);
int obtener_indice(int nroPagina, t_memoria_data_holder* memoriaData);
bool obtener_bit_uso(int nroPagina, t_memoria_data_holder* memoriaData);
bool obtener_bit_modificado(int nroPagina, t_memoria_data_holder* memoriaData);
void setear_bit_uso(int nroPagina, bool bitUso, t_memoria_data_holder* memoriaData);
bool obtener_bit_pagina_en_swap(int nroDeTabla2, int entradaDeTabla2, t_memoria_data_holder* memoriaData);
void setear_bit_pagina_en_swap(int nroDeTabla2, int entradaDeTabla2, bool bitPaginaEnSwap, t_memoria_data_holder* memoriaData);

#endif /* TABLA_NIVEL_2_H_INCLUDED */
