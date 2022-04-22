#ifndef SCHEDULER_H_INCLUDED
#define SCHEDULER_H_INCLUDED

#include <pcb.h>
#include <stdint.h>
#include <string.h>

void* encolar_en_new_a_nuevo_pcb_entrante(void* socket);
void inicializar_estructuras(void);
void interrumpir_cpu(void);

#endif
