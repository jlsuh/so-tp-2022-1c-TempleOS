#ifndef KERNEL_CONFIG_H_INCLUDED
#define KERNEL_CONFIG_H_INCLUDED

#include <commons/log.h>

typedef struct t_kernel_config t_kernel_config;

bool kernel_config_es_algoritmo_fifo(t_kernel_config *);
bool kernel_config_es_algoritmo_sjf(t_kernel_config *);
char *kernel_config_get_algoritmo(t_kernel_config *);
char *kernel_config_get_ip_cpu(t_kernel_config *);
char *kernel_config_get_ip_escucha(t_kernel_config *);
char *kernel_config_get_ip_memoria(t_kernel_config *);
char *kernel_config_get_puerto_cpu_dispatch(t_kernel_config *);
char *kernel_config_get_puerto_cpu_interrupt(t_kernel_config *);
char *kernel_config_get_puerto_escucha(t_kernel_config *);
char *kernel_config_get_puerto_memoria(t_kernel_config *);
double kernel_config_get_est_inicial(t_kernel_config *);
int kernel_config_get_alfa(t_kernel_config *);
int kernel_config_get_grado_multiprogramacion(t_kernel_config *);
int kernel_config_get_maximo_bloq(t_kernel_config *);
int kernel_config_get_socket_dispatch_cpu(t_kernel_config *);
int kernel_config_get_socket_interrupt_cpu(t_kernel_config *);
int kernel_config_get_socket_memoria(t_kernel_config *);
t_kernel_config *kernel_config_create(char *kernelConfigPath, t_log *kernelLogger);
void kernel_config_destroy(t_kernel_config *);
void kernel_config_set_socket_dispatch_cpu(t_kernel_config *, int socket);
void kernel_config_set_socket_interrupt_cpu(t_kernel_config *, int socket);
void kernel_config_set_socket_memoria(t_kernel_config *, int socketMemoria);

#endif
