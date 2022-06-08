#include "tabla_suspendido.h"

#include <stdbool.h>
#include <stdint.h>
#include <commons/collections/list.h>

struct t_proceso_suspendido {
    uint32_t id;
    uint32_t tamanio;
};

t_list* crear_tabla_de_suspendidos(void){
    t_list* tablaSuspendidos = list_create();
    return tablaSuspendidos;
}

