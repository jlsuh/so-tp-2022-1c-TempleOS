#ifndef COMMON_FLAGS_H_INCLUDED
#define COMMON_FLAGS_H_INCLUDED

typedef enum {
    OK_CONTINUE,
    OK_FINISH,
    MEMORIA,
    FAIL,
    DEADLOCK_END,
} t_handshake;

typedef enum {
    SEM_INIT,
    SEM_WAIT,
    SEM_POST,
    SEM_DESTROY,
    CALL_IO,
    MEM_ALLOC,
    MEM_FREE,
    MEM_READ,
    MEM_WRITE,
    MATE_CLOSE,
    MATE_INIT,
    SUSPEND_CARPINCHO
} t_tarea;

#endif
