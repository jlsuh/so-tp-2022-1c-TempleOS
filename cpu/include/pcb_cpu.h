#ifndef PCB_CPU_H_INCLUDED
#define PCB_CPU_H_INCLUDED

#include <commons/collections/list.h>
#include <stdint.h>

typedef struct t_pcb_cpu t_pcb_cpu;

t_pcb_cpu* pcb_cpu_create(uint32_t pid, uint64_t programCounter, uint32_t tablaPaginaPrimerNivel);

uint32_t pcb_cpu_get_pid(t_pcb_cpu* self);

uint64_t pcb_cpu_get_program_counter(t_pcb_cpu* self);
void pcb_cpu_set_program_counter(t_pcb_cpu* self, uint64_t);

uint32_t pcb_cpu_get_tabla_pagina_primer_nivel(t_pcb_cpu* self);
void pcb_cpu_set_tabla_pagina_primer_nivel(t_pcb_cpu* self, uint32_t);

uint32_t pcb_cpu_get_tiempo_de_bloq(t_pcb_cpu* self);
void pcb_cpu_set_tiempo_de_bloq(t_pcb_cpu* self, uint32_t t);

t_list* pcb_cpu_get_instrucciones(t_pcb_cpu* self);

#endif
