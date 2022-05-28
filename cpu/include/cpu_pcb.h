#ifndef CPU_PCB_H_INCLUDED
#define CPU_PCB_H_INCLUDED

#include <commons/collections/list.h>
#include <stdint.h>

typedef struct t_cpu_pcb t_cpu_pcb;

t_list* cpu_pcb_get_instrucciones(t_cpu_pcb*);
t_cpu_pcb* cpu_pcb_create(uint32_t pid, uint64_t programCounter, uint32_t tablaPaginaPrimerNivel);
uint32_t cpu_pcb_get_pid(t_cpu_pcb*);
uint32_t cpu_pcb_get_tabla_pagina_primer_nivel(t_cpu_pcb*);
uint64_t cpu_pcb_get_program_counter(t_cpu_pcb*);
void cpu_pcb_set_instrucciones(t_cpu_pcb*, t_list* instrucciones);
void cpu_pcb_set_program_counter(t_cpu_pcb*, uint64_t pc);
void cpu_pcb_set_tabla_pagina_primer_nivel(t_cpu_pcb*, uint32_t tablaPaginaPrimerNivel)

#endif
