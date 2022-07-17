#include <CUnit/Basic.h>

#include "testing_utils.h"
#include "tlb_tests.h"

static void tlb_tests(void) {
    CU_pSuite tlbSuite = CU_add_suite_with_setup_and_teardown(
        "tlb.h Test Suite",
        NULL,
        NULL,
        test_tlb_setup,
        test_tlb_tear_down);
    t_test_case tlbTestCases[] = {
        TEST_FUNC(test_fifo),
        TEST_FUNC(test_lru),
        TEST_FUNC(test_tlb_create),
        TEST_FUNC(test_tlb_flush),
    };
    ADD_TEST_CASES_TO_SUITE(tlbSuite, tlbTestCases);
}

int main(int argc, char* argv[]) {
    CU_initialize_registry();
    CU_basic_set_mode(CU_BRM_VERBOSE);

    tlb_tests();

    CU_basic_run_tests();
    CU_cleanup_registry();

    return CU_get_error();
}
