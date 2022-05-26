#include "pcb.h"

#include <commons/collections/list.h>
#include <stdbool.h>
#include <stdint.h>

#include "estados.h"
#include "instruccion.h"
#include "stream.h"

struct t_pcb {
    uint32_t pid;
    uint32_t tamanio;
    uint64_t programCounter;
    uint32_t tablaPaginaPrimerNivel;
    double ultimaEstimacion;
    double ultimaEjecucion;
    uint8_t estadoActual;
    uint32_t tiempoDeBloqueo;
    int socketConsola;
    t_buffer* instructionsBuffer;
    pthread_mutex_t* mutex;
    time_t tiempoInicialBloqueado;
    time_t tiempoFinalBloqueado;
    bool tiempoFinalBloqueadoEstablecido;
    int vecesBloqueado;
};

static void __pcb_marcar_tiempo_final_como_establecido(t_pcb* self) {
    self->tiempoFinalBloqueadoEstablecido = true;
}

static void __pcb_marcar_tiempo_final_bloqueado(t_pcb* self) {
    time(&self->tiempoFinalBloqueado);
}

static bool __pcb_tiempo_final_ya_establecido(t_pcb* self) {
    return self->tiempoFinalBloqueadoEstablecido;
}

t_pcb* pcb_create(uint32_t pid, uint32_t tamanio, double estimacionInicial) {
    t_pcb* self = malloc(sizeof(*self));
    self->pid = pid;
    self->tamanio = tamanio;
    self->programCounter = 0;
    self->tablaPaginaPrimerNivel = 0;
    self->ultimaEstimacion = estimacionInicial;
    self->ultimaEjecucion = -1;
    self->estadoActual = NEW;
    self->tiempoDeBloqueo = 0;
    self->socketConsola = -1;
    self->instructionsBuffer = NULL;
    self->mutex = malloc(sizeof(*(self->mutex)));
    pthread_mutex_init(self->mutex, NULL);
    self->tiempoFinalBloqueadoEstablecido = false;
    self->vecesBloqueado = 0;
    return self;
}

void pcb_destroy(t_pcb* self) {
    if (self->instructionsBuffer != NULL) {
        buffer_destroy(self->instructionsBuffer);
    }
    pthread_mutex_destroy(self->mutex);
    free(self->mutex);
    free(self);
}

void pcb_responder_a_consola(t_pcb* self, uint8_t responseHeader) {
    stream_send_empty_buffer(self->socketConsola, responseHeader);
}

double pcb_estimar_srt(t_pcb* self, int alfa) {
    if (self->ultimaEjecucion == -1) {
        return self->ultimaEstimacion;  // estimación inicial
    }
    self->ultimaEstimacion = alfa * self->ultimaEjecucion + (1 - alfa) * self->ultimaEstimacion;
    return self->ultimaEstimacion;
}

void pcb_test_and_set_tiempo_final_bloqueado(t_pcb* self) {
    pthread_mutex_lock(pcb_get_mutex(self));
    if (!__pcb_tiempo_final_ya_establecido(self)) {
        __pcb_marcar_tiempo_final_bloqueado(self);
        __pcb_marcar_tiempo_final_como_establecido(self);
    }
    pthread_mutex_unlock(pcb_get_mutex(self));
}

uint32_t pcb_get_pid(t_pcb* self) {
    return self->pid;
}

uint32_t pcb_get_tamanio(t_pcb* self) {
    return self->tamanio;
}

uint64_t pcb_get_program_counter(t_pcb* self) {
    return self->programCounter;
}

void pcb_set_program_counter(t_pcb* self, uint64_t programCounter) {
    self->programCounter = programCounter;
}

uint32_t pcb_get_tabla_pagina_primer_nivel(t_pcb* self) {
    return self->tablaPaginaPrimerNivel;
}

void pcb_set_tabla_pagina_primer_nivel(t_pcb* self, uint32_t tablaPaginaPrimerNivel) {
    self->tablaPaginaPrimerNivel = tablaPaginaPrimerNivel;
}

double pcb_get_estimacion_actual(t_pcb* self) {
    return self->ultimaEstimacion;
}

void pcb_set_ultima_ejecucion(t_pcb* self, double ultimaEjecucion) {
    self->ultimaEjecucion = ultimaEjecucion;
}

uint8_t pcb_get_estado_actual(t_pcb* self) {
    return self->estadoActual;
}

void pcb_set_estado_actual(t_pcb* self, uint8_t estadoActual) {
    self->estadoActual = estadoActual;
}

uint32_t pcb_get_tiempo_de_bloqueo(t_pcb* self) {
    return self->tiempoDeBloqueo;
}

void pcb_set_tiempo_de_bloqueo(t_pcb* self, uint32_t tiempoDeBloqueo) {
    self->tiempoDeBloqueo = tiempoDeBloqueo;
}

void pcb_set_socket(t_pcb* self, int socket) {
    self->socketConsola = socket;
}

t_buffer* pcb_get_instruction_buffer(t_pcb* self) {
    return self->instructionsBuffer;
}

void pcb_set_instruction_buffer(t_pcb* self, t_buffer* instructionsBuffer) {
    self->instructionsBuffer = instructionsBuffer;
}

pthread_mutex_t* pcb_get_mutex(t_pcb* self) {
    return self->mutex;
}

time_t pcb_get_tiempo_inicial_bloqueado(t_pcb* self) {
    return self->tiempoInicialBloqueado;
}

time_t pcb_get_tiempo_final_bloqueado(t_pcb* self) {
    return self->tiempoFinalBloqueado;
}

void pcb_set_veces_bloqueado(t_pcb* self, int vecesBloqueado) {
    self->vecesBloqueado = vecesBloqueado;
}

int pcb_get_veces_bloqueado(t_pcb* self) {
    return self->vecesBloqueado;
}

void pcb_marcar_tiempo_inicial_bloqueado(t_pcb* self) {
    time(&self->tiempoInicialBloqueado);
}

void pcb_marcar_tiempo_final_como_no_establecido(t_pcb* self) {
    self->tiempoFinalBloqueadoEstablecido = false;
}
