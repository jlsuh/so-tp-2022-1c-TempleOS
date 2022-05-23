#include "pcb.h"

#include <commons/collections/list.h>
#include <stdint.h>

#include "estados.h"
#include "instruccion.h"
#include "stream.h"

struct t_pcb {
    uint32_t pid;
    uint32_t tamanio;
    t_list* instrucciones;
    uint64_t programCounter;
    uint32_t tablaPaginaPrimerNivel;
    double ultimaEstimacion;
    double ultimaEjecucion;
    uint8_t estadoActual;
    uint32_t tiempoDeBloqueo;
    int socketConsola;
    t_buffer* instructionsBuffer;
};

t_pcb* pcb_create(uint32_t pid, uint32_t tamanio, double estimacionInicial) {
    t_pcb* self = malloc(sizeof(*self));
    self->pid = pid;
    self->tamanio = tamanio;
    self->instrucciones = NULL;
    self->programCounter = 0;
    self->ultimaEstimacion = estimacionInicial;
    self->ultimaEjecucion = -1;
    self->estadoActual = NEW;
    self->tiempoDeBloqueo = 0;
    return self;
}

void pcb_destroy(t_pcb* self) {
    list_destroy_and_destroy_elements(self->instrucciones, instruccion_destroy);
    buffer_destroy(self->instructionsBuffer);
    free(self);
}

void pcb_responder_a_consola(t_pcb* self, uint8_t rta) {
    stream_send_empty_buffer(self->socketConsola, rta);
}

double pcb_estimar_srt(t_pcb* self, int alfa) {
    if (self->ultimaEjecucion == -1) {
        return self->ultimaEstimacion;  // estimación inicial
    }

    self->ultimaEstimacion = alfa * self->ultimaEjecucion + (1 - alfa) * self->ultimaEstimacion;
    return self->ultimaEstimacion;
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

void pcb_set_instrucciones(t_pcb* self, t_list* instrucciones) {
    self->instrucciones = instrucciones;
}

uint64_t pcb_get_program_counter(t_pcb* self) {
    return self->programCounter;
}

void pcb_set_program_counter(t_pcb* self, uint64_t pc) {
    self->programCounter = pc;
}

uint32_t pcb_get_tabla_pagina_primer_nivel(t_pcb* self) {
    return self->tablaPaginaPrimerNivel;
}

void pcb_set_tabla_pagina_primer_nivel(t_pcb* self, uint32_t tablaPaginaPrimerNivel) {
    self->tablaPaginaPrimerNivel = tablaPaginaPrimerNivel;
}

double pcb_get_est_actual(t_pcb* self) {
    return self->ultimaEstimacion;
}

void pcb_set_est_actual(t_pcb* self, double est) {
    self->ultimaEstimacion = est;
}

double pcb_get_ultima_ejecucion(t_pcb* self) {
    return self->ultimaEjecucion;
}

void pcb_set_ultima_ejecucion(t_pcb* self, double ej) {
    self->ultimaEjecucion = ej;
}

uint8_t pcb_get_estado_actual(t_pcb* self) {
    return self->estadoActual;
}

void pcb_set_estado_actual(t_pcb* self, uint8_t estado) {
    self->estadoActual = estado;
}

uint32_t pcb_get_tiempo_de_bloqueo(t_pcb* self) {
    return self->tiempoDeBloqueo;
}

void pcb_set_tiempo_de_bloqueo(t_pcb* self, uint32_t t) {
    self->tiempoDeBloqueo = t;
}

int pcb_get_socket(t_pcb* self) {
    return self->socketConsola;
}

void pcb_set_socket(t_pcb* self, int socket) {
    self->socketConsola = socket;
}

t_buffer* pcb_get_instruction_buffer(t_pcb* self) {
    return self->instructionsBuffer;
}

void pcb_set_instruction_buffer(t_pcb* self, t_buffer* buffer) {
    self->instructionsBuffer = buffer;
}