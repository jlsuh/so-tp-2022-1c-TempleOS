#ifndef TABLA_SUSPENDIDO_H_INCLUDED
#define TABLA_SUSPENDIDO_H_INCLUDED

#include <commons/collections/dictionary.h>
#include <stdint.h>

#include "memoria_data_holder.h"

typedef struct t_proceso_suspendido t_proceso_suspendido;

t_dictionary* crear_tabla_de_suspendidos(void);
void suspender_proceso(uint32_t nroDeTabla1, t_memoria_data_holder* memoriaData);
void despertar_proceso(uint32_t nroDeTabla1, t_memoria_data_holder* memoriaData);
bool esta_suspendido(uint32_t nroTablaNivel1, t_memoria_data_holder* memoriaData);
void eliminar_de_tabla_suspendidos(uint32_t nroTablaNivel1, t_memoria_data_holder* memoriaData);

#endif /* TABLA_SUSPENDIDO_H_INCLUDED */
