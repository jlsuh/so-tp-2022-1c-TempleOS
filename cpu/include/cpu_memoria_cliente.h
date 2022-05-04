#ifndef CPU_MEMORIA_CLIENTE_H_INCLUDED
#define CPU_MEMORIA_CLIENTE_H_INCLUDED

#include <stdint.h>

uint32_t leer_en_memoria(uint32_t direccion, uint32_t nroDeTabla1);
void escribir_en_memoria(uint32_t direccion, uint32_t nroDeTabla1, uint32_t valor);

#endif
