#include "scheduler_tests.h"

#include <CUnit/Basic.h>

#include "scheduler.h"

static t_dispatch_handler elegir_pcb;
static t_onBlocked_handler actualizar_pcb_por_bloqueo;

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

////////////////////////////// FIFO //////////////////////////////
void test_planificar_segun_fifo(void) {
    elegir_pcb = elegir_pcb_segun_fifo;

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

////////////////////////////// SRT //////////////////////////////
void test_empates_en_srt_degradan_en_un_fifo(void) {
    elegir_pcb = elegir_pcb_segun_srt;

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

//////////////////// Desalojos ////////////////////
void test_actualizar_un_pcb_por_desalojo_no_actualiza_estimacion_actual(void) {
    elegir_pcb = elegir_pcb_segun_srt;

    estado_encolar_pcb_atomic(estadoReady, pcb2);
    estado_encolar_pcb_atomic(estadoReady, pcb1);
    estado_encolar_pcb_atomic(estadoReady, pcb3);

    t_pcb* electo = elegir_pcb(estadoReady, alfa);
    CU_ASSERT_EQUAL(pcb_get_pid(electo), pcb_get_pid(pcb2));

    actualizar_pcb_por_desalojo(electo, 2000);
    CU_ASSERT_EQUAL(pcb_get_estimacion_actual(electo), estimacionInicial);

    pcb_destroy(electo);
}

void test_actualizar_un_pcb_por_desalojo_actualiza_reales_ejecutados_hasta_ahora(void) {
    elegir_pcb = elegir_pcb_segun_srt;

    estado_encolar_pcb_atomic(estadoReady, pcb1);
    estado_encolar_pcb_atomic(estadoReady, pcb2);
    estado_encolar_pcb_atomic(estadoReady, pcb3);

    t_pcb* electo = elegir_pcb(estadoReady, alfa);
    CU_ASSERT_EQUAL(pcb_get_pid(electo), pcb_get_pid(pcb1));

    actualizar_pcb_por_desalojo(electo, 3043);
    CU_ASSERT_EQUAL(pcb_get_reales_ejecutados_hasta_ahora(electo), 3043);

    pcb_destroy(electo);
}

void test_actualizar_un_pcb_por_varios_desalojos_actualiza_reales_ejecutados_hasta_ahora_como_la_sumatoria(void) {
    elegir_pcb = elegir_pcb_segun_srt;

    estado_encolar_pcb_atomic(estadoReady, pcb1);
    estado_encolar_pcb_atomic(estadoReady, pcb2);
    estado_encolar_pcb_atomic(estadoReady, pcb3);

    t_pcb* electo = elegir_pcb(estadoReady, alfa);
    CU_ASSERT_EQUAL(pcb_get_pid(electo), pcb_get_pid(pcb1));

    actualizar_pcb_por_desalojo(electo, 2011);
    CU_ASSERT_EQUAL(pcb_get_reales_ejecutados_hasta_ahora(electo), 2011);

    estado_encolar_pcb_atomic(estadoReady, electo);

    electo = elegir_pcb(estadoReady, alfa);
    CU_ASSERT_EQUAL(pcb_get_pid(electo), pcb_get_pid(pcb1));

    actualizar_pcb_por_desalojo(electo, 1042);
    CU_ASSERT_EQUAL(pcb_get_reales_ejecutados_hasta_ahora(electo), 2011 + 1042);

    pcb_destroy(electo);
}

void test_actualizar_un_pcb_por_varios_desalojos_disminuye_rafaga_restante(void) {
    elegir_pcb = elegir_pcb_segun_srt;

    estado_encolar_pcb_atomic(estadoReady, pcb1);
    estado_encolar_pcb_atomic(estadoReady, pcb2);
    estado_encolar_pcb_atomic(estadoReady, pcb3);

    t_pcb* electo = elegir_pcb(estadoReady, alfa);
    CU_ASSERT_EQUAL(pcb_get_pid(electo), pcb_get_pid(pcb1));

    actualizar_pcb_por_desalojo(electo, 2011);
    estado_encolar_pcb_atomic(estadoReady, electo);

    electo = elegir_pcb(estadoReady, alfa);
    CU_ASSERT_EQUAL(pcb_get_pid(electo), pcb_get_pid(pcb1));

    actualizar_pcb_por_desalojo(electo, 1042);
    CU_ASSERT_EQUAL(calcular_estimacion_restante(electo), estimacionInicial - 2011 - 1042);

    pcb_destroy(electo);
}

//////////////////// Bloqueos ////////////////////
void test_actualizar_un_pcb_por_bloqueo_actualiza_real_anterior_como_sumatoria_de_reales_ejecutados_hasta_ahora(void) {
    elegir_pcb = elegir_pcb_segun_srt;
    actualizar_pcb_por_bloqueo = actualizar_pcb_por_bloqueo_segun_srt;

    estado_encolar_pcb_atomic(estadoReady, pcb3);
    estado_encolar_pcb_atomic(estadoReady, pcb1);
    estado_encolar_pcb_atomic(estadoReady, pcb2);

    t_pcb* electo = elegir_pcb(estadoReady, alfa);
    CU_ASSERT_EQUAL(pcb_get_pid(electo), pcb_get_pid(pcb3));

    t_pcb* pcb4 = pcb_create(4, size, estimacionInicial);
    actualizar_pcb_por_desalojo(electo, 3043);
    estado_encolar_pcb_atomic(estadoReady, electo);
    estado_encolar_pcb_atomic(estadoReady, pcb4);

    electo = elegir_pcb(estadoReady, alfa);
    CU_ASSERT_EQUAL(pcb_get_pid(electo), pcb_get_pid(pcb3));

    actualizar_pcb_por_desalojo(electo, 4024);
    estado_encolar_pcb_atomic(estadoReady, electo);

    electo = elegir_pcb(estadoReady, alfa);
    CU_ASSERT_EQUAL(pcb_get_pid(electo), pcb_get_pid(pcb3));

    actualizar_pcb_por_bloqueo(electo, 1032, alfa);
    CU_ASSERT_EQUAL(pcb_get_real_anterior(electo), 3043 + 4024 + 1032);

    pcb_destroy(electo);
}

void test_actualizar_un_pcb_por_bloqueo_actualiza_reales_ejecutados_hasta_ahora_en_cero(void) {
    elegir_pcb = elegir_pcb_segun_srt;
    actualizar_pcb_por_bloqueo = actualizar_pcb_por_bloqueo_segun_srt;

    estado_encolar_pcb_atomic(estadoReady, pcb3);
    estado_encolar_pcb_atomic(estadoReady, pcb1);
    estado_encolar_pcb_atomic(estadoReady, pcb2);

    t_pcb* electo = elegir_pcb(estadoReady, alfa);
    CU_ASSERT_EQUAL(pcb_get_pid(electo), pcb_get_pid(pcb3));

    t_pcb* pcb4 = pcb_create(4, size, estimacionInicial);
    actualizar_pcb_por_desalojo(electo, 3043);
    estado_encolar_pcb_atomic(estadoReady, electo);
    estado_encolar_pcb_atomic(estadoReady, pcb4);

    electo = elegir_pcb(estadoReady, alfa);
    CU_ASSERT_EQUAL(pcb_get_pid(electo), pcb_get_pid(pcb3));

    actualizar_pcb_por_desalojo(electo, 4024);
    estado_encolar_pcb_atomic(estadoReady, electo);

    electo = elegir_pcb(estadoReady, alfa);
    CU_ASSERT_EQUAL(pcb_get_pid(electo), pcb_get_pid(pcb3));

    actualizar_pcb_por_bloqueo(electo, 1032, alfa);
    CU_ASSERT_EQUAL(pcb_get_reales_ejecutados_hasta_ahora(electo), 0);

    pcb_destroy(electo);
}

void test_actualizar_un_pcb_por_bloqueo_actualiza_estimacion_actual(void) {
    elegir_pcb = elegir_pcb_segun_srt;
    actualizar_pcb_por_bloqueo = actualizar_pcb_por_bloqueo_segun_srt;

    estado_encolar_pcb_atomic(estadoReady, pcb3);
    estado_encolar_pcb_atomic(estadoReady, pcb1);
    estado_encolar_pcb_atomic(estadoReady, pcb2);

    t_pcb* electo = elegir_pcb(estadoReady, alfa);
    CU_ASSERT_EQUAL(pcb_get_pid(electo), pcb_get_pid(pcb3));

    actualizar_pcb_por_desalojo(electo, 3043);
    estado_encolar_pcb_atomic(estadoReady, electo);

    electo = elegir_pcb(estadoReady, alfa);
    CU_ASSERT_EQUAL(pcb_get_pid(electo), pcb_get_pid(pcb3));

    actualizar_pcb_por_desalojo(electo, 4024);
    estado_encolar_pcb_atomic(estadoReady, electo);

    electo = elegir_pcb(estadoReady, alfa);
    CU_ASSERT_EQUAL(pcb_get_pid(electo), pcb_get_pid(pcb3));

    double estimacionAntesDeBloquearse = pcb_get_estimacion_actual(electo);
    double realAntesDeBloquearse = pcb_get_reales_ejecutados_hasta_ahora(electo) + 1032;
    actualizar_pcb_por_bloqueo(electo, 1032, alfa);

    double actual = pcb_get_estimacion_actual(electo);
    double expected = alfa * realAntesDeBloquearse + (1 - alfa) * estimacionAntesDeBloquearse;

    CU_ASSERT_EQUAL(actual, expected);

    pcb_destroy(electo);
}
