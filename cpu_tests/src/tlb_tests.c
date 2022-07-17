#include "tlb_tests.h"

#include <CUnit/Basic.h>
#include <stdlib.h>

#include "tlb.h"

#define CANTIDAD_TLBS 2
#define CANTIDAD_ENTRADAS_TLB 4

static t_tlb* tlbFIFO = NULL;
static t_tlb* tlbLRU = NULL;
static t_tlb** tlbs = NULL;

// @Before
void test_tlb_setup(void) {
    tlbFIFO = tlb_create(CANTIDAD_ENTRADAS_TLB, "FIFO");
    tlbLRU = tlb_create(CANTIDAD_ENTRADAS_TLB, "LRU");
    tlbs = calloc(CANTIDAD_TLBS, sizeof(t_tlb*));
    tlbs[0] = tlbFIFO;
    tlbs[1] = tlbLRU;
}

// @After
void test_tlb_tear_down(void) {
    tlb_destroy(tlbFIFO);
    tlb_destroy(tlbLRU);
    free(tlbs);
}

static void __assert_entrada_tlb(t_tlb* tlb, uint32_t index, uint32_t numeroPagina, uint32_t marco, uint32_t instanteDeTiempo) {
    CU_ASSERT_EQUAL(entrada_tlb_get_numero_pagina(tlb, index), numeroPagina);
    CU_ASSERT_EQUAL(entrada_tlb_get_marco(tlb, index), marco);
    CU_ASSERT_EQUAL(entrada_tlb_get_instante_de_tiempo(tlb, index), instanteDeTiempo);
}

static void __assert_tlb_entries_all_minus_one(t_tlb* tlb) {
    for (int i = 0; i < tlb_get_cantidad_entradas_totales(tlb); i++) {
        __assert_entrada_tlb(tlb, i, -1, -1, -1);
    }
}

static void __assert_tlb_entries_all_zero(t_tlb* tlb) {
    for (int i = 0; i < tlb_get_cantidad_entradas_totales(tlb); i++) {
        __assert_entrada_tlb(tlb, i, 0, 0, 0);
    }
}

void test_tlb_create(void) {
    for (int i = 0; i < CANTIDAD_TLBS; i++) {
        t_tlb* tlb = tlbs[i];
        __assert_tlb_entries_all_zero(tlb);
    }
}

void test_tlb_flush(void) {
    for (int i = 0; i < CANTIDAD_TLBS; i++) {
        t_tlb* tlb = tlbs[i];
        tlb_flush(tlb);
        __assert_tlb_entries_all_minus_one(tlb);
    }
    // __tlb_print_entries(tlbFIFO);
}

