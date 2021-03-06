#include "kernel_config.h"

#include <commons/config.h>
#include <commons/log.h>
#include <stdlib.h>
#include <string.h>

#include "module_config.h"

#define MILISECS_IN_SECS 1000.0

t_log* kernelLogger;
t_kernel_config* kernelConfig;

struct t_kernel_config {
    char* IP_MEMORIA;
    char* PUERTO_MEMORIA;
    char* IP_CPU;
    char* PUERTO_CPU_DISPATCH;
    char* PUERTO_CPU_INTERRUPT;
    char* IP_ESCUCHA;
    char* PUERTO_ESCUCHA;
    char* ALGORITMO_PLANIFICACION;
    double ESTIMACION_INICIAL;
    double ALFA;
    int SOCKET_MEMORIA;
    int SOCKET_DISPATCH_CPU;
    int SOCKET_INTERRUPT_CPU;
    int GRADO_MULTIPROGRAMACION;
    int TIEMPO_MAXIMO_BLOQUEADO;
};

static void __kernel_config_initializer(void* moduleConfig, t_config* tempCfg) {
    t_kernel_config* kernelConfig = (t_kernel_config*)moduleConfig;
    kernelConfig->IP_MEMORIA = strdup(config_get_string_value(tempCfg, "IP_MEMORIA"));
    kernelConfig->PUERTO_MEMORIA = strdup(config_get_string_value(tempCfg, "PUERTO_MEMORIA"));
    kernelConfig->IP_CPU = strdup(config_get_string_value(tempCfg, "IP_CPU"));
    kernelConfig->PUERTO_CPU_DISPATCH = strdup(config_get_string_value(tempCfg, "PUERTO_CPU_DISPATCH"));
    kernelConfig->PUERTO_CPU_INTERRUPT = strdup(config_get_string_value(tempCfg, "PUERTO_CPU_INTERRUPT"));
    kernelConfig->IP_ESCUCHA = strdup(config_get_string_value(tempCfg, "IP_ESCUCHA"));
    kernelConfig->PUERTO_ESCUCHA = strdup(config_get_string_value(tempCfg, "PUERTO_ESCUCHA"));
    kernelConfig->ALGORITMO_PLANIFICACION = strdup(config_get_string_value(tempCfg, "ALGORITMO_PLANIFICACION"));
    kernelConfig->ESTIMACION_INICIAL = config_get_double_value(tempCfg, "ESTIMACION_INICIAL");
    kernelConfig->ALFA = config_get_double_value(tempCfg, "ALFA");
    kernelConfig->SOCKET_MEMORIA = -1;
    kernelConfig->SOCKET_DISPATCH_CPU = -1;
    kernelConfig->SOCKET_INTERRUPT_CPU = -1;
    kernelConfig->GRADO_MULTIPROGRAMACION = config_get_int_value(tempCfg, "GRADO_MULTIPROGRAMACION");
    kernelConfig->TIEMPO_MAXIMO_BLOQUEADO = config_get_int_value(tempCfg, "TIEMPO_MAXIMO_BLOQUEADO");
}

t_kernel_config* kernel_config_create(char* kernelConfigPath, t_log* kernelLogger) {
    t_kernel_config* self = malloc(sizeof(*self));
    config_init(self, kernelConfigPath, kernelLogger, __kernel_config_initializer);
    return self;
}

void kernel_config_destroy(t_kernel_config* self) {
    free(self->IP_MEMORIA);
    free(self->PUERTO_MEMORIA);
    free(self->IP_CPU);
    free(self->PUERTO_CPU_DISPATCH);
    free(self->PUERTO_CPU_INTERRUPT);
    free(self->IP_ESCUCHA);
    free(self->PUERTO_ESCUCHA);
    free(self->ALGORITMO_PLANIFICACION);
    free(self);
}

double kernel_config_get_est_inicial(t_kernel_config* self) {
    return self->ESTIMACION_INICIAL;
}

char* kernel_config_get_ip_escucha(t_kernel_config* self) {
    return self->IP_ESCUCHA;
}

char* kernel_config_get_puerto_escucha(t_kernel_config* self) {
    return self->PUERTO_ESCUCHA;
}

char* kernel_config_get_ip_cpu(t_kernel_config* self) {
    return self->IP_CPU;
}

char* kernel_config_get_puerto_cpu_dispatch(t_kernel_config* self) {
    return self->PUERTO_CPU_DISPATCH;
}

char* kernel_config_get_puerto_cpu_interrupt(t_kernel_config* self) {
    return self->PUERTO_CPU_INTERRUPT;
}

char* kernel_config_get_puerto_memoria(t_kernel_config* self) {
    return self->PUERTO_MEMORIA;
}

char* kernel_config_get_ip_memoria(t_kernel_config* self) {
    return self->IP_MEMORIA;
}

void kernel_config_set_socket_dispatch_cpu(t_kernel_config* self, int socketDispatch) {
    self->SOCKET_DISPATCH_CPU = socketDispatch;
}

void kernel_config_set_socket_interrupt_cpu(t_kernel_config* self, int socketInterrupt) {
    self->SOCKET_INTERRUPT_CPU = socketInterrupt;
}

void kernel_config_set_socket_memoria(t_kernel_config* self, int socketMemoria) {
    self->SOCKET_MEMORIA = socketMemoria;
}

int kernel_config_get_socket_dispatch_cpu(t_kernel_config* self) {
    return self->SOCKET_DISPATCH_CPU;
}

int kernel_config_get_socket_interrupt_cpu(t_kernel_config* self) {
    return self->SOCKET_INTERRUPT_CPU;
}

int kernel_config_get_socket_memoria(t_kernel_config* self) {
    return self->SOCKET_MEMORIA;
}

int kernel_config_get_grado_multiprogramacion(t_kernel_config* self) {
    return self->GRADO_MULTIPROGRAMACION;
}

double kernel_config_get_alfa(t_kernel_config* self) {
    return self->ALFA;
}

int kernel_config_get_tiempo_maximo_bloqueado(t_kernel_config* self) {
    return self->TIEMPO_MAXIMO_BLOQUEADO;
}

double kernel_config_get_tiempo_maximo_bloqueado_en_secs(t_kernel_config* self) {
    return self->TIEMPO_MAXIMO_BLOQUEADO / MILISECS_IN_SECS;
}

bool kernel_config_es_algoritmo_srt(t_kernel_config* self) {
    return strcmp(self->ALGORITMO_PLANIFICACION, "SRT") == 0;
}

bool kernel_config_es_algoritmo_fifo(t_kernel_config* self) {
    return strcmp(self->ALGORITMO_PLANIFICACION, "FIFO") == 0;
}
