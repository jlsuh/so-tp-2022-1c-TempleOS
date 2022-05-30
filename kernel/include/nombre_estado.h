#ifndef NOMBRE_ESTADO_H_INCLUDED
#define NOMBRE_ESTADO_H_INCLUDED

typedef enum {
    NEW,
    READY,
    EXEC,
    EXIT,
    BLOCKED,
    SUSPENDED_READY,
    SUSPENDED_BLOCKED,
    PCBS_ESPERANDO_PARA_IO,
} t_nombre_estado;

#endif
