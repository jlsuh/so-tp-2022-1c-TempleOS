#include "pcb.h"

#include <commons/collections/list.h>
#include <stdint.h>

#include "estados.h"

struct t_pcb {
    uint32_t pid;
    uint32_t tamanio;
    t_list* instrucciones;
    uint64_t programCounter;  // Contiene un índice de la t_list instrucciones
    uint32_t tablaPaginaPrimerNivel;
    double ultimaEstimacion;
    double ultimaEjecucion;
    uint8_t estadoActual;
};

t_pcb* pcb_create(uint32_t pid, uint32_t tamanio, double estimacionInicial) {
    t_pcb* self = malloc(sizeof(*self));
    self->pid = pid;
    self->tamanio = tamanio;
    self->instrucciones = list_create();
    self->programCounter = 0;
    self->ultimaEstimacion = estimacionInicial;
    self->ultimaEjecucion = -1;
    self->estadoActual = NEW;
    return self;
}

uint32_t pcb_get_pid(t_pcb* self) {
    return self->pid;
}

uint32_t pcb_get_tamanio(t_pcb* self) {
    return self->tamanio;
}

t_list* pcb_get_instrucciones(t_pcb* self) {
    return self->instrucciones;
}

uint64_t pcb_get_program_counter(t_pcb* self) {
    return self->programCounter;
}

uint32_t pcb_get_tabla_pagina_primer_nivel(t_pcb* self) {
    return self->tablaPaginaPrimerNivel;
}

double pcb_get_est_actual(t_pcb* self) {
    return self->ultimaEstimacion;
}

uint8_t pcb_get_estado_actual(t_pcb* self) {
    return self->estadoActual;
}

void pcb_set_estado_actual(t_pcb* self, uint8_t estado) {
    self->estadoActual = estado;
}
