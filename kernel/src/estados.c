#include "estados.h"

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

static bool es_este_pcb_por_id(void* pcbDeLaLista, void* targetPcb) {
    return (pcb_get_pid((t_pcb*)pcbDeLaLista)) == pcb_get_pid(targetPcb);
}

t_pcb* estado_remover_pcb_de_cola(t_estado* estadoTarget, t_pcb* targetPcb) {
    pthread_mutex_lock(estado_get_mutex(estadoTarget));
    t_pcb* pcb = NULL;
    uint32_t index = list_get_index(estado_get_list(estadoTarget), es_este_pcb_por_id, targetPcb);
    if (index != -1) {
        pcb = list_remove(estado_get_list(estadoTarget), index);
    }
    pthread_mutex_unlock(estado_get_mutex(estadoTarget));
    return pcb;
}

t_pcb* estado_desencolar_primer_pcb(t_estado* estadoTarget) {
    pthread_mutex_lock(estado_get_mutex(estadoTarget));
    t_pcb* pcb = list_remove(estado_get_list(estadoTarget), 0);
    pthread_mutex_unlock(estado_get_mutex(estadoTarget));
    return pcb;
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

void estado_encolar_pcb(t_estado* estadoDest, t_pcb* targetPcb) {
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
