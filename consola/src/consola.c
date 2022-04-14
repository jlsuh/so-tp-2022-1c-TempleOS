#include <commons/log.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "buffer.h"
#include "common_flags.h"
#include "connections.h"
#include "consola_config.h"
#include "consola_serializer.h"
#include "stream.h"

#define CONSOLA_CONFIG_PATH "cfg/consola_config.cfg"
#define CONSOLA_LOG_PATH "bin/consola.log"
#define CONSOLA_MODULE_NAME "Consola"
#define MAX_LENGTH_INSTRUCTION 6

static void __consola_enviar_instrucciones_a_kernel(const char* pathInstrucciones, t_log* consolaLogger) {
    FILE *archivoInstrucciones = fopen(pathInstrucciones, "r");
    t_buffer *buffer = buffer_create();
    uint32_t op1 = 0;
    uint32_t op2 = 0;
    char *instruccion = malloc(MAX_LENGTH_INSTRUCTION);
    for (;;) {
        fscanf(archivoInstrucciones, "%s", instruccion);
        if (strcmp(instruccion, "NO_OP") == 0) {
            fscanf(archivoInstrucciones, "%d", &op1);
            consola_serializer_pack_one_args(buffer, INSTRUCCION_no_op, op1);
        } else if (strcmp(instruccion, "I/O") == 0) {
            fscanf(archivoInstrucciones, "%d", &op1);
            consola_serializer_pack_one_args(buffer, INSTRUCCION_io, op1);
        } else if (strcmp(instruccion, "READ") == 0) {
            fscanf(archivoInstrucciones, "%d", &op1);
            consola_serializer_pack_one_args(buffer, INSTRUCCION_read, op1);
        } else if (strcmp(instruccion, "COPY") == 0) {
            fscanf(archivoInstrucciones, "%d %d", &op1, &op2);
            consola_serializer_pack_two_args(buffer, INSTRUCCION_copy, op1, op2);
        } else if (strcmp(instruccion, "WRITE") == 0) {
            fscanf(archivoInstrucciones, "%d %d", &op1, &op2);
            consola_serializer_pack_two_args(buffer, INSTRUCCION_write, op1, op2);
        } else if (strcmp(instruccion, "EXIT") == 0) {
            consola_serializer_pack_no_args(buffer, INSTRUCCION_exit);
            free(instruccion);
            break;
        } else {
            log_error(consolaLogger, "Instruccion invalida");
            // Free memory
            return;
        }
    }
    buffer_destroy(buffer);
    fclose(archivoInstrucciones);
    return;
}

static void __consola_destroy(t_consola_config* consolaConfig, t_log* consolaLogger) {
    consola_config_destroy(consolaConfig);
    log_destroy(consolaLogger);
}

int main(int argc, char *argv[]) {
    t_log *consolaLogger = log_create(CONSOLA_LOG_PATH, CONSOLA_MODULE_NAME, true, LOG_LEVEL_INFO);
    t_consola_config *consolaConfig = consola_config_create(CONSOLA_CONFIG_PATH, consolaLogger);
    if (argc != 3) {
        log_error(consolaLogger, "Cantidad de argumentos invalida.\n Formato <tamaño> <path>");
        __consola_destroy(consolaConfig, consolaLogger);
        return -1;
    }

    char *kernelIP = consola_config_get_kernel_ip(consolaConfig);
    char *kernelPort = consola_config_get_kernel_port(consolaConfig);

    const int kernelSocket = conectar_a_servidor(kernelIP, kernelPort);
    if (kernelSocket == -1) {
        log_error(consolaLogger, "Error al intentar conectar con módulo Kernel");
        __consola_destroy(consolaConfig, consolaLogger);
        return -1;
    }
    consola_config_set_kernel_socket(consolaConfig, kernelSocket);

    const uint32_t tamanioProceso = atoi(argv[1]);  // TODO: Enviar esto a Kernel en el handshake inicial

    stream_send_empty_buffer(kernelSocket, HANDSHAKE_consola);
    uint8_t response = stream_recv_header(kernelSocket);
    if(response != HANDSHAKE_ok_continue) {
        log_error(consolaLogger, "Error al intentar conectar con módulo Kernel");
        __consola_destroy(consolaConfig, consolaLogger);
        return -1;
    }

    const char *pathInstrucciones = argv[2];
    __consola_enviar_instrucciones_a_kernel(pathInstrucciones, consolaLogger);

    consola_config_destroy(consolaConfig);
    log_destroy(consolaLogger);
    return 0;
}
