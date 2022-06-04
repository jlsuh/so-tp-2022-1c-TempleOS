#ifndef PCB_H_INCLUDED
#define PCB_H_INCLUDED

#include <commons/collections/list.h>
#include <pthread.h>
#include <stdint.h>

#include "stream.h"

typedef struct t_instruccion t_instruccion;
typedef struct t_pcb t_pcb;

bool pcb_es_este_pcb_por_pid(void* unPcb, void* otroPcb);
double pcb_get_estimacion_actual(t_pcb*);
double pcb_get_real_anterior(t_pcb*);
double pcb_get_reales_ejecutados_hasta_ahora(t_pcb*);
int pcb_get_veces_bloqueado(t_pcb*);
pthread_mutex_t* pcb_get_mutex(t_pcb*);
t_buffer* pcb_get_instruction_buffer(t_pcb*);
t_pcb* pcb_create(uint32_t pid, uint32_t tamanio, double estimacionInicial);
time_t pcb_get_tiempo_final_bloqueado(t_pcb*);
time_t pcb_get_tiempo_inicial_bloqueado(t_pcb*);
uint32_t pcb_get_pid(t_pcb*);
uint32_t pcb_get_tabla_pagina_primer_nivel(t_pcb*);
uint32_t pcb_get_tamanio(t_pcb*);
uint32_t pcb_get_tiempo_de_bloqueo(t_pcb*);
uint32_t pcb_get_program_counter(t_pcb*);
uint8_t pcb_get_estado_actual(t_pcb*);
void pcb_destroy(t_pcb*);
void pcb_marcar_tiempo_final_como_no_establecido(t_pcb*);
void pcb_marcar_tiempo_inicial_bloqueado(t_pcb*);
void pcb_responder_a_consola(t_pcb*, uint8_t responseHeader);
void pcb_set_estado_actual(t_pcb*, uint8_t estadoActual);
void pcb_set_estimacion_actual(t_pcb*, double estimacionActual);
void pcb_set_instruction_buffer(t_pcb*, t_buffer* instructionsBuffer);
void pcb_set_program_counter(t_pcb*, uint32_t programCounter);
void pcb_set_real_anterior(t_pcb*, double realAnterior);
void pcb_set_reales_ejecutados_hasta_ahora(t_pcb*, double realesEjecutadosHastaAhora);
void pcb_set_socket(t_pcb*, int* socket);
void pcb_set_tabla_pagina_primer_nivel(t_pcb*, uint32_t tablaPaginaPrimerNivel);
void pcb_set_tiempo_de_bloqueo(t_pcb*, uint32_t tiempoDeBloqueo);
void pcb_set_veces_bloqueado(t_pcb*, int vecesBloqueado);
void pcb_test_and_set_tiempo_final_bloqueado(t_pcb*);

#endif
