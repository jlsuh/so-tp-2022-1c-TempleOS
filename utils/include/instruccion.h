#ifndef INSTRUCCION_H_INCLUDED
#define INSTRUCCION_H_INCLUDED

#include <stdint.h>

#include "common_flags.h"

typedef struct t_instruccion t_instruccion;

t_instruccion* instruccion_create(t_tipo_instruccion tipoInstruccion, uint32_t operando1, uint32_t operando2);
void instruccion_destroy(void* selfVoid);

#endif
