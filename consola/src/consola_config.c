#include "consola_config.h"

#include <commons/string.h>
#include <stdlib.h>
#include <string.h>

#include "module_config.h"

struct t_consola_config {
    char* IP_KERNEL;
    char* PUERTO_KERNEL;
    int SOCKET_KERNEL;
};

static void __consola_config_initializer(void* moduleConfig, t_config* tempCfg) {
    t_consola_config* consolaConfig = (t_consola_config*)moduleConfig;
    consolaConfig->IP_KERNEL = strdup(config_get_string_value(tempCfg, "IP_KERNEL"));
    consolaConfig->PUERTO_KERNEL = strdup(config_get_string_value(tempCfg, "PUERTO_KERNEL"));
    consolaConfig->SOCKET_KERNEL = -1;
}

t_consola_config* consola_config_create(char* consolaConfigPath, t_log* consolaLogger) {
    t_consola_config* self = malloc(sizeof(*self));
    config_init(self, consolaConfigPath, consolaLogger, __consola_config_initializer);
    return self;
}

char* consola_config_get_kernel_ip(t_consola_config* self) {
    return self->IP_KERNEL;
}

char* consola_config_get_kernel_port(t_consola_config* self) {
    return self->PUERTO_KERNEL;
}

int consola_config_get_kernel_socket(t_consola_config* self) {
    return self->SOCKET_KERNEL;
}

void consola_config_set_kernel_socket(t_consola_config* self, int socket) {
    self->SOCKET_KERNEL = socket;
}

void consola_config_destroy(t_consola_config* self) {
    free(self->IP_KERNEL);
    free(self->PUERTO_KERNEL);
    free(self);
}
