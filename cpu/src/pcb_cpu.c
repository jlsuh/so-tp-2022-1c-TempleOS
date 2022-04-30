#include "pcb_cpu.h"

#include <commons/collections/list.h>
#include <stdint.h>
#include <stdlib.h>

struct t_pcb_cpu {
    uint32_t pid;
    uint64_t programCounter;
    uint32_t tablaPaginaPrimerNivel;
    uint32_t tiempoDeBloqueo;
    t_list* instrucciones;
};

t_pcb_cpu* pcb_cpu_create(uint32_t pid, uint64_t programCounter, uint32_t tablaPaginaPrimerNivel) {
    t_pcb_cpu* self = malloc(sizeof(*self));
    self->pid = pid;
    self->programCounter = programCounter;
    self->tablaPaginaPrimerNivel = tablaPaginaPrimerNivel;
    self->tiempoDeBloqueo = 0;
    self->instrucciones = NULL;
    return self;
}

uint32_t pcb_cpu_get_pid(t_pcb_cpu* self) {
    return self->pid;
}

uint64_t pcb_cpu_get_program_counter(t_pcb_cpu* self) {
    return self->programCounter;
}

void pcb_cpu_set_program_counter(t_pcb_cpu* self, uint64_t pc) {
    self->programCounter = pc;
}

uint32_t pcb_cpu_get_tabla_pagina_primer_nivel(t_pcb_cpu* self) {
    return self->tablaPaginaPrimerNivel;
}

void pcb_cpu_set_tabla_pagina_primer_nivel(t_pcb_cpu* self, uint32_t tablaPaginaPrimerNivel) {
    self->tablaPaginaPrimerNivel = tablaPaginaPrimerNivel;
}

uint32_t pcb_cpu_get_tiempo_de_bloq(t_pcb_cpu* self) {
    return self->tiempoDeBloqueo;
}

void pcb_cpu_set_tiempo_de_bloq(t_pcb_cpu* self, uint32_t t) {
    self->tiempoDeBloqueo = t;
}

t_list* pcb_cpu_get_instrucciones(t_pcb_cpu* self) {
    return self->instrucciones;
}

void pcb_cpu_set_instrucciones(t_pcb_cpu* self, t_list* instrucciones) {
    self->instrucciones = instrucciones;
}
