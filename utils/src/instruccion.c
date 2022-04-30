#include "instruccion.h"

#include <commons/collections/list.h>
#include <commons/log.h>
#include <stdlib.h>

#include "buffer.h"
#include "common_flags.h"

struct t_instruccion {
    t_tipo_instruccion tipoInstruccion;
    uint32_t operando1;
    uint32_t operando2;
};

t_instruccion* instruccion_create(t_tipo_instruccion tipoInstruccion, uint32_t operando1, uint32_t operando2) {
    t_instruccion* self = malloc(sizeof(*self));
    self->tipoInstruccion = tipoInstruccion;
    self->operando1 = operando1;
    self->operando2 = operando2;
    return self;
}

void instruccion_destroy(void* selfVoid) {
    t_instruccion* self = (t_instruccion*)selfVoid;
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
                buffer_unpack(bufferConInstrucciones, &op1, sizeof(op1));
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
        log_info(logger, "Instrucción: %d con operandos: %d y %d", instruccionActual->tipoInstruccion, instruccionActual->operando1, instruccionActual->operando2);
        list_add(instrucciones, instruccionActual);
    }
    return instrucciones;
}
