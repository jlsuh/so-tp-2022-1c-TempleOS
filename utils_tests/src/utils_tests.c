#include <stdio.h>
#include <stdbool.h>
#include <CUnit/Basic.h>

#include "testing_utils.h"

#include "buffer_tests.h"

static void buffer_tests(void) {
    CU_pSuite bufferSuite = CU_add_suite_with_setup_and_teardown(
        "buffer.h Test Suite",
        NULL,
        NULL,
        test_buffer_setup,
        test_buffer_tear_down);
    t_test_case bufferTestCases[] = {
        TEST_FUNC(test_es_posible_desempaquetar_cualquier_valor),
        TEST_FUNC(test_es_posible_empaquetar_cualquier_valor),
        TEST_FUNC(test_es_posible_empaquetar_y_desempaquetar_un_string),
    };
    ADD_TEST_CASES_TO_SUITE(bufferSuite, bufferTestCases);
}

int main(int argc, char* argv[]) {
    CU_initialize_registry();
    CU_basic_set_mode(CU_BRM_VERBOSE);

    // Llamadas a funciones que agreguen los tests a sus respectivos suites
    buffer_tests();

    CU_basic_run_tests();
    CU_cleanup_registry();

    return CU_get_error();
}
