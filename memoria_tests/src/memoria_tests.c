#include <stdio.h>
#include <stdbool.h>
#include <CUnit/Basic.h>

#include "memoria_parser_test.h"
#include "testing_utils.h"

static void memoria_tests(void) {
    CU_pSuite memoriaParserSuite = CU_add_suite_with_setup_and_teardown(
        "memoria_parser.h Test Suite",
        NULL,
        NULL,
        test_memoria_parser_setup,
        test_memoria_parser_tear_down);
    t_test_case memoriaParserTestCases[] = {
        TEST_FUNC(test_se_crea_correctamente),
    };
    ADD_TEST_CASES_TO_SUITE(memoriaParserSuite, memoriaParserTestCases);
}

int main(int argc, char* argv[]) {
    CU_initialize_registry();
    CU_basic_set_mode(CU_BRM_VERBOSE);

    memoria_tests();

    CU_basic_run_tests();
    CU_cleanup_registry();

    return CU_get_error();
}