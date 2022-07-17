#ifndef COMMON_FLAGS_H_INCLUDED
#define COMMON_FLAGS_H_INCLUDED

typedef enum {
    HEADER_error,
    HEADER_interrumpir_ejecucion,
    HEADER_lista_instrucciones,
    HEADER_marco,
    HEADER_memoria_insuficiente,
    HEADER_pcb_a_ejecutar,
    HEADER_pcb_a_ejecutar_ultimo_suspendido,
    HEADER_pid,
    HEADER_proceso_bloqueado,
    HEADER_proceso_desalojado,
    HEADER_proceso_suspendido,
    HEADER_proceso_terminado,
    HEADER_read,
    HEADER_solicitud_tabla_paginas,
    HEADER_tabla_nivel_2,
    HEADER_write,
} t_header;

typedef enum {
    HANDSHAKE_consola,
    HANDSHAKE_cpu,
    HANDSHAKE_dispatch,
    HANDSHAKE_interrupt,
    HANDSHAKE_kernel,
    HANDSHAKE_ok_continue
} t_handshake;

typedef enum {
    INSTRUCCION_copy,
    INSTRUCCION_exit,
    INSTRUCCION_io,
    INSTRUCCION_no_op,
    INSTRUCCION_read,
    INSTRUCCION_write
} t_tipo_instruccion;

#endif
