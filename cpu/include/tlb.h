#ifndef TLB_H_INCLUDED
#define TLB_H_INCLUDED

#include <stdint.h>

typedef struct t_tlb t_tlb;
typedef struct t_entrada_tlb t_entrada_tlb;

int entrada_tlb_get_instante_de_tiempo(t_tlb*, uint32_t index);
int entrada_tlb_get_marco(t_tlb*, uint32_t index);
int entrada_tlb_get_numero_pagina(t_tlb*, uint32_t index);
int tlb_get_marco(t_tlb*, uint32_t numeroPagina);
t_tlb* tlb_create(uint32_t cantidadDeEntradas, char* algoritmoReemplazo);
uint32_t tlb_get_cantidad_entradas_totales(t_tlb*);
void tlb_destroy(t_tlb*);
void tlb_flush(t_tlb*);
void tlb_registrar_entrada_en_tlb(t_tlb*, uint32_t numeroPagina, uint32_t marco);

#endif
