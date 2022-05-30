#include "estado.h"

#include <pthread.h>
#include <semaphore.h>

#include "common_flags.h"
#include "common_utils.h"
#include "pcb.h"

struct t_estado {
    t_nombre_estado nombreEstado;
    t_list* listaProcesos;
    sem_t* semaforoEstado;
    pthread_mutex_t* mutexEstado;
};

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

void estado_destroy(t_estado* self) {
    if (list_is_empty(self->listaProcesos)) {
        list_destroy(self->listaProcesos);
    } else {
        list_destroy_and_destroy_elements(self->listaProcesos, (void*)pcb_destroy);
    }
    pthread_mutex_destroy(self->mutexEstado);
    sem_destroy(self->semaforoEstado);
    free(self->semaforoEstado);
    free(self->mutexEstado);
    free(self);
}

t_pcb* estado_remover_pcb_de_cola(t_estado* self, t_pcb* targetPcb) {
    t_pcb* pcb = NULL;
    uint32_t index = list_get_index(estado_get_list(self), pcb_es_este_pcb_por_pid, targetPcb);
    if (index != -1) {
        pcb = list_remove(estado_get_list(self), index);
    }
    return pcb;
}

t_pcb* estado_remover_pcb_de_cola_atomic(t_estado* self, t_pcb* targetPcb) {
    pthread_mutex_lock(estado_get_mutex(self));
    t_pcb* pcb = estado_remover_pcb_de_cola(self, targetPcb);
    pthread_mutex_unlock(estado_get_mutex(self));
    return pcb;
}

bool estado_contiene_pcb_atomic(t_estado* self, t_pcb* targetPcb) {
    pthread_mutex_lock(estado_get_mutex(self));
    bool contains = false;
    uint32_t index = list_get_index(estado_get_list(self), pcb_es_este_pcb_por_pid, targetPcb);
    if (index != -1) {
        contains = true;
    }
    pthread_mutex_unlock(estado_get_mutex(self));
    return contains;
}

t_pcb* estado_desencolar_primer_pcb(t_estado* self) {
    return list_remove(estado_get_list(self), 0);
}

t_pcb* estado_desencolar_primer_pcb_atomic(t_estado* self) {
    pthread_mutex_lock(estado_get_mutex(self));
    t_pcb* pcb = estado_desencolar_primer_pcb(self);
    pthread_mutex_unlock(estado_get_mutex(self));
    return pcb;
}

void estado_encolar_pcb_atomic(t_estado* estadoDest, t_pcb* targetPcb) {
    pthread_mutex_lock(estado_get_mutex(estadoDest));
    list_add(estado_get_list(estadoDest), targetPcb);
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
