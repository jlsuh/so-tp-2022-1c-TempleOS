#ifndef KERNEL_CONFIG_H_INCLUDED
#define KERNEL_CONFIG_H_INCLUDED

#include <commons/log.h>

typedef struct t_kernel_config t_kernel_config;

t_kernel_config* kernel_config_create(char* kernelConfigPath, t_log*);
char* kernel_config_get_ip_escucha(t_kernel_config*);
char* kernel_config_get_puerto_escucha(t_kernel_config*);
char* kernel_config_get_ip_cpu(t_kernel_config*);
char* kernel_config_get_puerto_cpu_dispatch(t_kernel_config*);
char* kernel_config_get_puerto_cpu_interrupt(t_kernel_config*);
char* kernel_config_get_puerto_memoria(t_kernel_config*);
char* kernel_config_get_ip_memoria(t_kernel_config*);
void kernel_config_set_socket_dispatch_cpu(t_kernel_config*, int socket);
void kernel_config_set_socket_interrupt_cpu(t_kernel_config*, int socket);
void kernel_config_set_socket_memoria(t_kernel_config*, int socketMemoria);
void kernel_config_destroy(t_kernel_config*);

#endif
