#ifndef MEM_ADAPTER_H_INCLUDED
#define MEM_ADAPTER_H_INCLUDED

#include <commons/log.h>
#include <stdint.h>

#include "kernel_config.h"
#include "pcb.h"

uint32_t mem_adapter_obtener_tabla_pagina(t_pcb* pcbAIniciar, t_kernel_config*, t_log*);
void mem_adapter_finalizar_proceso(t_pcb* pcbAFinalizar, t_kernel_config* kernelConfig, t_log* kernelLogger);

#endif
