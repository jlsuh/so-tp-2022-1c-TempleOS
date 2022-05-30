#ifndef SCHEDULER_H_INCLUDED
#define SCHEDULER_H_INCLUDED

#include <pcb.h>
#include <stdint.h>
#include <string.h>

#include "estado.h"

typedef t_pcb* (*t_algoritmo)(t_estado*, double);

double calcular_estimacion_restante(t_pcb*);
double calcular_siguiente_estimacion(t_pcb*, double alfa);
double media_exponencial(double alfa, double realAnterior, double estimacionAnterior);
t_pcb* menor_rafaga_restante(t_pcb*, t_pcb*);
t_pcb* segun_fifo(t_estado*, double _);
t_pcb* segun_srt(t_estado*, double alfa);
void* encolar_en_new_a_nuevo_pcb_entrante(void* socket);
void actualizar_pcb_por_bloqueo(t_pcb*, double realEjecutado, double alfa);
void actualizar_pcb_por_desalojo(t_pcb*, double realEjecutado);
void inicializar_estructuras(void);

#endif
