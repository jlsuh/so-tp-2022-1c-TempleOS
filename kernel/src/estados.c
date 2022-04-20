#include "estados.h"

#include "common_flags.h"
#include "pcb.h"

void cambiar_estado(t_estado estadoNuevo, t_pcb* proceso) {
    switch (proceso->estadoActual){
        case NEW:
        	remover_de_lista_de_estado(nuevo, proceso->pid);
        case READY:
        	remover_de_lista_de_estado(ready, proceso->pid);
        case EXEC:
        	remover_de_lista_de_estado(exec, proceso->pid);
        case EXIT:
        	remover_de_lista_de_estado(salida, proceso->pid);
        case BLOCKED:
        	remover_de_lista_de_estado(blocked, proceso->pid);
        case SUSPENDED_BLOCKED:
        	remover_de_lista_de_estado(suspendedBlocked, proceso->pid);
        case SUSPENDED_READY:
        	remover_de_lista_de_estado(suspendedReady, proceso->pid);
        default:
            break;
    }
    pthread_mutex_lock(&estadoNuevo.mutexEstado);
    list_add(estadoNuevo.listaProcesos,proceso);
    proceso->estadoActual = estadoNuevo.nombreEstado;
    sem_post(&estadoNuevo.semaforoEstado);
    pthread_mutex_unlock(&estadoNuevo.mutexEstado);
    return;
}

void remover_de_lista_de_estado(t_estado estadoObjetivo, uint32_t idProceso){
    bool buscar_proceso_en_lista_de_estado(void* proceso_en_lista) {
	    	return ((t_pcb*)proceso_en_lista)->pid == idProceso;
    }

    pthread_mutex_lock(&estadoObjetivo.mutexEstado);
    list_remove_by_condition(estadoObjetivo.listaProcesos,buscar_proceso_en_lista_de_estado);
    pthread_mutex_unlock(&estadoObjetivo.mutexEstado);
    sem_wait(&estadoObjetivo.semaforoEstado);
    return;
}
