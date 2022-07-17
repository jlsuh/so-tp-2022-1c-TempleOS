#ifndef CPU_ADAPTER_H_INCLUDED
#define CPU_ADAPTER_H_INCLUDED

#include "kernel_config.h"
#include "pcb.h"

t_pcb *cpu_adapter_recibir_pcb_actualizado_de_cpu(t_pcb *, uint8_t cpuResponse, t_kernel_config *, t_log *);
void cpu_adapter_enviar_pcb_a_cpu(t_pcb *, t_kernel_config *, t_log *, uint8_t header);
void cpu_adapter_interrumpir_cpu(t_kernel_config *, t_log *);

#endif
