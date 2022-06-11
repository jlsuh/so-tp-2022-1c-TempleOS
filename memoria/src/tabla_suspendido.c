#include "tabla_suspendido.h"

#include <commons/collections/list.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "archivo.h"
#include "tabla_nivel_1.h"
#include "tabla_nivel_2.h"
struct t_proceso_suspendido {
    uint32_t id;
    uint32_t tamanio;
};

t_list* crear_tabla_de_suspendidos(void) {
    t_list* tablaSuspendidos = list_create();
    return tablaSuspendidos;
}

//-----------------------------SUSPENSION PROCESO-------------------------------

void suspender_proceso(uint32_t nroDeTabla1, t_memoria_data_holder memoriaData) {
    uint32_t tamanioNroDeTabla1 = obtener_tamanio(nroDeTabla1, memoriaData);

    abrir_archivo(tamanioNroDeTabla1, nroDeTabla1, memoriaData);
    int entradasPorTabla = memoriaData.entradasPorTabla;

    for (int i = 0; i < entradasPorTabla; i++) {
        int nroDeTabla2 = obtener_tabla_de_nivel_2(nroDeTabla1, i, memoriaData);
        for (int j = 0; j < entradasPorTabla; j++) {
            if (pagina_en_memoria(nroDeTabla2, j, memoriaData)) {
                int pagina = nroDeTabla2 * entradasPorTabla + j;
                int marco = obtener_marco(nroDeTabla2, j, memoriaData);
                log_trace(memoriaData.memoriaLogger, "La pagina %i esta en memoria", pagina);

                memcpy(memoriaData.inicio_archivo + memoriaData.tamanioPagina * pagina, memoriaData.memoriaPrincipal + marco * memoriaData.tamanioPagina, memoriaData.tamanioPagina);
                memset(memoriaData.memoriaPrincipal + marco * memoriaData.tamanioPagina, 0, memoriaData.tamanioPagina);
            }
        }
        limpiar_tabla_nivel_2(nroDeTabla2, memoriaData);
    }

    cerrar_archivo(memoriaData);
}

// ---------------------------- DESPERTAR PROCESO -----------------------------
// TODO usuar crear proceso, y luego pisar en la tabla recibida por crear proceso los bits con los de la tabla de suspendidos y luego liberar la tabla de suspendidos.
// TODO asigna los marcos a los bit de presencia 1 trayendo del archivo la pagina correspondiente.