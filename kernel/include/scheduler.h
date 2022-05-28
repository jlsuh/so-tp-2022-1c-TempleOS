#ifndef SCHEDULER_H_INCLUDED
#define SCHEDULER_H_INCLUDED

#include <pcb.h>
#include <stdint.h>
#include <string.h>

#include "estado.h"

bool pcb_es_este_pcb_por_pid(void* unPcb, void* otroPcb);
double calcular_siguiente_estimacion(t_pcb* pcb, double alfa);
double media_exponencial(double alfa, double realAnterior, double estimacionAnterior);
t_pcb* elegir_pcb_segun_fifo(t_estado* estado, double _);
t_pcb* elegir_pcb_segun_srt(t_estado* estado, double alfa);
t_pcb* menor_rafaga_restante(t_pcb* unPcb, t_pcb* otroPcb);
void* encolar_en_new_a_nuevo_pcb_entrante(void* socket);
void inicializar_estructuras(void);

#endif
