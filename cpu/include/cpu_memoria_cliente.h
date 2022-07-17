#ifndef CPU_MEMORIA_CLIENTE_H_INCLUDED
#define CPU_MEMORIA_CLIENTE_H_INCLUDED

#include "tlb.h"

uint32_t cpu_leer_en_memoria(t_tlb*, int toSocket, uint32_t direccionALeer, uint32_t numberoDeTabla1);
void cpu_escribir_en_memoria(t_tlb*, int toSocket, uint32_t direccionAEscribir, uint32_t numberoDeTabla1, uint32_t contenidoAEscribir);

#endif
