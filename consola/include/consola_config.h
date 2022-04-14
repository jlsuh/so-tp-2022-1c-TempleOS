#ifndef CONSOLA_CONFIG_H_INCLUDED
#define CONSOLA_CONFIG_H_INCLUDED

#include <commons/config.h>
#include <commons/log.h>

typedef struct t_consola_config t_consola_config;

t_consola_config* consola_config_create(char* consolaConfigPath, t_log*);
void consola_config_destroy(t_consola_config*);
char* consola_config_get_kernel_ip(t_consola_config*);
char* consola_config_get_kernel_port(t_consola_config*);
void consola_config_set_kernel_socket(t_consola_config*, int socket);

#endif
