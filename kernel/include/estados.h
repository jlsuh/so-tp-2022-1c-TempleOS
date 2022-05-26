#ifndef ESTADOS_H_INCLUDED
#define ESTADOS_H_INCLUDED

#include <commons/collections/list.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "pcb.h"

typedef struct t_estado t_estado;
typedef enum {
    NEW,
    READY,
    EXEC,
    EXIT,
    BLOCKED,
    SUSPENDED_READY,
    SUSPENDED_BLOCKED,
    PCBS_ESPERANDO_PARA_IO,
} t_nombre_estado;

void cambiar_estado(t_estado *estadoDest, t_pcb *pcb);
t_pcb *estado_remover_pcb_de_cola(t_estado *estadoTarget, t_pcb *targetPcb);
pthread_mutex_t *estado_get_mutex(t_estado *);
sem_t *estado_get_sem(t_estado *);
t_estado *estado_create(t_nombre_estado nombre);
t_list *estado_get_list(t_estado *);
t_nombre_estado estado_get_nombre_estado(t_estado *);
void estado_encolar_pcb(t_estado *estadoDest, t_pcb *);
t_pcb *estado_desencolar_primer_pcb(t_estado *);
bool estado_contiene_pcb(t_estado *, t_pcb *);

#endif
