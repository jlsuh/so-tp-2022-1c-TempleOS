#ifndef CPU_MEMORIA_CLIENTE_H_INCLUDED
#define CPU_MEMORIA_CLIENTE_H_INCLUDED

#include <stdint.h>

uint32_t leer_en_memoria(int toSocket, uint32_t direccionALeer, uint32_t numberoDeTabla1);
void escribir_en_memoria(int toSocket, uint32_t direccionAEscribir, uint32_t numberoDeTabla1, uint32_t contenidoAEscribir);

#endif
