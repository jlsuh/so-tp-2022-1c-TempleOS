#ifndef TLB_H_INCLUDED
#define TLB_H_INCLUDED

#include <stdint.h>

typedef struct t_tlb t_tlb;

t_tlb* tlb_create(uint32_t cantidadDeEntradas, char* algoritmoReemplazo);
uint32_t tlb_get_marco(t_tlb* self, uint32_t entradaTablaNivel1, uint32_t entradaTablaNivel2);
void tlb_flush(t_tlb* self);
void tlb_registrar_entrada_en_tlb(t_tlb* self, uint32_t entradaTablaNivel1, uint32_t entradaTablaNivel2, uint32_t marco);

#endif
