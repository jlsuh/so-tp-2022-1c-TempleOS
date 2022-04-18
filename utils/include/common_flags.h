#ifndef COMMON_FLAGS_H_INCLUDED
#define COMMON_FLAGS_H_INCLUDED

typedef enum {
    HEADER_lista_instrucciones,
} t_header;

typedef enum {
    HANDSHAKE_consola,
    HANDSHAKE_ok_continue,
    HANDSHAKE_cpu,
    HANDSHAKE_kernel,
    HANDSHAKE_dispatch,
    HANDSHAKE_interrupt,
} t_handshake;

typedef enum {
    INSTRUCCION_no_op,
    INSTRUCCION_io,
    INSTRUCCION_read,
    INSTRUCCION_copy,
    INSTRUCCION_write,
    INSTRUCCION_exit
} t_instruccion;

#endif
