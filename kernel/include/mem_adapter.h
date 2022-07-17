#ifndef MEM_ADAPTER_H_INCLUDED
#define MEM_ADAPTER_H_INCLUDED

#include "kernel_config.h"
#include "pcb.h"

int mem_adapter_obtener_tabla_pagina(t_pcb *, t_kernel_config *, t_log *);
void mem_adapter_avisar_suspension(t_pcb *, t_kernel_config *, t_log *);
void mem_adapter_finalizar_proceso(t_pcb *, t_kernel_config *, t_log *);

#endif
