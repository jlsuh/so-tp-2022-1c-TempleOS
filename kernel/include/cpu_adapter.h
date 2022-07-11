#ifndef CPU_ADAPTER_H_INCLUDED
#define CPU_ADAPTER_H_INCLUDED

#include <commons/log.h>
#include <stdint.h>

#include "kernel_config.h"
#include "pcb.h"

void cpu_adapter_enviar_pcb_a_cpu(t_pcb* pcbAEnviar, t_kernel_config*, t_log*, uint8_t header);
t_pcb* cpu_adapter_recibir_pcb_actualizado_de_cpu(t_pcb* pcbARecibir, uint8_t cpuResponse, t_kernel_config*, t_log*);
void cpu_adapter_interrumpir_cpu(t_kernel_config*, t_log*);

#endif
