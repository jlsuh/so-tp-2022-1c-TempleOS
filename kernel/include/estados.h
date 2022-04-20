#ifndef ESTADOS_H_INCLUDED
#define ESTADOS_H_INCLUDED

#include <commons/collections/list.h>
#include <stdint.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

typedef struct t_estado{
	t_nombre_estado nombreEstado;
    t_list* listaProcesos;
    sem_t semaforoEstado;
    pthread_mutex_t mutexEstado;
}t_estado;

struct t_estado nuevo; // TODO: al parecer new es palabra reservada
struct t_estado ready;
struct t_estado exec;
struct t_estado salida; // TODO: exit también, pasar a español si al final hacemos así los estados
struct t_estado blocked;
struct t_estado suspendedBlocked;
struct t_estado suspendedReady;

void cambiar_estado(t_estado estadoNuevo, t_pcb* proceso);
void remover_de_lista_de_estado(t_estado estadoObjetivo, uint32_t idProceso);

#endif
