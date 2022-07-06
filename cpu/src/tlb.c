#include "tlb.h"

#include <commons/collections/list.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern t_tlb* tlb;

typedef struct {
    int numeroPagina;
    int marco;
    int instanteDeTiempo;
} t_entrada_tlb;

struct t_tlb {
    uint32_t cantidadEntradasTotales;
    uint32_t cantidadEntradasLibres;
    uint32_t ultimoInstanteDeTiempo;
    char* algoritmoReemplazo;
    t_entrada_tlb* entradas;
    void (*actualizar_ultima_referencia)(t_tlb*, int indiceEntrada);
};

static void __flush_entrada(t_tlb* self, int entrada) {
    self->entradas[entrada].numeroPagina = -1;
    self->entradas[entrada].marco = -1;
    self->entradas[entrada].instanteDeTiempo = -1;
}

static int __obtener_y_actualizar_ultimo_instante(t_tlb* self) {
    return self->ultimoInstanteDeTiempo++;
}

static void __actualizar_instante_fifo(t_tlb* self, int indiceEntrada) {}

static void __actualizar_instante_lru(t_tlb* self, int indiceEntrada) {
    self->entradas[indiceEntrada].instanteDeTiempo = __obtener_y_actualizar_ultimo_instante(self);
}

static bool __es_reeemplazo_fifo(t_tlb* tlb) {
    return strcmp(tlb->algoritmoReemplazo, "FIFO") == 0;
}

static bool __es_reemplazo_lru(t_tlb* tlb) {
    return strcmp(tlb->algoritmoReemplazo, "LRU") == 0;
}

static int __elegir_entrada_de_menor_instante_de_tiempo(t_tlb* self) {
    int entradaTemp = 0;
    for (int i = 1; i < self->cantidadEntradasTotales; i++) {
        if (self->entradas[i].instanteDeTiempo < self->entradas[entradaTemp].instanteDeTiempo) {
            entradaTemp = i;
        }
    }
    return entradaTemp;
}

static bool __es_entrada_libre(t_tlb* self, int entrada) {
    return self->entradas[entrada].numeroPagina == -1 &&
           self->entradas[entrada].marco == -1;
}

static int __obtener_primer_entrada_libre(t_tlb* self) {
    for (int i = 0; i < self->cantidadEntradasTotales; i++) {
        if (__es_entrada_libre(self, i)) {
            return i;
        }
    }
    return -1;
}

void tlb_registrar_entrada_en_tlb(t_tlb* self, uint32_t numeroPagina, uint32_t marco) {
    int indiceEntrada = -1;
    if (self->cantidadEntradasLibres > 0) {
        indiceEntrada = __obtener_primer_entrada_libre(self);
        self->cantidadEntradasLibres -= 1;
    } else {
        indiceEntrada = __elegir_entrada_de_menor_instante_de_tiempo(self);
    }
    self->entradas[indiceEntrada].numeroPagina = numeroPagina;
    self->entradas[indiceEntrada].marco = marco;
    self->entradas[indiceEntrada].instanteDeTiempo = __obtener_y_actualizar_ultimo_instante(self);
}

t_tlb* tlb_create(uint32_t cantidadDeEntradas, char* algoritmoReemplazo) {
    t_tlb* self = malloc(sizeof(*self));
    self->cantidadEntradasTotales = cantidadDeEntradas;
    self->cantidadEntradasLibres = cantidadDeEntradas;
    self->ultimoInstanteDeTiempo = 0;
    self->algoritmoReemplazo = strdup(algoritmoReemplazo);
    self->entradas = calloc(self->cantidadEntradasTotales, sizeof(*self->entradas));
    if (__es_reeemplazo_fifo(self)) {
        self->actualizar_ultima_referencia = __actualizar_instante_fifo;
    } else if (__es_reemplazo_lru(self)) {
        self->actualizar_ultima_referencia = __actualizar_instante_lru;
    } else {
        printf("Error: Algoritmo de reemplazo desconocido: %s\n", self->algoritmoReemplazo);
        exit(-1);
    }
    return self;
}

void tlb_flush(t_tlb* self) {
    for (int i = 0; i < self->cantidadEntradasTotales; i++) {
        __flush_entrada(self, i);
    }
    self->cantidadEntradasLibres = self->cantidadEntradasTotales;
}

int tlb_get_marco(t_tlb* self, uint32_t numeroPagina) {
    for (int i = 0; i < self->cantidadEntradasTotales; i++) {
        if (self->entradas[i].numeroPagina == numeroPagina) {
            self->actualizar_ultima_referencia(self, i);
            return self->entradas[i].marco;
        }
    }
    return -1;
}