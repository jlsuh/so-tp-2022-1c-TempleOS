#include "consola_parser.h"

#include <commons/string.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include "common_flags.h"
#include "consola_serializer.h"

#define MAX_LENGTH_INSTRUCTION 6

bool consola_parser_parse_instructions(t_buffer *buffer, const char *pathInstrucciones, t_log *consolaLogger) {
    bool parseSuccess = false;
    FILE *archivoInstrucciones = fopen(pathInstrucciones, "r");
    uint32_t op1 = -1;
    uint32_t op2 = -1;
    char *instruccion = malloc(MAX_LENGTH_INSTRUCTION);
    char *fscanfError = NULL;
    char const *const fscanfErrorPrefix = "Error al intentar parsear";
    for (;;) {
        if (fscanf(archivoInstrucciones, "%s", instruccion)) {
            bool hayDosArgumentos = false;
            bool shouldFail = false;
            if (strcmp(instruccion, "NO_OP") == 0) {
                if (fscanf(archivoInstrucciones, "%d", &op1)) {
                    for (int i = 0; i < op1; i++) {
                        consola_serializer_pack_no_args(buffer, INSTRUCCION_no_op);
                    }
                } else {
                    shouldFail = true;
                    string_from_format(fscanfError, "%s argumento de NO_OP: %s", fscanfErrorPrefix, strerror(errno));
                }
            } else if (strcmp(instruccion, "I/O") == 0) {
                if (fscanf(archivoInstrucciones, "%d", &op1)) {
                    consola_serializer_pack_one_args(buffer, INSTRUCCION_io, op1);
                } else {
                    shouldFail = true;
                    string_from_format(fscanfError, "%s argumento de I/O: %s", fscanfErrorPrefix, strerror(errno));
                }
            } else if (strcmp(instruccion, "READ") == 0) {
                if (fscanf(archivoInstrucciones, "%d", &op1)) {
                    consola_serializer_pack_one_args(buffer, INSTRUCCION_read, op1);
                } else {
                    shouldFail = true;
                    string_from_format(fscanfError, "%s argumento de READ: %s", fscanfErrorPrefix, strerror(errno));
                }
            } else if (strcmp(instruccion, "COPY") == 0) {
                if (fscanf(archivoInstrucciones, "%d %d", &op1, &op2)) {
                    consola_serializer_pack_two_args(buffer, INSTRUCCION_copy, op1, op2);
                    hayDosArgumentos = true;
                } else {
                    shouldFail = true;
                    string_from_format(fscanfError, "%s argumentos de COPY: %s", fscanfErrorPrefix, strerror(errno));
                }
            } else if (strcmp(instruccion, "WRITE") == 0) {
                if (fscanf(archivoInstrucciones, "%d %d", &op1, &op2)) {
                    consola_serializer_pack_two_args(buffer, INSTRUCCION_write, op1, op2);
                    hayDosArgumentos = true;
                } else {
                    shouldFail = true;
                    string_from_format(fscanfError, "%s argumentos de WRITE: %s", fscanfErrorPrefix, strerror(errno));
                }
            } else if (strcmp(instruccion, "EXIT") == 0) {
                consola_serializer_pack_no_args(buffer, INSTRUCCION_exit);
                log_info(consolaLogger, "Se empaqueta instruccion: EXIT");
                parseSuccess = true;
                break;
            } else {
                log_error(consolaLogger, "Instruccion no reconocida");
                break;
            }

            if (shouldFail) {
                log_error(consolaLogger, "%s", fscanfError);
                free(fscanfError);
                exit(-1);
            } else if (hayDosArgumentos) {
                log_info(consolaLogger, "Se empaqueta instruccion: %s con operandos %d y %d", instruccion, op1, op2);
            } else {
                log_info(consolaLogger, "Se empaqueta instruccion: %s con operando %d", instruccion, op1);
            }
        } else {
            log_error(consolaLogger, "Error al leer el archivo %s de instrucciones: %s", pathInstrucciones, strerror(errno));
            break;
        }
    }
    free(instruccion);
    fclose(archivoInstrucciones);
    return parseSuccess;
}
