#ifndef STREAM_H_INCLUDED
#define STREAM_H_INCLUDED

#include <stdint.h>

#include "buffer.h"

/**
 * @brief Recibe solamente el header del buffer enviado
 *
 * @example stream_recv_header(socketModuloRemitente);
 */
uint8_t stream_recv_header(int fromSocket);

/**
 * @brief Recibe solamente el payload (stream de bytes) del buffer enviado
 *
 * @example stream_recv_buffer(socketModuloRemitente, destBuffer);
 */
void stream_recv_buffer(int fromSocket, t_buffer* destBuffer);

/**
 * @brief En combinación con stream_recv_header/1, recibe un buffer que se
 * sabe que vendrá con un payload vacío. En otras palabras, útil para casos en
 * que se quiera recibir solamente un header (sin payload)
 *
 * @example stream_recv_header(socketModuloRemitente);
 *          stream_recv_empty_buffer(socketModuloRemitente);
 */
void stream_recv_empty_buffer(int fromSocket);

/**
 * @brief Envía un buffer previamente serializado, asignándole un header en su envío
 *
 * @example stream_send_buffer(socketModuloDestinatario, header, bufferSerializado);
 */
void stream_send_buffer(int toSocket, uint8_t header, t_buffer* buffer);

/**
 * @brief Envía solamente el header del buffer (sin payload)
 *
 * @example stream_send_empty_buffer(socketModuloDestinatario, header);
 */
void stream_send_empty_buffer(int toSocket, uint8_t header);

#endif
