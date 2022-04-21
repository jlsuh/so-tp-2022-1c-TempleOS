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
} t_nombre_estado;

pthread_mutex_t *estado_get_mutex(t_estado *);
sem_t *estado_get_sem(t_estado *);
t_estado *estado_create(t_nombre_estado nombre);
t_list *estado_get_list(t_estado *);
t_nombre_estado estado_get_nombre_estado(t_estado *);
void estado_encolar_pcb(t_estado *estadoDest, t_pcb *);

#endif
