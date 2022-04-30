#include "cpu_config.h"

#include <commons/config.h>
#include <commons/log.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "module_config.h"

struct t_cpu_config {
    char* IP;
    uint32_t ENTRADAS_TLB;
    char* REEMPLAZO_TLB;
    uint32_t RETARDO_NOOP;
    char* IP_MEMORIA;
    char* PUERTO_MEMORIA;
    char* PUERTO_ESCUCHA_DISPATCH;
    char* PUERTO_ESCUCHA_INTERRUPT;
    int SOCKET_MEMORIA;
    int CANT_ENTRADAS_POR_TP;
    int TAM_PAGINA;
    int SOCKET_DISPATCH;
    int SOCKET_INTERRUPT;
};

static void __cpu_config_initializer(void* moduleConfig, t_config* tempCfg) {
    t_cpu_config* cpuConfig = (t_cpu_config*)moduleConfig;
    cpuConfig->IP = strdup(config_get_string_value(tempCfg, "IP"));
    cpuConfig->ENTRADAS_TLB = config_get_int_value(tempCfg, "ENTRADAS_TLB");
    cpuConfig->REEMPLAZO_TLB = strdup(config_get_string_value(tempCfg, "REEMPLAZO_TLB"));
    cpuConfig->RETARDO_NOOP = config_get_int_value(tempCfg, "RETARDO_NOOP");
    cpuConfig->IP_MEMORIA = strdup(config_get_string_value(tempCfg, "IP_MEMORIA"));
    cpuConfig->PUERTO_MEMORIA = strdup(config_get_string_value(tempCfg, "PUERTO_MEMORIA"));
    cpuConfig->PUERTO_ESCUCHA_DISPATCH = strdup(config_get_string_value(tempCfg, "PUERTO_ESCUCHA_DISPATCH"));
    cpuConfig->PUERTO_ESCUCHA_INTERRUPT = strdup(config_get_string_value(tempCfg, "PUERTO_ESCUCHA_INTERRUPT"));
    cpuConfig->SOCKET_MEMORIA = -1;
    cpuConfig->CANT_ENTRADAS_POR_TP = -1;
    cpuConfig->TAM_PAGINA = -1;
    cpuConfig->SOCKET_DISPATCH = -1;
    cpuConfig->SOCKET_INTERRUPT = -1;
}

t_cpu_config* cpu_config_create(char* cpuConfigPath, t_log* cpuLogger) {
    t_cpu_config* self = malloc(sizeof(*self));
    config_init(self, cpuConfigPath, cpuLogger, __cpu_config_initializer);
    return self;
}

char* cpu_config_get_ip_memoria(t_cpu_config* self) {
    return self->IP_MEMORIA;
}

char* cpu_config_get_puerto_memoria(t_cpu_config* self) {
    return self->PUERTO_MEMORIA;
}

char* cpu_config_get_puerto_dispatch(t_cpu_config* self) {
    return self->PUERTO_ESCUCHA_DISPATCH;
}

char* cpu_config_get_puerto_interrupt(t_cpu_config* self) {
    return self->PUERTO_ESCUCHA_INTERRUPT;
}

char* cpu_config_get_ip_cpu(t_cpu_config* self) {
    return self->IP;
}

int cpu_config_get_socket_dispatch(t_cpu_config* self) {
    return self->SOCKET_DISPATCH;
}

int cpu_config_get_socket_interrupt(t_cpu_config* self) {
    return self->SOCKET_INTERRUPT;
}

int cpu_config_get_socket_memoria(t_cpu_config* self) {
    return self->SOCKET_MEMORIA;
}

void cpu_config_set_socket_memoria(t_cpu_config* self, int socketMemoria) {
    self->SOCKET_MEMORIA = socketMemoria;
}

void cpu_config_set_socket_dispatch(t_cpu_config* self, int socketDispatch) {
    self->SOCKET_DISPATCH = socketDispatch;
}

void cpu_config_set_socket_interrupt(t_cpu_config* self, int socketInterrupt) {
    self->SOCKET_INTERRUPT = socketInterrupt;
}

void cpu_config_set_entradas_por_tabla(t_cpu_config* self, int cantEntradas) {
    self->CANT_ENTRADAS_POR_TP = cantEntradas;
}

int cpu_config_get_entradas_por_tabla(t_cpu_config* self) {
    return self->CANT_ENTRADAS_POR_TP;
}

void cpu_config_set_tamanio_pagina(t_cpu_config* self, int tamanioPagina) {
    self->TAM_PAGINA = tamanioPagina;
}

int cpu_config_get_tamanio_pagina(t_cpu_config* self) {
    return self->TAM_PAGINA;
}
