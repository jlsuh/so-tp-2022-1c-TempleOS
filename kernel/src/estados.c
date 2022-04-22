#include "estados.h"

#include <pthread.h>
#include <semaphore.h>

#include "common_flags.h"
#include "pcb.h"

struct t_estado {
    t_nombre_estado nombreEstado;
    t_list* listaProcesos;
    sem_t* semaforoEstado;
    pthread_mutex_t* mutexEstado;
};

 void cambiar_estado(t_estado* estadoDest, t_pcb* pcb) {
   /* switch (pcb_get_estado_actual(pcb)) {
        case NEW:
            remover_de_lista_de_estado(estadoNew, pcb);
        case READY:
            remover_de_lista_de_estado(estadoReady, pcb);
        case EXEC:
            remover_de_lista_de_estado(estadoExec, pcb);
        case EXIT:
            remover_de_lista_de_estado(estadoExit, pcb);
        case BLOCKED:
            remover_de_lista_de_estado(estadoBlocked, pcb);
        case SUSPENDED_BLOCKED:
            remover_de_lista_de_estado(estadoSuspendedBlocked, pcb);
        case SUSPENDED_READY:
            remover_de_lista_de_estado(estadoSuspendedReady, pcb);
        default:
            break;
    }*/ // TODO: terminar de ver esto, los structs estados estan en el scheduler.c
    estado_encolar_pcb(estadoDest,pcb);
    return;
}

void remover_de_lista_de_estado(t_estado* estadoObjetivo, t_pcb* pcb) {
    bool buscar_proceso_en_lista_de_estado(void* proceso_en_lista) {
        return (pcb_get_pid((t_pcb*)proceso_en_lista)) == pcb_get_pid(pcb);
    }

    pthread_mutex_lock(estado_get_mutex(estadoObjetivo));
    pcb = list_remove_by_condition(estado_get_list(estadoObjetivo), buscar_proceso_en_lista_de_estado); // TODO: ver si esto está bien, esto devolvería el mismo pcb pero sacado de la lista? 
    sem_wait(estado_get_sem(estadoObjetivo));
    pthread_mutex_unlock(estado_get_mutex(estadoObjetivo));
    return;
}

t_estado* estado_create(t_nombre_estado nombre) {
    t_estado* self = malloc(sizeof(*self));
    self->listaProcesos = list_create();
    self->mutexEstado = malloc(sizeof(*(self->mutexEstado)));
    pthread_mutex_init(self->mutexEstado, NULL);
    self->nombreEstado = nombre;
    self->semaforoEstado = malloc(sizeof(*(self->semaforoEstado)));
    sem_init(self->semaforoEstado, 0, 0);
    return self;
}

void estado_encolar_pcb(t_estado* estadoDest, t_pcb* pcb) {
    pthread_mutex_lock(estado_get_mutex(estadoDest));
    list_add(estado_get_list(estadoDest), pcb);
    pcb_set_estado_actual(pcb, estado_get_nombre_estado(estadoDest));
    pthread_mutex_unlock(estado_get_mutex(estadoDest));
}

t_nombre_estado estado_get_nombre_estado(t_estado* self) {
    return self->nombreEstado;
}

t_list* estado_get_list(t_estado* self) {
    return self->listaProcesos;
}

sem_t* estado_get_sem(t_estado* self) {
    return self->semaforoEstado;
}

pthread_mutex_t* estado_get_mutex(t_estado* self) {
    return self->mutexEstado;
}
