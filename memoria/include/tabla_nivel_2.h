#ifndef TABLA_NIVEL_2_H_INCLUDED
#define TABLA_NIVEL_2_H_INCLUDED

#include <stdint.h>

typedef struct t_tabla_nivel_2 t_tabla_nivel_2;

t_tabla_nivel_2* crear_tablas_de_nivel_2(int cantidadProcesosMax, int entradasPorTabla);
void actualizar_lectura_pagina(int nroPagina, int nroTablaNivel2, int entradasPorTabla, t_tabla_nivel_2* tablasDeNivel2);
void actualizar_escritura_pagina(int nroPagina, int nroTablaNivel2, int entradasPorTabla, t_tabla_nivel_2* tablasDeNivel2);
int obtener_marco(uint32_t nroDeTabla2, uint32_t entradaDeTabla2, t_tabla_nivel_2* tablasDeNivel2);

#endif /* TABLA_NIVEL_2_H_INCLUDED */
