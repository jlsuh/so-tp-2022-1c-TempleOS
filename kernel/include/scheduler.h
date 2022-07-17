#ifndef SCHEDULER_H_INCLUDED
#define SCHEDULER_H_INCLUDED

#include <pcb.h>
#include <stdint.h>
#include <string.h>

#include "estado.h"

typedef t_pcb* (*t_dispatch_handler)(t_estado*, double);
typedef void (*t_onBlocked_handler)(t_pcb*, uint32_t, double);

double calcular_estimacion_restante(t_pcb*);
t_pcb* elegir_pcb_segun_fifo(t_estado*, double _);
t_pcb* elegir_pcb_segun_srt(t_estado*, double alfa);
void* encolar_en_new_a_nuevo_pcb_entrante(void* socket);
void actualizar_pcb_por_bloqueo_segun_srt(t_pcb*, uint32_t realEjecutado, double alfa);
void actualizar_pcb_por_desalojo(t_pcb*, double realEjecutado);
void inicializar_estructuras(void);

#endif
