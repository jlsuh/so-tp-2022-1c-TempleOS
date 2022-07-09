#include <CUnit/Basic.h>
#include <stdbool.h>
#include <stdio.h>

#include "buffer_tests.h"
#include "misc_tests.h"
#include "stream_connections_tests.h"
#include "testing_utils.h"

static void misc_tests(void) {
    CU_pSuite miscSuite = CU_add_suite_with_setup_and_teardown(
        "Misc Tests Suite",
        NULL,
        NULL,
        NULL,
        NULL);
    t_test_case miscTestCases[] = {
        TEST_FUNC(dictionary_tests),
    };
    ADD_TEST_CASES_TO_SUITE(miscSuite, miscTestCases);
}

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

static void stream_connections_tests(void) {
    CU_pSuite streamConexionesSuite = CU_add_suite_with_setup_and_teardown(
        "stream.h & conexiones.h Test Suite",
        NULL,
        NULL,
        test_stream_conexiones_setup,
        test_stream_conexiones_tear_down);
    t_test_case streamConexionesTestCases[] = {
        TEST_FUNC(test_es_posible_establecer_una_conexion),
        TEST_FUNC(test_es_posible_serializar_un_string_enviarlo_y_deserializarlo),
    };
    ADD_TEST_CASES_TO_SUITE(streamConexionesSuite, streamConexionesTestCases);
}

int main(int argc, char* argv[]) {
    CU_initialize_registry();
    CU_basic_set_mode(CU_BRM_VERBOSE);

    // Llamadas a funciones que agreguen los tests a sus respectivos suites
    misc_tests();
    buffer_tests();
    stream_connections_tests();

    CU_basic_run_tests();
    CU_cleanup_registry();

    return CU_get_error();
}
