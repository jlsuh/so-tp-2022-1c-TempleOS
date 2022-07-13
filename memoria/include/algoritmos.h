#ifndef ALGORITMOS_H_INCLUDED
#define ALGORITMOS_H_INCLUDED

#include "memoria_data_holder.h"
#include <stdint.h>

int seleccionar_victima_clock(uint32_t nroTablaNivel1, t_memoria_data_holder* memoriaData);
int seleccionar_victima_clock_modificado(uint32_t nroTablaNivel1, t_memoria_data_holder* memoriaData);


#endif /* ALGORITMOS_H_INCLUDED */