void test_fifo(void) {
    // Flush entries at start
    tlb_flush(tlbFIFO);

    CU_ASSERT_EQUAL(tlb_get_marco(tlbFIFO, 0), -1);
    // __tlb_print_entries(tlbFIFO);
    tlb_registrar_entrada_en_tlb(tlbFIFO, 0, 0);
    __assert_entrada_tlb(tlbFIFO, 0, 0, 0, 0);
    CU_ASSERT_EQUAL(tlb_get_marco(tlbFIFO, 0), 0);
    // __tlb_print_entries(tlbFIFO);

    CU_ASSERT_EQUAL(tlb_get_marco(tlbFIFO, 1), -1);
    // __tlb_print_entries(tlbFIFO);
    tlb_registrar_entrada_en_tlb(tlbFIFO, 1, 1);
    __assert_entrada_tlb(tlbFIFO, 1, 1, 1, 1);
    CU_ASSERT_EQUAL(tlb_get_marco(tlbFIFO, 1), 1);
    // __tlb_print_entries(tlbFIFO);

    __assert_entrada_tlb(tlbFIFO, 0, 0, 0, 0);
    CU_ASSERT_EQUAL(tlb_get_marco(tlbFIFO, 0), 0);
    // __tlb_print_entries(tlbFIFO);

    __assert_entrada_tlb(tlbFIFO, 1, 1, 1, 1);
    CU_ASSERT_EQUAL(tlb_get_marco(tlbFIFO, 1), 1);
    // __tlb_print_entries(tlbFIFO);

    CU_ASSERT_EQUAL(tlb_get_marco(tlbFIFO, 2), -1);
    // __tlb_print_entries(tlbFIFO);
    tlb_registrar_entrada_en_tlb(tlbFIFO, 2, 2);
    __assert_entrada_tlb(tlbFIFO, 2, 2, 2, 2);
    CU_ASSERT_EQUAL(tlb_get_marco(tlbFIFO, 2), 2);
    // __tlb_print_entries(tlbFIFO);

    CU_ASSERT_EQUAL(tlb_get_marco(tlbFIFO, 3), -1);
    // __tlb_print_entries(tlbFIFO);
    tlb_registrar_entrada_en_tlb(tlbFIFO, 3, 3);
    __assert_entrada_tlb(tlbFIFO, 3, 3, 3, 3);
    CU_ASSERT_EQUAL(tlb_get_marco(tlbFIFO, 3), 3);
    // __tlb_print_entries(tlbFIFO);

    CU_ASSERT_EQUAL(tlb_get_marco(tlbFIFO, 4), -1);
    // __tlb_print_entries(tlbFIFO);
    tlb_registrar_entrada_en_tlb(tlbFIFO, 4, 4);
    __assert_entrada_tlb(tlbFIFO, 0, 4, 4, 4);
    CU_ASSERT_EQUAL(tlb_get_marco(tlbFIFO, 4), 4);
    // __tlb_print_entries(tlbFIFO);

    CU_ASSERT_EQUAL(tlb_get_marco(tlbFIFO, 5), -1);
    // __tlb_print_entries(tlbFIFO);
    tlb_registrar_entrada_en_tlb(tlbFIFO, 5, 5);
    __assert_entrada_tlb(tlbFIFO, 1, 5, 5, 5);
    CU_ASSERT_EQUAL(tlb_get_marco(tlbFIFO, 5), 5);
    // __tlb_print_entries(tlbFIFO);

    CU_ASSERT_EQUAL(tlb_get_marco(tlbFIFO, 6), -1);
    // __tlb_print_entries(tlbFIFO);
    tlb_registrar_entrada_en_tlb(tlbFIFO, 6, 6);
    __assert_entrada_tlb(tlbFIFO, 2, 6, 6, 6);
    CU_ASSERT_EQUAL(tlb_get_marco(tlbFIFO, 6), 6);
    // __tlb_print_entries(tlbFIFO);

    CU_ASSERT_EQUAL(tlb_get_marco(tlbFIFO, 7), -1);
    // __tlb_print_entries(tlbFIFO);
    tlb_registrar_entrada_en_tlb(tlbFIFO, 7, 7);
    __assert_entrada_tlb(tlbFIFO, 3, 7, 7, 7);
    CU_ASSERT_EQUAL(tlb_get_marco(tlbFIFO, 7), 7);
    // __tlb_print_entries(tlbFIFO);

    CU_ASSERT_EQUAL(tlb_get_marco(tlbFIFO, 8), -1);
    // __tlb_print_entries(tlbFIFO);
    tlb_registrar_entrada_en_tlb(tlbFIFO, 8, 8);
    __assert_entrada_tlb(tlbFIFO, 0, 8, 8, 8);
    CU_ASSERT_EQUAL(tlb_get_marco(tlbFIFO, 8), 8);
    // __tlb_print_entries(tlbFIFO);

    // Flush entries
    tlb_flush(tlbFIFO);
    __assert_tlb_entries_all_minus_one(tlbFIFO);
    // __tlb_print_entries(tlbFIFO);

    CU_ASSERT_EQUAL(tlb_get_marco(tlbFIFO, 0), -1);
    // __tlb_print_entries(tlbFIFO);
    tlb_registrar_entrada_en_tlb(tlbFIFO, 0, 0);
    __assert_entrada_tlb(tlbFIFO, 0, 0, 0, 9);
    CU_ASSERT_EQUAL(tlb_get_marco(tlbFIFO, 0), 0);
    // __tlb_print_entries(tlbFIFO);

    CU_ASSERT_EQUAL(tlb_get_marco(tlbFIFO, 1), -1);
    // __tlb_print_entries(tlbFIFO);
    tlb_registrar_entrada_en_tlb(tlbFIFO, 1, 1);
    __assert_entrada_tlb(tlbFIFO, 1, 1, 1, 10);
    CU_ASSERT_EQUAL(tlb_get_marco(tlbFIFO, 1), 1);
    // __tlb_print_entries(tlbFIFO);

    __assert_entrada_tlb(tlbFIFO, 0, 0, 0, 9);
    CU_ASSERT_EQUAL(tlb_get_marco(tlbFIFO, 0), 0);
    // __tlb_print_entries(tlbFIFO);

    __assert_entrada_tlb(tlbFIFO, 1, 1, 1, 10);
    CU_ASSERT_EQUAL(tlb_get_marco(tlbFIFO, 1), 1);
    // __tlb_print_entries(tlbFIFO);

    CU_ASSERT_EQUAL(tlb_get_marco(tlbFIFO, 2), -1);
    // __tlb_print_entries(tlbFIFO);
    tlb_registrar_entrada_en_tlb(tlbFIFO, 2, 2);
    __assert_entrada_tlb(tlbFIFO, 2, 2, 2, 11);
    CU_ASSERT_EQUAL(tlb_get_marco(tlbFIFO, 2), 2);
    // __tlb_print_entries(tlbFIFO);

    CU_ASSERT_EQUAL(tlb_get_marco(tlbFIFO, 3), -1);
    // __tlb_print_entries(tlbFIFO);
    tlb_registrar_entrada_en_tlb(tlbFIFO, 3, 3);
    __assert_entrada_tlb(tlbFIFO, 3, 3, 3, 12);
    CU_ASSERT_EQUAL(tlb_get_marco(tlbFIFO, 3), 3);
    // __tlb_print_entries(tlbFIFO);

    CU_ASSERT_EQUAL(tlb_get_marco(tlbFIFO, 4), -1);
    // __tlb_print_entries(tlbFIFO);
    tlb_registrar_entrada_en_tlb(tlbFIFO, 4, 4);
    __assert_entrada_tlb(tlbFIFO, 0, 4, 4, 13);
    CU_ASSERT_EQUAL(tlb_get_marco(tlbFIFO, 4), 4);
    // __tlb_print_entries(tlbFIFO);
}

