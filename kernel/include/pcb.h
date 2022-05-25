#ifndef PCB_H_INCLUDED
#define PCB_H_INCLUDED

#include <commons/collections/list.h>
#include <pthread.h>
#include <stdint.h>

#include "stream.h"

typedef struct t_instruccion t_instruccion;
typedef struct t_pcb t_pcb;

uint32_t pcb_get_pid(t_pcb*);
uint32_t pcb_get_tamanio(t_pcb*);
uint64_t pcb_get_program_counter(t_pcb*);
void pcb_set_program_counter(t_pcb* self, uint64_t pc);
uint8_t pcb_get_estado_actual(t_pcb*);
void pcb_set_estado_actual(t_pcb*, uint8_t estado);
double pcb_get_est_actual(t_pcb*);
void pcb_set_est_actual(t_pcb*, double est);
double pcb_get_ultima_ejecucion(t_pcb*);
void pcb_set_ultima_ejecucion(t_pcb*, double);
uint32_t pcb_get_tabla_pagina_primer_nivel(t_pcb*);
void pcb_set_tabla_pagina_primer_nivel(t_pcb* self, uint32_t);
uint32_t pcb_get_tiempo_de_bloqueo(t_pcb* self);
void pcb_set_tiempo_de_bloqueo(t_pcb* self, uint32_t);
int pcb_get_socket(t_pcb* self);
void pcb_set_socket(t_pcb* self, int);
t_buffer* pcb_get_instruction_buffer(t_pcb* self);
void pcb_set_instruction_buffer(t_pcb* self, t_buffer*);
t_pcb* pcb_create(uint32_t pid, uint32_t tamanio, double estimacionRafaga);
void pcb_destroy(t_pcb*);
void pcb_responder_a_consola(t_pcb* self, uint8_t rta);
double pcb_estimar_srt(t_pcb*, int);
pthread_mutex_t* pcb_get_mutex(t_pcb*);
void pcb_set_tiempo_inicial_bloqueado(t_pcb*, time_t tiempoInicialBloqueado);
time_t pcb_get_tiempo_inicial_bloqueado(t_pcb*);
void pcb_set_tiempo_final_bloqueado(t_pcb*, time_t tiempoFinalBloqueado);
time_t pcb_get_tiempo_final_bloqueado(t_pcb*);
bool pcb_get_tiempo_final_bloqueado_setteado(t_pcb*);
void pcb_set_tiempo_final_bloqueado_setteado(t_pcb*, bool tiempoFinalBloqueadoSetteado);
void pcb_set_veces_bloqueado(t_pcb*, int vecesBloqueado);
int pcb_get_veces_bloqueado(t_pcb*);

#endif
