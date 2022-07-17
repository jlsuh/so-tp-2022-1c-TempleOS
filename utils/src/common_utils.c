#include "common_utils.h"

#include <time.h>

int list_get_index(t_list* list, bool (*cutting_condition)(void*, void*), void* target) {
    for (int i = 0; i < list_size(list); i++) {
        void* temp = list_get(list, i);
        if (cutting_condition(temp, target)) {
            return i;
        }
    }
    return -1;
}

void intervalo_de_pausa(uint32_t duracionEnMilisegundos) {
    const uint32_t SECS_MILISECS = 1000;
    const uint32_t MILISECS_NANOSECS = 1000000;
    struct timespec timeSpec;
    timeSpec.tv_sec = duracionEnMilisegundos / SECS_MILISECS;
    timeSpec.tv_nsec = (duracionEnMilisegundos % SECS_MILISECS) * MILISECS_NANOSECS;
    nanosleep(&timeSpec, &timeSpec);
}
