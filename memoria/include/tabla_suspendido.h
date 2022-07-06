#ifndef TABLA_SUSPENDIDO_H_INCLUDED
#define TABLA_SUSPENDIDO_H_INCLUDED

#include <commons/collections/dictionary.h>
#include "memoria_data_holder.h"
#include <stdint.h>

typedef struct t_proceso_suspendido t_proceso_suspendido;

t_dictionary* crear_tabla_de_suspendidos(void);
void suspender_proceso(uint32_t nroDeTabla1, t_memoria_data_holder memoriaData);
void despertar_proceso(uint32_t nroDeTabla1, t_memoria_data_holder memoriaData);

#endif /* TABLA_SUSPENDIDO_H_INCLUDED */
