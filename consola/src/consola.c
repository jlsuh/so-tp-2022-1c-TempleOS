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

#define CONSOLA_LOG_PATH "bin/consola.log"
#define CONSOLA_CONFIG_PATH "cfg/consola_config.cfg"
#define CONSOLA_MODULE_NAME "Consola"
#define MAX_LENGTH_INSTRUCTION 6

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Cantidad de argumentos invalida.\n Formato <tamaño> <path>");
        return -1;
    }
    uint32_t tamanio = atoi(argv[1]);  // TODO: Enviar esto a Kernel en el handshake inicial
    char *pathInstrucciones = argv[2];

    bool activeConsole = true;
    t_log *consolaLogger = log_create(CONSOLA_LOG_PATH, CONSOLA_MODULE_NAME, activeConsole, LOG_LEVEL_INFO);
    t_consola_config *consolaConfig = consola_config_create(CONSOLA_CONFIG_PATH, consolaLogger);

    /* char *kernelIP = consola_config_get_kernel_ip(consolaConfig);
    char *kernelPort = consola_config_get_kernel_port(consolaConfig);

    int connected = conectar_a_servidor(kernelIP, kernelPort);

    if (-1 == connected) {
        return -1;
    } */

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
            return -1;
        }
    }
    buffer_destroy(buffer);
    fclose(archivoInstrucciones);
    consola_config_destroy(consolaConfig);
    log_destroy(consolaLogger);
    return 0;
}
