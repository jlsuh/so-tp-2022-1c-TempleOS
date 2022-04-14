#ifndef MODULE_CONFIG_H_INCLUDED
#define MODULE_CONFIG_H_INCLUDED

#include <commons/config.h>
#include <commons/log.h>

int config_init(void* moduleConfig, char* pathToConfig, t_log* moduleLogger,
                void (*config_initializer)(void* moduleConfig, t_config* tempConfig));

#endif
