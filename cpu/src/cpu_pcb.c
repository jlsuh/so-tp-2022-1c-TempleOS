#include "cpu_pcb.h"

#include <stdlib.h>

#include "instruccion.h"

struct t_cpu_pcb {
    uint32_t pid;
    uint32_t programCounter;
    uint32_t tablaPaginaPrimerNivel;
    t_list* instrucciones;
};

t_cpu_pcb* cpu_pcb_create(uint32_t pid, uint32_t programCounter, uint32_t tablaPaginaPrimerNivel) {
    t_cpu_pcb* self = malloc(sizeof(*self));
    self->pid = pid;
    self->programCounter = programCounter;
    self->tablaPaginaPrimerNivel = tablaPaginaPrimerNivel;
    self->instrucciones = NULL;
    return self;
}

void cpu_pcb_destroy(t_cpu_pcb* self) {
    list_destroy_and_destroy_elements(self->instrucciones, instruccion_destroy);
    free(self);
}

uint32_t cpu_pcb_get_pid(t_cpu_pcb* self) {
    return self->pid;
}

uint32_t cpu_pcb_get_program_counter(t_cpu_pcb* self) {
    return self->programCounter;
}

void cpu_pcb_set_program_counter(t_cpu_pcb* self, uint32_t programCounter) {
    self->programCounter = programCounter;
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
