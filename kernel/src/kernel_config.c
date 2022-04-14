#include "kernel_config.h"

#include "module_config.h"

#include <commons/config.h>
#include <stdlib.h>
#include <string.h>

struct t_kernel_config {
    char* IP_MEMORIA;
    char* PUERTO_MEMORIA;
    int SOCKET_MEMORIA;
    char* IP_CPU;
    char* PUERTO_CPU_DISPATCH;
    int SOCKET_DISPATCH_CPU;
    char* PUERTO_CPU_INTERRUPT;
    int SOCKET_INTERRUPT_CPU;
    char* PUERTO_ESCUCHA;
    char* ALGORITMO_PLANIFICACION;
    int ESTIMACION_INICIAL;
    double ALFA;
    int GRADO_MULTIPROGRAMACION;
    int TIEMPO_MAXIMO_BLOQUEADO;
};

static void __kernel_config_initializer(void* moduleConfig, t_config* tempCfg) {
    t_kernel_config* kernelConfig = (t_kernel_config*)moduleConfig;
    kernelConfig->IP_MEMORIA = strdup(config_get_string_value(tempCfg, "IP_MEMORIA"));
    kernelConfig->PUERTO_MEMORIA = strdup(config_get_string_value(tempCfg, "PUERTO_MEMORIA"));
    kernelConfig->SOCKET_MEMORIA = -1;
    kernelConfig->IP_CPU = strdup(config_get_string_value(tempCfg, "IP_CPU"));
    kernelConfig->PUERTO_CPU_DISPATCH = strdup(config_get_string_value(tempCfg, "PUERTO_CPU_DISPATCH"));
    kernelConfig->SOCKET_DISPATCH_CPU = -1;
    kernelConfig->PUERTO_CPU_INTERRUPT = strdup(config_get_string_value(tempCfg, "PUERTO_CPU_INTERRUPT"));
    kernelConfig->SOCKET_INTERRUPT_CPU = -1;
    kernelConfig->PUERTO_ESCUCHA = strdup(config_get_string_value(tempCfg, "PUERTO_ESCUCHA"));
    kernelConfig->ALGORITMO_PLANIFICACION = strdup(config_get_string_value(tempCfg, "ALGORITMO_PLANIFICACION"));
    kernelConfig->ESTIMACION_INICIAL = config_get_int_value(tempCfg, "ESTIMACION_INICIAL");
    kernelConfig->ALFA = config_get_double_value(tempCfg, "ALFA");
    kernelConfig->GRADO_MULTIPROGRAMACION = config_get_int_value(tempCfg, "GRADO_MULTIPROGRAMACION");
    kernelConfig->TIEMPO_MAXIMO_BLOQUEADO = config_get_int_value(tempCfg, "TIEMPO_MAXIMO_BLOQUEADO");
}

t_kernel_config* kernel_config_create(char* kernelConfigPath, t_log* kernelLogger) {
    t_kernel_config* self = malloc(sizeof(*self));
    config_init(self, kernelConfigPath, kernelLogger, __kernel_config_initializer);
    return self;
}
