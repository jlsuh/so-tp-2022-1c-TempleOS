#ifndef SCHEDULER_H_INCLUDED
#define SCHEDULER_H_INCLUDED

#include <stdint.h>

uint32_t get_next_pid(void);
void* encolar_en_new_a_nuevo_pcb_entrante(void* socket);
void inicializar_estructuras(void);

#endif
