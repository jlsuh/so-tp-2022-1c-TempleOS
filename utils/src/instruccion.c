#include "instruccion.h"

#include <commons/collections/list.h>
#include <commons/log.h>
#include <commons/string.h>
#include <stdlib.h>

#include "buffer.h"
#include "common_flags.h"

struct t_instruccion {
    t_tipo_instruccion tipoInstruccion;
    uint32_t operando1;
    uint32_t operando2;
    char* toString;
};

static char* __instruccion_to_string(t_instruccion* self) {
    uint32_t operando1 = self->operando1;
    uint32_t operando2 = self->operando2;
    return self->tipoInstruccion == INSTRUCCION_no_op   ? string_from_format("NO_OP")
           : self->tipoInstruccion == INSTRUCCION_exit  ? string_from_format("EXIT")
           : self->tipoInstruccion == INSTRUCCION_io    ? string_from_format("I/O %d", operando1)
           : self->tipoInstruccion == INSTRUCCION_read  ? string_from_format("READ %d", operando1)
           : self->tipoInstruccion == INSTRUCCION_write ? string_from_format("WRITE %d %d", operando1, operando2)
           : self->tipoInstruccion == INSTRUCCION_copy  ? string_from_format("COPY %d %d", operando1, operando2)
                                                        : string_from_format("UNKNOWN");
}

t_instruccion* instruccion_create(t_tipo_instruccion tipoInstruccion, uint32_t operando1, uint32_t operando2) {
    t_instruccion* self = malloc(sizeof(*self));
    self->tipoInstruccion = tipoInstruccion;
    self->operando1 = operando1;
    self->operando2 = operando2;
    self->toString = __instruccion_to_string(self);
    return self;
}

void instruccion_destroy(void* selfVoid) {
    t_instruccion* self = (t_instruccion*)selfVoid;
    free(self->toString);
    free(self);
}

t_list* instruccion_list_create_from_buffer(t_buffer* bufferConInstrucciones, t_log* logger) {
    t_list* instrucciones = list_create();
    uint8_t instruction = -1;
    bool isExit = false;
    while (!isExit) {
        buffer_unpack(bufferConInstrucciones, &instruction, sizeof(instruction));
        uint32_t op1 = -1;
        uint32_t op2 = -1;
        switch (instruction) {
            case INSTRUCCION_no_op:
                break;
            case INSTRUCCION_io:
                buffer_unpack(bufferConInstrucciones, &op1, sizeof(op1));
                break;
            case INSTRUCCION_read:
                buffer_unpack(bufferConInstrucciones, &op1, sizeof(op1));
                break;
            case INSTRUCCION_copy:
                buffer_unpack(bufferConInstrucciones, &op1, sizeof(op1));
                buffer_unpack(bufferConInstrucciones, &op2, sizeof(op2));
                break;
            case INSTRUCCION_write:
                buffer_unpack(bufferConInstrucciones, &op1, sizeof(op1));
                buffer_unpack(bufferConInstrucciones, &op2, sizeof(op2));
                break;
            case INSTRUCCION_exit:
                isExit = true;
                break;
            default:
                log_error(logger, "Error al intentar desempaquetar una instrucción");
                exit(-1);
        }
        t_instruccion* instruccionActual = instruccion_create(instruction, op1, op2);
        list_add(instrucciones, instruccionActual);
    }
    log_info(logger, "Se desempaquetan %d instrucciones", list_size(instrucciones));
    return instrucciones;
}

t_tipo_instruccion instruccion_get_tipo_instruccion(t_instruccion* self) {
    return self->tipoInstruccion;
}

uint32_t instruccion_get_operando1(t_instruccion* self) {
    return self->operando1;
}

uint32_t instruccion_get_operando2(t_instruccion* self) {
    return self->operando2;
}

char const* instruccion_to_string(t_instruccion* self) {
    return self->toString;
}
