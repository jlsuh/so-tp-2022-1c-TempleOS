#ifndef ESTADOS_H_INCLUDED
#define ESTADOS_H_INCLUDED

#include <commons/collections/list.h>
#include <semaphore.h>

#include "nombre_estado.h"
#include "pcb.h"

typedef struct t_estado t_estado;

bool estado_contiene_pcb_atomic(t_estado *, t_pcb *);
pthread_mutex_t *estado_get_mutex(t_estado *);
sem_t *estado_get_sem(t_estado *);
t_estado *estado_create(t_nombre_estado);
t_list *estado_get_list(t_estado *);
t_pcb *estado_desencolar_primer_pcb(t_estado *);
t_pcb *estado_desencolar_primer_pcb_atomic(t_estado *);
t_pcb *estado_remover_pcb_de_cola(t_estado *, t_pcb *);
t_pcb *estado_remover_pcb_de_cola_atomic(t_estado *, t_pcb *);
void estado_destroy(t_estado *);
void estado_encolar_pcb_atomic(t_estado *, t_pcb *);

#endif
