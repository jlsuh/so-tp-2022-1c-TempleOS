#include <CUnit/Basic.h>
#include <stdbool.h>
#include <stdio.h>

#include "consola_parser_tests.h"
#include "testing_utils.h"

static void consola_tests(void) {
    CU_pSuite consolaParserSuite = CU_add_suite_with_setup_and_teardown(
        "consola_parser.h Test Suite",
        NULL,
        NULL,
        test_consola_parser_setup,
        test_consola_parser_tear_down);
    t_test_case consolaParserTestCases[] = {
        TEST_FUNC(test_es_posible_parsear_un_archivo_valido_de_instrucciones),
        TEST_FUNC(test_es_posible_parsear_un_archivo_valido_de_instrucciones_con_lineas_vacias),
        TEST_FUNC(test_no_es_posible_parsear_un_archivo_invalido_de_instrucciones_debido_a_la_primer_instruccion),
        TEST_FUNC(test_no_es_posible_parsear_un_archivo_invalido_de_instrucciones_debido_a_la_ultima_instruccion),
        TEST_FUNC(test_no_es_posible_parsear_un_archivo_invalido_de_instrucciones_con_lineas_vacias_debido_a_la_ultima_instruccion),
    };
    ADD_TEST_CASES_TO_SUITE(consolaParserSuite, consolaParserTestCases);
}

int main(int argc, char* argv[]) {
    CU_initialize_registry();
    CU_basic_set_mode(CU_BRM_VERBOSE);

    consola_tests();

    CU_basic_run_tests();
    CU_cleanup_registry();

    return CU_get_error();
}
