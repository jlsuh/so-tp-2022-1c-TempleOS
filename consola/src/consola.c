#include <commons/log.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common_flags.h"
#include "connections.h"
#include "consola_config.h"
#include "consola_parser.h"
#include "stream.h"

#define CONSOLA_CONFIG_PATH "cfg/consola_config.cfg"
#define CONSOLA_LOG_PATH "bin/consola.log"
#define CONSOLA_MODULE_NAME "Consola"
#define NUMBER_OF_ARGS_REQUIRED 3

static void __consola_enviar_instrucciones_a_kernel(const char *pathInstrucciones, t_log *, int kernelSocket);
static void __consola_destroy(t_consola_config *, t_log *);

int main(int argc, char *argv[]) {
    t_log *consolaLogger = log_create(CONSOLA_LOG_PATH, CONSOLA_MODULE_NAME, true, LOG_LEVEL_INFO);
    t_consola_config *consolaConfig = consola_config_create(CONSOLA_CONFIG_PATH, consolaLogger);
    if (argc != NUMBER_OF_ARGS_REQUIRED) {
        log_error(consolaLogger, "Cantidad de argumentos inválida.\nArgumentos: <tamañoProceso> <pathInstrucciones>");
        __consola_destroy(consolaConfig, consolaLogger);
        return -1;
    }

    char *kernelIP = consola_config_get_kernel_ip(consolaConfig);
    char *kernelPort = consola_config_get_kernel_port(consolaConfig);
    const int kernelSocket = conectar_a_servidor(kernelIP, kernelPort);
    if (kernelSocket == -1) {
        log_error(consolaLogger, "Error al intentar establecer conexión inicial con módulo Kernel");
        __consola_destroy(consolaConfig, consolaLogger);
        return -1;
    }
    consola_config_set_kernel_socket(consolaConfig, kernelSocket);

    t_buffer *buffer = buffer_create();
    uint32_t tamanioProceso = atoi(argv[1]);
    buffer_pack(buffer, &tamanioProceso, sizeof(tamanioProceso));
    stream_send_buffer(kernelSocket, HANDSHAKE_consola, buffer);
    buffer_destroy(buffer);

    uint8_t kernelResponse = stream_recv_header(kernelSocket);
    stream_recv_empty_buffer(kernelSocket);
    if (kernelResponse != HANDSHAKE_ok_continue) {
        log_error(consolaLogger, "Error al intentar establecer Handshake inicial con módulo Kernel");
        __consola_destroy(consolaConfig, consolaLogger);
        return -1;
    }

    const char *pathInstrucciones = argv[2];
    __consola_enviar_instrucciones_a_kernel(pathInstrucciones, consolaLogger, kernelSocket);

    consola_config_destroy(consolaConfig);
    log_destroy(consolaLogger);
    return 0;
}

static void __consola_enviar_instrucciones_a_kernel(const char *pathInstrucciones, t_log *consolaLogger, int kernelSocket) {
    t_buffer *instructionsBuffer = buffer_create();
    consola_parser_parse_instructions(instructionsBuffer, pathInstrucciones, consolaLogger);
    stream_send_buffer(kernelSocket, HEADER_lista_instrucciones, instructionsBuffer);
    log_info(consolaLogger, "Se envía la lista de instrucciones al Kernel");
    buffer_destroy(instructionsBuffer);
    return;
}

static void __consola_destroy(t_consola_config *consolaConfig, t_log *consolaLogger) {
    consola_config_destroy(consolaConfig);
    log_destroy(consolaLogger);
}
