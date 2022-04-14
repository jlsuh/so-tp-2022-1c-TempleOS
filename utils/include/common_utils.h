#ifndef COMMON_UTILS_H_INCLUDED
#define COMMON_UTILS_H_INCLUDED

#include <commons/collections/list.h>
#include <stdint.h>

int list_get_index(t_list *self, bool (*cutting_condition)(void *, void *), void *target);
void *list_find2(t_list *self, bool (*condition)(void *, void *), void *data);
void intervalo_de_pausa(uint32_t duracionEnMilisegundos);

#endif
