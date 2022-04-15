#include "consola_serializer.h"

void consola_serializer_pack_no_args(t_buffer* buffer, uint8_t instruccion) {
    buffer_pack(buffer, &instruccion, sizeof(instruccion));
}

void consola_serializer_pack_one_args(t_buffer* buffer, uint8_t instruccion, uint32_t op1) {
    buffer_pack(buffer, &instruccion, sizeof(instruccion));
    buffer_pack(buffer, &op1, sizeof(op1));
}

void consola_serializer_pack_two_args(t_buffer* buffer, uint8_t instruccion, uint32_t op1, uint32_t op2) {
    buffer_pack(buffer, &instruccion, sizeof(instruccion));
    buffer_pack(buffer, &op1, sizeof(op1));
    buffer_pack(buffer, &op2, sizeof(op2));
}
