#ifndef BUFFER_H_INCLUDED
#define BUFFER_H_INCLUDED

#include <stdint.h>

typedef struct {
    uint32_t size;
    void *stream;
} t_buffer;

/**
 * @brief Crea un t_buffer*
 */
t_buffer *buffer_create(void);

/**
 * @brief Genera una nueva copia de un t_buffer*
 */
t_buffer *buffer_create_copy(t_buffer *bufferToCopy);

/**
 * @brief Destruye un t_buffer*
 */
void buffer_destroy(t_buffer *buffer);

/**
 * @brief Agrega al buffer un streamToAdd de tamaño size
 *
 * @example buffer_pack(buffer, unStream, sizeof(unStream));
 */
void buffer_pack(t_buffer *buffer, void *streamToAdd, int size);

/**
 * @brief Desempaqueta size bytes del buffer desde el inicio,
 * almacenando lo desempaquetado en dest
 *
 * @example buffer_unpack(buffer, destino, sizeof(destino));
 */
void buffer_unpack(t_buffer *buffer, void *dest, int size);

/**
 * @brief Agrega al buffer un stringToAdd, empaquetando previamente
 * la longitud del string
 *
 * @example buffer_pack_string(buffer, stringAEmpaquetar);
 */
void buffer_pack_string(t_buffer *buffer, char *stringToAdd);

/**
 * @brief Desempaqueta un string del buffer, leyendo y desempaquetando
 * previamente la longitud del string
 *
 * @example char* unString = buffer_unpack_string(buffer);
 */
char *buffer_unpack_string(t_buffer *buffer);

#endif
