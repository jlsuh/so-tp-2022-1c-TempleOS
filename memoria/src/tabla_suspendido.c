#include "tabla_suspendido.h"

#include <commons/collections/dictionary.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "archivo.h"
#include "tabla_nivel_1.h"
#include "tabla_nivel_2.h"

t_dictionary* crear_tabla_de_suspendidos(void) {
    return dictionary_create();
}


void __aniadir_a_tabla(uint32_t nroDeTabla1, uint32_t tamanioNroDeTabla1, t_memoria_data_holder memoriaData){
    int length = snprintf( NULL, 0, "%d", nroDeTabla1 );
    char* nroDeTabla1Str = malloc( length + 1 );
    snprintf( nroDeTabla1Str, length + 1, "%d", nroDeTabla1 );
    dictionary_put(memoriaData.tablaSuspendidos, nroDeTabla1Str, (void*) &tamanioNroDeTabla1);
    free(nroDeTabla1Str);
}

void suspender_proceso(uint32_t nroDeTabla1, t_memoria_data_holder memoriaData) {
    uint32_t tamanioNroDeTabla1 = obtener_tamanio(nroDeTabla1, memoriaData);
    __aniadir_a_tabla(nroDeTabla1, tamanioNroDeTabla1, memoriaData);

    abrir_archivo(tamanioNroDeTabla1, nroDeTabla1, memoriaData);
    int entradasPorTabla = memoriaData.entradasPorTabla;

    for (int i = 0; i < entradasPorTabla; i++) {
        int nroDeTabla2 = obtener_tabla_de_nivel_2(nroDeTabla1, i, memoriaData);
        for (int j = 0; j < entradasPorTabla; j++) {
            if (pagina_en_memoria(nroDeTabla2, j, memoriaData)) { //TODO y el bit de modificado, para no escribir si no se modifico. Ademas de añadir el retardo de swap/escribir en disco!
                int pagina = nroDeTabla2 * entradasPorTabla + j;
                int marco = obtener_marco(nroDeTabla2, j, memoriaData);
                log_trace(memoriaData.memoriaLogger, "La pagina %d esta en memoria", pagina);

                memcpy(memoriaData.inicio_archivo + memoriaData.tamanioPagina * pagina, memoriaData.memoriaPrincipal + marco * memoriaData.tamanioPagina, memoriaData.tamanioPagina);
                memset(memoriaData.memoriaPrincipal + marco * memoriaData.tamanioPagina, 0, memoriaData.tamanioPagina);
            }
        }
        limpiar_tabla_nivel_2(nroDeTabla2, memoriaData);
    }
    limpiar_tabla_nivel_1(nroDeTabla1, memoriaData);
    cerrar_archivo(memoriaData);
}

void despertar_proceso(uint32_t nroDeTabla1, t_memoria_data_holder memoriaData){
    int length = snprintf( NULL, 0, "%d", nroDeTabla1 );
    char* nroDeTabla1Str = malloc( length + 1 );
    snprintf( nroDeTabla1Str, length + 1, "%d", nroDeTabla1 );
    uint32_t tamanioNroDeTabla1 = *((uint32_t*) dictionary_remove(memoriaData.tablaSuspendidos, nroDeTabla1Str));
    free(nroDeTabla1Str);

    uint32_t indiceTablaNivel1 = obtener_tabla_libre_de_nivel_1(memoriaData);
    asignar_tabla_nivel_1_with_id(indiceTablaNivel1, nroDeTabla1, tamanioNroDeTabla1, memoriaData);
}
