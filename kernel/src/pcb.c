#include "pcb.h"

#include <commons/collections/list.h>
#include <stdint.h>

#include "common_flags.h"

struct t_instruccion {
    t_tipo_instruccion tipoInstruccion;
    uint32_t operando1;
    uint32_t operando2;
};

struct t_pcb {
    uint32_t pid;
    uint32_t tamanio;
    t_list* instrucciones;
    uint64_t programCounter;  // Contiene un índice de la t_list instrucciones
    uint32_t tablaPaginaPrimerNivel;
    double estimacionRafaga;
    t_nombre_estado estadoActual;
};
