#ifndef MEMORIA_CONFIG_H_INCLUDED
#define MEMORIA_CONFIG_H_INCLUDED

#include <commons/log.h>

typedef struct t_memoria_config t_memoria_config;

bool memoria_config_es_algoritmo_sustitucion_clock(t_memoria_config*);
bool memoria_config_es_algoritmo_sustitucion_clock_modificado(t_memoria_config*);
char* memoria_config_get_ip_escucha(t_memoria_config*);
char* memoria_config_get_path_swap(t_memoria_config*);
char* memoria_config_get_puerto_escucha(t_memoria_config*);
int memoria_config_get_entradas_por_tabla(t_memoria_config*);
int memoria_config_get_marcos_por_proceso(t_memoria_config*);
int memoria_config_get_procesos_totales(t_memoria_config*);
int memoria_config_get_retardo_memoria(t_memoria_config*);
int memoria_config_get_retardo_swap(t_memoria_config*);
int memoria_config_get_tamanio_memoria(t_memoria_config*);
int memoria_config_get_tamanio_pagina(t_memoria_config*);
t_memoria_config* memoria_config_create(char* memoriaConfigPath, t_log*);

#endif
