#ifndef MEMORIA_CONFIG_H_INCLUDED
#define MEMORIA_CONFIG_H_INCLUDED

#include <commons/log.h>

typedef struct t_memoria_config t_memoria_config;

t_memoria_config* memoria_config_create(char* memoriaConfigPath, t_log*);
char* memoria_config_get_ip_escucha(t_memoria_config*);
char* memoria_config_get_puerto_escucha(t_memoria_config*);
void memoria_config_destroy(t_memoria_config*);
int memoria_config_set_kernel_socket(t_memoria_config*, int socket);
int memoria_config_set_cpu_socket(t_memoria_config*, int socket);
int memoria_config_get_kernel_socket(t_memoria_config*);
int memoria_config_get_cpu_socket(t_memoria_config*);

#endif
