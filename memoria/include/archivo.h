#ifndef ARCHIVO_H_INCLUDED
#define ARCHIVO_H_INCLUDED

#include <stdint.h>

#include "memoria_data_holder.h"

void abrir_archivo(uint32_t tamanio, int nroTablaNivel1, t_memoria_data_holder* memoriaData);
void crear_archivo_de_proceso(uint32_t tamanio, int nroTablaNivel1, t_memoria_data_holder* memoriaData);
void cerrar_archivo(t_memoria_data_holder* memoriaData);
void eliminar_archivo_de_proceso(int nroTablaNivel1, t_memoria_data_holder* memoriaData);

#endif /* ARCHIVO_H_INCLUDED */