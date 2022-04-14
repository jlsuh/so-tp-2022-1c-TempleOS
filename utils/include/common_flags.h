#ifndef COMMON_FLAGS_H_INCLUDED
#define COMMON_FLAGS_H_INCLUDED

typedef enum {
    HEADER_lista_instrucciones,

} t_header;

typedef enum {
    INSTRUCCION_no_op,
    INSTRUCCION_io,
    INSTRUCCION_read,
    INSTRUCCION_copy,
    INSTRUCCION_write,
    INSTRUCCION_exit
} t_instruccion;

#endif