void test_lru(void) {
    // Flush entries at start
    tlb_flush(tlbLRU);

    CU_ASSERT_EQUAL(tlb_get_marco(tlbLRU, 0), -1);
    // __tlb_print_entries(tlbLRU);
    tlb_registrar_entrada_en_tlb(tlbLRU, 0, 0);
    __assert_entrada_tlb(tlbLRU, 0, 0, 0, 0);
    CU_ASSERT_EQUAL(tlb_get_marco(tlbLRU, 0), 0);
    __assert_entrada_tlb(tlbLRU, 0, 0, 0, 1);
    // __tlb_print_entries(tlbLRU);

    CU_ASSERT_EQUAL(tlb_get_marco(tlbLRU, 1), -1);
    // __tlb_print_entries(tlbLRU);
    tlb_registrar_entrada_en_tlb(tlbLRU, 1, 1);
    __assert_entrada_tlb(tlbLRU, 1, 1, 1, 2);
    CU_ASSERT_EQUAL(tlb_get_marco(tlbLRU, 1), 1);
    __assert_entrada_tlb(tlbLRU, 1, 1, 1, 3);
    // __tlb_print_entries(tlbLRU);

    CU_ASSERT_EQUAL(tlb_get_marco(tlbLRU, 2), -1);
    // __tlb_print_entries(tlbLRU);
    tlb_registrar_entrada_en_tlb(tlbLRU, 2, 2);
    __assert_entrada_tlb(tlbLRU, 2, 2, 2, 4);
    CU_ASSERT_EQUAL(tlb_get_marco(tlbLRU, 2), 2);
    __assert_entrada_tlb(tlbLRU, 2, 2, 2, 5);
    // __tlb_print_entries(tlbLRU);

    CU_ASSERT_EQUAL(tlb_get_marco(tlbLRU, 3), -1);
    // __tlb_print_entries(tlbLRU);
    tlb_registrar_entrada_en_tlb(tlbLRU, 3, 3);
    __assert_entrada_tlb(tlbLRU, 3, 3, 3, 6);
    CU_ASSERT_EQUAL(tlb_get_marco(tlbLRU, 3), 3);
    __assert_entrada_tlb(tlbLRU, 3, 3, 3, 7);
    // __tlb_print_entries(tlbLRU);

    CU_ASSERT_EQUAL(tlb_get_marco(tlbLRU, 4), -1);
    // __tlb_print_entries(tlbLRU);
    tlb_registrar_entrada_en_tlb(tlbLRU, 4, 4);
    __assert_entrada_tlb(tlbLRU, 0, 4, 4, 8);
    CU_ASSERT_EQUAL(tlb_get_marco(tlbLRU, 4), 4);
    __assert_entrada_tlb(tlbLRU, 0, 4, 4, 9);
    // __tlb_print_entries(tlbLRU);

    CU_ASSERT_EQUAL(tlb_get_marco(tlbLRU, 5), -1);
    // __tlb_print_entries(tlbLRU);
    tlb_registrar_entrada_en_tlb(tlbLRU, 5, 5);
    __assert_entrada_tlb(tlbLRU, 1, 5, 5, 10);
    CU_ASSERT_EQUAL(tlb_get_marco(tlbLRU, 5), 5);
    __assert_entrada_tlb(tlbLRU, 1, 5, 5, 11);
    // __tlb_print_entries(tlbLRU);

    CU_ASSERT_EQUAL(tlb_get_marco(tlbLRU, 6), -1);
    // __tlb_print_entries(tlbLRU);
    tlb_registrar_entrada_en_tlb(tlbLRU, 6, 6);
    __assert_entrada_tlb(tlbLRU, 2, 6, 6, 12);
    CU_ASSERT_EQUAL(tlb_get_marco(tlbLRU, 6), 6);
    __assert_entrada_tlb(tlbLRU, 2, 6, 6, 13);
    // __tlb_print_entries(tlbLRU);

    CU_ASSERT_EQUAL(tlb_get_marco(tlbLRU, 7), -1);
    // __tlb_print_entries(tlbLRU);
    tlb_registrar_entrada_en_tlb(tlbLRU, 7, 7);
    __assert_entrada_tlb(tlbLRU, 3, 7, 7, 14);
    CU_ASSERT_EQUAL(tlb_get_marco(tlbLRU, 7), 7);
    __assert_entrada_tlb(tlbLRU, 3, 7, 7, 15);
    // __tlb_print_entries(tlbLRU);

    CU_ASSERT_EQUAL(tlb_get_marco(tlbLRU, 8), -1);
    // __tlb_print_entries(tlbLRU);
    tlb_registrar_entrada_en_tlb(tlbLRU, 8, 8);
    __assert_entrada_tlb(tlbLRU, 0, 8, 8, 16);
    CU_ASSERT_EQUAL(tlb_get_marco(tlbLRU, 8), 8);
    __assert_entrada_tlb(tlbLRU, 0, 8, 8, 17);
    // __tlb_print_entries(tlbLRU);

    CU_ASSERT_EQUAL(tlb_get_marco(tlbLRU, 2), -1);
    // __tlb_print_entries(tlbLRU);
    tlb_registrar_entrada_en_tlb(tlbLRU, 2, 2);
    __assert_entrada_tlb(tlbLRU, 1, 2, 2, 18);
    CU_ASSERT_EQUAL(tlb_get_marco(tlbLRU, 2), 2);
    __assert_entrada_tlb(tlbLRU, 1, 2, 2, 19);
    // __tlb_print_entries(tlbLRU);

    CU_ASSERT_EQUAL(tlb_get_marco(tlbLRU, 2), 2);
    __assert_entrada_tlb(tlbLRU, 1, 2, 2, 20);
    // __tlb_print_entries(tlbLRU);

    CU_ASSERT_EQUAL(tlb_get_marco(tlbLRU, 2), 2);
    __assert_entrada_tlb(tlbLRU, 1, 2, 2, 21);
    // __tlb_print_entries(tlbLRU);

    CU_ASSERT_EQUAL(tlb_get_marco(tlbLRU, 7), 7);
    __assert_entrada_tlb(tlbLRU, 3, 7, 7, 22);
    // __tlb_print_entries(tlbLRU);

    CU_ASSERT_EQUAL(tlb_get_marco(tlbLRU, 0), -1);
    // __tlb_print_entries(tlbLRU);
    tlb_registrar_entrada_en_tlb(tlbLRU, 0, 0);
    __assert_entrada_tlb(tlbLRU, 2, 0, 0, 23);
    CU_ASSERT_EQUAL(tlb_get_marco(tlbLRU, 0), 0);
    __assert_entrada_tlb(tlbLRU, 2, 0, 0, 24);
    // __tlb_print_entries(tlbLRU);

    // Flush entries
    tlb_flush(tlbLRU);
    __assert_tlb_entries_all_minus_one(tlbLRU);
    // __tlb_print_entries(tlbLRU);

    CU_ASSERT_EQUAL(tlb_get_marco(tlbLRU, 4), -1);
    // __tlb_print_entries(tlbLRU);
    tlb_registrar_entrada_en_tlb(tlbLRU, 4, 0);
    __assert_entrada_tlb(tlbLRU, 0, 4, 0, 25);
    CU_ASSERT_EQUAL(tlb_get_marco(tlbLRU, 4), 0);
    __assert_entrada_tlb(tlbLRU, 0, 4, 0, 26);
    // __tlb_print_entries(tlbLRU);

    CU_ASSERT_EQUAL(tlb_get_marco(tlbLRU, 2), -1);
    // __tlb_print_entries(tlbLRU);
    tlb_registrar_entrada_en_tlb(tlbLRU, 2, 1);
    __assert_entrada_tlb(tlbLRU, 1, 2, 1, 27);
    CU_ASSERT_EQUAL(tlb_get_marco(tlbLRU, 2), 1);
    __assert_entrada_tlb(tlbLRU, 1, 2, 1, 28);
    // __tlb_print_entries(tlbLRU);

    CU_ASSERT_EQUAL(tlb_get_marco(tlbLRU, 2), 1);
    __assert_entrada_tlb(tlbLRU, 1, 2, 1, 29);
    // __tlb_print_entries(tlbLRU);

    CU_ASSERT_EQUAL(tlb_get_marco(tlbLRU, 6), -1);
    // __tlb_print_entries(tlbLRU);
    tlb_registrar_entrada_en_tlb(tlbLRU, 6, 2);
    __assert_entrada_tlb(tlbLRU, 2, 6, 2, 30);
    CU_ASSERT_EQUAL(tlb_get_marco(tlbLRU, 6), 2);
    __assert_entrada_tlb(tlbLRU, 2, 6, 2, 31);
    // __tlb_print_entries(tlbLRU);

    CU_ASSERT_EQUAL(tlb_get_marco(tlbLRU, 420), -1);
    // __tlb_print_entries(tlbLRU);
    tlb_registrar_entrada_en_tlb(tlbLRU, 420, 3);
    __assert_entrada_tlb(tlbLRU, 3, 420, 3, 32);
    CU_ASSERT_EQUAL(tlb_get_marco(tlbLRU, 420), 3);
    __assert_entrada_tlb(tlbLRU, 3, 420, 3, 33);
    // __tlb_print_entries(tlbLRU);

    CU_ASSERT_EQUAL(tlb_get_marco(tlbLRU, 7), -1);
    // __tlb_print_entries(tlbLRU);
    tlb_registrar_entrada_en_tlb(tlbLRU, 7, 4);
    __assert_entrada_tlb(tlbLRU, 0, 7, 4, 34);
    CU_ASSERT_EQUAL(tlb_get_marco(tlbLRU, 7), 4);
    __assert_entrada_tlb(tlbLRU, 0, 7, 4, 35);
    // __tlb_print_entries(tlbLRU);

    // Flush entries
    tlb_flush(tlbLRU);
    __assert_tlb_entries_all_minus_one(tlbLRU);
    // __tlb_print_entries(tlbLRU);
}
