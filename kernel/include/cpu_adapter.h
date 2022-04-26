#ifndef CPU_ADAPTER_H_INCLUDED
#define CPU_ADAPTER_H_INCLUDED

#include <commons/log.h>
#include <stdint.h>

#include "kernel_config.h"
#include "pcb.h"

void cpu_adapter_enviar_pcb_a_cpu(t_pcb* pcbAEnviar, t_kernel_config*, t_log*); 
t_pcb* cpu_adapter_recibir_pcb_de_cpu(t_pcb* pcbARecibir, t_kernel_config* kernelConfig, t_log* kernelLogger);
void cpu_adapter_interrumpir_cpu(int);

#endif
