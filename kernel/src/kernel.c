#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#include <commons/log.h>

#include "kernel_config.h"

#define KERNEL_CONFIG_PATH "cfg/kernel_config.cfg"
#define KERNEL_LOG_PATH "bin/kernel.log"
#define KERNEL_MODULE_NAME "Kernel"

t_log* kernelLogger;
t_kernel_config* kernelConfig;

int main(int argc, char* argv[]) {
    kernelLogger = log_create(KERNEL_LOG_PATH, KERNEL_MODULE_NAME, true, LOG_LEVEL_INFO);
    kernelConfig = kernel_config_create(KERNEL_CONFIG_PATH, kernelLogger);
    return 0;
}
