#include "tabla_suspendido.h"

#include <stdbool.h>
#include <stdint.h>

typedef struct
{
    bool bitPresencia;
    bool bitUso;
    bool bitModificado;
} t_entrada_pagina_suspendida;

struct t_proceso_suspendido {
    uint32_t pid;
    uint32_t tamanio;
    t_entrada_pagina_suspendida* paginasSuspendidas;
};

t_proceso_suspendido* crear_tabla_de_suspendidos(void){
//TODO implementar
    return ;
}