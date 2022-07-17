#include "pcb.h"

#include <commons/collections/list.h>
#include <stdbool.h>
#include <stdint.h>
#include <unistd.h>

#include "estado.h"
#include "instruccion.h"
#include "stream.h"

#define MILISECS_IN_SEC 1000.0

struct t_pcb {
    uint32_t pid;
    uint32_t tamanio;
    uint32_t programCounter;
    uint32_t tablaPaginaPrimerNivel;
    double estimacionActual;
    double realAnterior;
    double realesEjecutadosHastaAhora;
    uint8_t estadoActual;
    uint32_t tiempoDeBloqueo;
    int* socketConsola;
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
    self->estimacionActual = estimacionInicial;
    self->realAnterior = 0.0;
    self->realesEjecutadosHastaAhora = 0.0;
    self->estadoActual = NEW;
    self->tiempoDeBloqueo = 0;
    self->socketConsola = NULL;
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
    if (self->socketConsola != NULL) {
        close(*self->socketConsola);
        free(self->socketConsola);
    }
    pthread_mutex_destroy(self->mutex);
    free(self->mutex);
    free(self);
}

void pcb_test_and_set_tiempo_final_bloqueado(t_pcb* self) {
    pthread_mutex_lock(pcb_get_mutex(self));
    if (!__pcb_tiempo_final_ya_establecido(self)) {
        __pcb_marcar_tiempo_final_bloqueado(self);
        __pcb_marcar_tiempo_final_como_establecido(self);
    }
    pthread_mutex_unlock(pcb_get_mutex(self));
}

bool pcb_es_este_pcb_por_pid(void* unPcb, void* otroPcb) {
    return pcb_get_pid((t_pcb*)unPcb) == pcb_get_pid((t_pcb*)otroPcb);
}

uint32_t pcb_get_pid(t_pcb* self) {
    return self->pid;
}

uint32_t pcb_get_tamanio(t_pcb* self) {
    return self->tamanio;
}

uint32_t pcb_get_program_counter(t_pcb* self) {
    return self->programCounter;
}

void pcb_set_program_counter(t_pcb* self, uint32_t programCounter) {
    self->programCounter = programCounter;
}

uint32_t pcb_get_tabla_pagina_primer_nivel(t_pcb* self) {
    return self->tablaPaginaPrimerNivel;
}

void pcb_set_tabla_pagina_primer_nivel(t_pcb* self, uint32_t tablaPaginaPrimerNivel) {
    self->tablaPaginaPrimerNivel = tablaPaginaPrimerNivel;
}

double pcb_get_estimacion_actual(t_pcb* self) {
    return self->estimacionActual;
}

void pcb_set_estimacion_actual(t_pcb* self, double estimacionActual) {
    self->estimacionActual = estimacionActual;
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

double pcb_get_tiempo_de_bloqueo_en_secs(t_pcb* self) {
    return self->tiempoDeBloqueo / MILISECS_IN_SEC;
}

void pcb_set_tiempo_de_bloqueo(t_pcb* self, uint32_t tiempoDeBloqueo) {
    self->tiempoDeBloqueo = tiempoDeBloqueo;
}

void pcb_set_socket(t_pcb* self, int* socketConsola) {
    self->socketConsola = socketConsola;
}

int pcb_get_socket(t_pcb* self) {
    return *self->socketConsola;
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

void pcb_set_real_anterior(t_pcb* self, double realAnterior) {
    self->realAnterior = realAnterior;
}

double pcb_get_real_anterior(t_pcb* self) {
    return self->realAnterior;
}

void pcb_set_reales_ejecutados_hasta_ahora(t_pcb* self, double realesEjecutadosHastaAhora) {
    self->realesEjecutadosHastaAhora = realesEjecutadosHastaAhora;
}

double pcb_get_reales_ejecutados_hasta_ahora(t_pcb* self) {
    return self->realesEjecutadosHastaAhora;
}
