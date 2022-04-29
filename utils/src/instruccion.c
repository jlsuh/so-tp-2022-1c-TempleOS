#include "instruccion.h"

#include <stdint.h>
#include <stdlib.h>

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
