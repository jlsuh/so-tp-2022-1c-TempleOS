#include "misc_tests.h"

#include <CUnit/Basic.h>
#include <commons/collections/dictionary.h>

void dictionary_tests(void) {
    t_dictionary* dict = dictionary_create();
    int* ints = malloc(sizeof(int) * 3);
    ints[0] = 1;
    ints[1] = 2;
    ints[2] = 3;
    dictionary_put(dict, "key1", ints);
    free(ints);
    int* removedInts = dictionary_get(dict, "key1");
    CU_ASSERT_EQUAL(removedInts[0], 1);
    CU_ASSERT_EQUAL(removedInts[1], 2);
    CU_ASSERT_EQUAL(removedInts[2], 3);
}
