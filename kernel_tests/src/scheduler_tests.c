#include "scheduler_tests.h"

#include <CUnit/Basic.h>
#include <commons/string.h>
#include <math.h>
#include <sys/time.h>
#include <unistd.h>

#include "common_utils.h"
#include "estado.h"
#include "pcb.h"
#include "scheduler.h"

static t_algoritmo elegir_pcb;

static t_estado* estadoReady = NULL;
static t_pcb* pcb1 = NULL;
static t_pcb* pcb2 = NULL;
static t_pcb* pcb3 = NULL;

static uint32_t size = 100;
static double estimacionInicial = 10000.0;
static double alfa = 0.5;

// @Before
void test_scheduler_setup(void) {
    estadoReady = estado_create(READY);
    pcb1 = pcb_create(1, size, estimacionInicial);
    pcb2 = pcb_create(2, size, estimacionInicial);
    pcb3 = pcb_create(3, size, estimacionInicial);
}

// @After
void test_scheduler_tear_down(void) {
    estado_destroy(estadoReady);
}

void test_planificar_segun_fifo(void) {
    elegir_pcb = segun_fifo;

    estado_encolar_pcb_atomic(estadoReady, pcb3);
    estado_encolar_pcb_atomic(estadoReady, pcb1);
    estado_encolar_pcb_atomic(estadoReady, pcb2);

    t_pcb* electo = elegir_pcb(estadoReady, alfa);
    CU_ASSERT_EQUAL(pcb_get_pid(electo), pcb_get_pid(pcb3));
    pcb_destroy(electo);

    electo = elegir_pcb(estadoReady, alfa);
    CU_ASSERT_EQUAL(pcb_get_pid(electo), pcb_get_pid(pcb1));
    pcb_destroy(electo);

    electo = elegir_pcb(estadoReady, alfa);
    CU_ASSERT_EQUAL(pcb_get_pid(electo), pcb_get_pid(pcb2));
    pcb_destroy(electo);
}

void test_empates_en_srt_degradan_en_un_fifo(void) {
    elegir_pcb = segun_srt;

    estado_encolar_pcb_atomic(estadoReady, pcb2);
    estado_encolar_pcb_atomic(estadoReady, pcb1);
    estado_encolar_pcb_atomic(estadoReady, pcb3);

    t_pcb* electo = elegir_pcb(estadoReady, alfa);
    CU_ASSERT_EQUAL(pcb_get_pid(electo), pcb_get_pid(pcb2));
    pcb_destroy(electo);

    electo = elegir_pcb(estadoReady, alfa);
    CU_ASSERT_EQUAL(pcb_get_pid(electo), pcb_get_pid(pcb1));
    pcb_destroy(electo);

    electo = elegir_pcb(estadoReady, alfa);
    CU_ASSERT_EQUAL(pcb_get_pid(electo), pcb_get_pid(pcb3));
    pcb_destroy(electo);
}

void test_se_elige_siempre_al_pcb_de_menor_rafaga_restante(void) {
    elegir_pcb = segun_srt;

    estado_encolar_pcb_atomic(estadoReady, pcb2);
    estado_encolar_pcb_atomic(estadoReady, pcb1);
    estado_encolar_pcb_atomic(estadoReady, pcb3);

    t_pcb* electo = elegir_pcb(estadoReady, alfa);
    CU_ASSERT_EQUAL(pcb_get_pid(electo), pcb_get_pid(pcb2));
    actualizar_pcb_por_bloqueo(electo, 2.0);
}

void just_a_test(void) {
    struct timespec start, end;
    clock_gettime(CLOCK_REALTIME, &start);
    intervalo_de_pausa(10500);
    clock_gettime(CLOCK_REALTIME, &end);

    uint64_t delta_us = (end.tv_sec - start.tv_sec) * 1000 + (end.tv_nsec - start.tv_nsec) / 1000000;
    char* str_delta_us = string_itoa(delta_us);
    str_delta_us[strlen(str_delta_us) - 1] = '0';
    uint64_t new_delta_us = atoi(str_delta_us);

    printf("\nElapsed time: %ld\n", delta_us);
    printf("Elapsed time (new): %ld\n", new_delta_us);

    free(str_delta_us);

    pcb_destroy(pcb1);
    pcb_destroy(pcb2);
    pcb_destroy(pcb3);
}
