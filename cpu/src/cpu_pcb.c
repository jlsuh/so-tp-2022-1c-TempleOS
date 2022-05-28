#include "cpu_pcb.h"

#include <commons/collections/list.h>
#include <stdint.h>
#include <stdlib.h>

struct t_cpu_pcb {
    uint32_t pid;
    uint64_t programCounter;
    uint32_t tablaPaginaPrimerNivel;
    t_list* instrucciones;
};

t_cpu_pcb* cpu_pcb_create(uint32_t pid, uint64_t programCounter, uint32_t tablaPaginaPrimerNivel) {
    t_cpu_pcb* self = malloc(sizeof(*self));
    self->pid = pid;
    self->programCounter = programCounter;
    self->tablaPaginaPrimerNivel = tablaPaginaPrimerNivel;
    self->instrucciones = NULL;
    return self;
}

uint32_t cpu_pcb_get_pid(t_cpu_pcb* self) {
    return self->pid;
}

uint64_t cpu_pcb_get_program_counter(t_cpu_pcb* self) {
    return self->programCounter;
}

void cpu_pcb_set_program_counter(t_cpu_pcb* self, uint64_t pc) {
    self->programCounter = pc;
}

uint32_t cpu_pcb_get_tabla_pagina_primer_nivel(t_cpu_pcb* self) {
    return self->tablaPaginaPrimerNivel;
}

t_list* cpu_pcb_get_instrucciones(t_cpu_pcb* self) {
    return self->instrucciones;
}

void cpu_pcb_set_instrucciones(t_cpu_pcb* self, t_list* instrucciones) {
    self->instrucciones = instrucciones;
}
