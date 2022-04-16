#include "buffer.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

t_buffer* buffer_create(void) {
    t_buffer* self = malloc(sizeof(*self));
    self->size = 0;
    self->stream = NULL;
    return self;
}

void buffer_destroy(t_buffer* self) {
    free(self->stream);
    free(self);
}

void buffer_pack(t_buffer* self, void* streamToAdd, int size) {
    self->stream = realloc(self->stream, self->size + size);
    memcpy(self->stream + self->size, streamToAdd, size);
    self->size += size;
}

void buffer_unpack(t_buffer* self, void* dest, int size) {
    if (self->stream == NULL || self->size == 0) {
        puts("\e[0;31mbuffer_unpack: Error en el desempaquetado del buffer\e[0m");
        exit(-1);
    }
    memcpy(dest, self->stream, size);
    self->size -= size;
    memmove(self->stream, self->stream + size, self->size);
    self->stream = realloc(self->stream, self->size);
}

void buffer_pack_string(t_buffer* self, char* stringToAdd) {
    uint32_t length = strlen(stringToAdd) + 1;
    buffer_pack(self, &length, sizeof(length));
    self->stream = realloc(self->stream, self->size + length);
    memcpy(self->stream + self->size, stringToAdd, length);
    self->size += length;
}

char* buffer_unpack_string(t_buffer* self) {
    char* str;
    uint32_t length;
    buffer_unpack(self, &length, sizeof(length));
    str = malloc(length);
    buffer_unpack(self, str, length);
    return str;
}
