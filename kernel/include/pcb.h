#ifndef PCB_H_INCLUDED
#define PCB_H_INCLUDED

#include <commons/collections/list.h>
#include <stdint.h>

typedef struct t_instruccion t_instruccion;
typedef struct t_pcb t_pcb;

uint32_t pcb_get_pid(t_pcb*);
uint8_t pcb_get_estado_actual(t_pcb*);
void pcb_set_estado_actual(t_pcb*, uint8_t estado);
uint32_t pcb_get_pid(t_pcb*);
uint32_t pcb_get_tamanio(t_pcb*);
t_list* pcb_get_instrucciones(t_pcb*);
uint64_t pcb_get_program_counter(t_pcb*);
double pcb_get_ultima_est(t_pcb*);
uint32_t pcb_get_tabla_pagina_primer_nivel(t_pcb*);
t_pcb* pcb_create(uint32_t pid, uint32_t tamanio, double estimacionRafaga);

#endif
