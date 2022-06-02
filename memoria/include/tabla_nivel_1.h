#ifndef TABLA_NIVEL_1_H_INCLUDED
#define TABLA_NIVEL_1_H_INCLUDED

typedef struct t_tabla_nivel_1 t_tabla_nivel_1;


t_tabla_nivel_1* crear_tablas_de_nivel_1(int cantidadProcesosMax, int entradasPorTabla);
int obtener_tabla_de_nivel_2_pagina(int nroPagina, int entradasPorTabla, t_tabla_nivel_1* tablasDeNivel1);
uint32_t obtener_tabla_de_nivel_2(uint32_t nroDeTabla1, uint32_t entradaDeTabla1, t_tabla_nivel_1* tablasDeNivel1);
int obtener_tabla_de_nivel_1(uint32_t nroDeTabla2, int cantidadProcesosMax, int entradasPorTabla, t_tabla_nivel_1* tablasDeNivel1);
int obtener_tabla_libre_de_nivel_1(int cantidadProcesosMax, t_tabla_nivel_1* tablasDeNivel1);
void asignar_tamanio_tabla_nivel_1(int nroTablaNivel1, uint32_t tamanio, t_tabla_nivel_1* tablasDeNivel1);



#endif /* TABLA_NIVEL_1_H_INCLUDED */
