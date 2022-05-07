#ifndef COMMON_FLAGS_H_INCLUDED
#define COMMON_FLAGS_H_INCLUDED

typedef enum {
    HEADER_lista_instrucciones,
    HEADER_nuevo_proceso,
    HEADER_proceso_terminado,
    HEADER_proceso_bloqueado,
    HEADER_proceso_desalojado,
    HEADER_proceso_suspendido,
    HEADER_read,
    HEADER_write,
    HEADER_copy,
    HEADER_tabla_de_paginas,
    HEADER_solicitud_tabla_paginas,
    HEADER_pcb_a_ejecutar,
    HEADER_memoria_insuficiente,
    HEADER_error,
    HEADER_tabla_nivel_2,
    HEADER_marco,
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
} t_tipo_instruccion;

typedef enum {
    INT_interrumpir_ejecucion,
    INT_interrupcion_recibida,
} t_interrupciones;

#endif
