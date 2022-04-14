#include "module_config.h"

#include <stdlib.h>

int config_init(void* moduleConfig, char* pathToConfig, t_log* moduleLogger,
                void (*config_initializer)(void* moduleConfig, t_config* tempConfig)) {
    t_config* tempConfig = config_create(pathToConfig);
    if (NULL == tempConfig) {
        log_error(moduleLogger, "Path \"%s\" no encontrado", pathToConfig);
        return EXIT_FAILURE;
    }
    config_initializer(moduleConfig, tempConfig);
    log_info(moduleLogger, "Inicialización de campos correcta");
    config_destroy(tempConfig);
    return EXIT_SUCCESS;
}
