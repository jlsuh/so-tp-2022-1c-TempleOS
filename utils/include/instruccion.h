#ifndef INSTRUCCION_H_INCLUDED
#define INSTRUCCION_H_INCLUDED

#include <commons/collections/list.h>
#include <commons/log.h>
#include <stdint.h>

#include "buffer.h"
#include "common_flags.h"

typedef struct t_instruccion t_instruccion;

t_instruccion* instruccion_create(t_tipo_instruccion tipoInstruccion, uint32_t operando1, uint32_t operando2);
void instruccion_destroy(void* selfVoid);
t_list* instruccion_list_create_from_buffer(t_buffer* bufferConInstrucciones, t_log*);
t_tipo_instruccion instruccion_get_tipo_instruccion(t_instruccion*);
uint32_t instruccion_get_operando1(t_instruccion*);
uint32_t instruccion_get_operando2(t_instruccion*);

#endif
