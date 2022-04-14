#ifndef CONSOLA_SERIALIZER_H_INCLUDED
#define CONSOLA_SERIALIZER_H_INCLUDED

#include "buffer.h"
#include "common_flags.h"

void consola_serializer_pack_no_args(t_buffer*, uint8_t instruccion);
void consola_serializer_pack_one_args(t_buffer*, uint8_t instruccion, uint32_t op1);
void consola_serializer_pack_two_args(t_buffer*, uint8_t instruccion, uint32_t op1, uint32_t op2);

#endif
