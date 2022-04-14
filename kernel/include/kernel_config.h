#ifndef KERNEL_CONFIG_H_INCLUDED
#define KERNEL_CONFIG_H_INCLUDED

#include <commons/log.h>

typedef struct t_kernel_config t_kernel_config;

t_kernel_config* kernel_config_create(char* kernelConfigPath, t_log*);

#endif
