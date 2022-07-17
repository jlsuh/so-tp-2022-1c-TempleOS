#ifndef CPU_CONFIG_H_INCLUDED
#define CPU_CONFIG_H_INCLUDED

#include <commons/log.h>
#include <stdint.h>

typedef struct t_cpu_config t_cpu_config;

char* cpu_config_get_ip_cpu(t_cpu_config*);
char* cpu_config_get_ip_memoria(t_cpu_config*);
char* cpu_config_get_puerto_dispatch(t_cpu_config*);
char* cpu_config_get_puerto_interrupt(t_cpu_config*);
char* cpu_config_get_puerto_memoria(t_cpu_config*);
char* cpu_config_get_reemplazo_tlb(t_cpu_config*);
int cpu_config_get_entradas_por_tabla(t_cpu_config*);
int cpu_config_get_socket_dispatch(t_cpu_config*);
int cpu_config_get_socket_interrupt(t_cpu_config*);
int cpu_config_get_socket_memoria(t_cpu_config*);
int cpu_config_get_tamanio_pagina(t_cpu_config*);
t_cpu_config* cpu_config_create(char* cpuConfigPath, t_log*);
uint32_t cpu_config_get_entradas_tlb(t_cpu_config*);
uint32_t cpu_config_get_retardo_no_op(t_cpu_config*);
void cpu_config_set_entradas_por_tabla(t_cpu_config*, int cantEntradas);
void cpu_config_set_socket_dispatch(t_cpu_config*, int socketDispatch);
void cpu_config_set_socket_interrupt(t_cpu_config*, int socketInterrupt);
void cpu_config_set_socket_memoria(t_cpu_config*, int socketMemoria);
void cpu_config_set_tamanio_pagina(t_cpu_config*, int tamanioPagina);

#endif
