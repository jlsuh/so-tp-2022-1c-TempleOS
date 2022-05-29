#include <CUnit/Basic.h>
#include <stdbool.h>
#include <stdio.h>

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
        TEST_FUNC(test_planificar_segun_fifo),
        TEST_FUNC(test_empates_en_srt_degradan_en_un_fifo),
        TEST_FUNC(just_a_test),
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
