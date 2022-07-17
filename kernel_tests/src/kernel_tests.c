#include <CUnit/Basic.h>

#include "scheduler_tests.h"
#include "testing_utils.h"

static void scheduler_tests(void) {
    CU_pSuite schedulerSuite = CU_add_suite_with_setup_and_teardown(
        "scheduler.h Test Suite",
        NULL,
        NULL,
        test_scheduler_setup,
        test_scheduler_tear_down);
    t_test_case schedulerTestCases[] = {
        TEST_FUNC(test_actualizar_un_pcb_por_bloqueo_actualiza_estimacion_actual),
        TEST_FUNC(test_actualizar_un_pcb_por_bloqueo_actualiza_real_anterior_como_sumatoria_de_reales_ejecutados_hasta_ahora),
        TEST_FUNC(test_actualizar_un_pcb_por_bloqueo_actualiza_reales_ejecutados_hasta_ahora_en_cero),
        TEST_FUNC(test_actualizar_un_pcb_por_desalojo_actualiza_reales_ejecutados_hasta_ahora),
        TEST_FUNC(test_actualizar_un_pcb_por_desalojo_no_actualiza_estimacion_actual),
        TEST_FUNC(test_actualizar_un_pcb_por_varios_desalojos_actualiza_reales_ejecutados_hasta_ahora_como_la_sumatoria),
        TEST_FUNC(test_actualizar_un_pcb_por_varios_desalojos_disminuye_rafaga_restante),
        TEST_FUNC(test_empates_en_srt_degradan_en_un_fifo),
        TEST_FUNC(test_planificar_segun_fifo),
    };
    ADD_TEST_CASES_TO_SUITE(schedulerSuite, schedulerTestCases);
}

int main(int argc, char* argv[]) {
    CU_initialize_registry();
    CU_basic_set_mode(CU_BRM_VERBOSE);

    scheduler_tests();

    CU_basic_run_tests();
    CU_cleanup_registry();

    return CU_get_error();
}
