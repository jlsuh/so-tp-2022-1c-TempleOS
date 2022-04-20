#ifndef PCB_H_INCLUDED
#define PCB_H_INCLUDED

typedef enum t_nombre_estado{
    NEW,
    READY,
    EXEC,
    EXIT,
    BLOCKED,
    SUSPENDED_READY,
    SUSPENDED_BLOCKED,
} t_nombre_estado;

typedef struct t_instruccion t_instruccion;
typedef struct t_pcb t_pcb;

#endif
