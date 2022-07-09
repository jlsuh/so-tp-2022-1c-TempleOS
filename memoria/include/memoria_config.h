#ifndef MEMORIA_CONFIG_H_INCLUDED
#define MEMORIA_CONFIG_H_INCLUDED

#include <commons/log.h>

typedef struct t_memoria_config t_memoria_config;

char* memoria_config_get_ip_escucha(t_memoria_config* self);
char* memoria_config_get_puerto_escucha(t_memoria_config* self);
int memoria_config_get_cpu_socket(t_memoria_config* self);
int memoria_config_get_entradas_por_tabla(t_memoria_config* self);
int memoria_config_get_kernel_socket(t_memoria_config* self);
int memoria_config_get_marcos_por_proceso(t_memoria_config* self);
int memoria_config_get_tamanio_memoria(t_memoria_config* self);
int memoria_config_get_tamanio_pagina(t_memoria_config* self);
int memoria_config_set_cpu_socket(t_memoria_config* self, int socket);
int memoria_config_set_kernel_socket(t_memoria_config* self, int socket);
char* memoria_config_get_path_swap(t_memoria_config* self);
t_memoria_config* memoria_config_create(char* memoriaConfigPath, t_log* memoriaLogger);
int memoria_config_get_retardo_swap(t_memoria_config* self);
int memoria_config_get_marcos_totales(t_memoria_config* self);
int memoria_config_get_procesos_totales(t_memoria_config* self);
bool memoria_config_es_algoritmo_sustitucion_clock(t_memoria_config* self);
bool memoria_config_es_algoritmo_sustitucion_clock_modificado(t_memoria_config* self);
int memoria_config_get_retardo_memoria(t_memoria_config* self);

#endif
