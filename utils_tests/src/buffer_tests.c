#include "buffer_tests.h"

#include <CUnit/Basic.h>
#include <stdlib.h>

#include "buffer.h"

static t_buffer* buffer;
static char* testString = "TEST_STRING";

// @Before
void test_buffer_setup(void) {
    buffer = buffer_create();
}

// @After
void test_buffer_tear_down(void) {
    buffer_destroy(buffer);
}

void test_es_posible_empaquetar_cualquier_valor(void) {
    uint8_t uint8 = 1;
    uint16_t uint16 = 2;
    uint32_t uint32 = 4;
    uint64_t uint64 = 8;

    buffer_pack(buffer, &uint8, sizeof(uint8));
    buffer_pack(buffer, &uint16, sizeof(uint16));
    buffer_pack(buffer, &uint32, sizeof(uint32));
    buffer_pack(buffer, &uint64, sizeof(uint64));
    buffer_pack_string(buffer, testString);

    CU_ASSERT_EQUAL(buffer->size, /*uint8_t:*/ 1 + /*uint16_t:*/ 2 + /*uint32_t:*/ 4 + /*uint64_t:*/ 8 + (/*uint32_t:*/ 4 + /*strlen+1:*/ 12));
}

void test_es_posible_desempaquetar_cualquier_valor(void) {
    uint8_t uint8 = 1;
    uint32_t uint32 = 4;

    buffer_pack(buffer, &uint8, sizeof(uint8));
    buffer_pack(buffer, &uint32, sizeof(uint32));
    buffer_pack_string(buffer, testString);

    uint8_t uint8_;
    uint32_t uint32_;
    buffer_unpack(buffer, &uint8_, sizeof(uint8_));
    buffer_unpack(buffer, &uint32_, sizeof(uint32_));
    char* unpackedStr = buffer_unpack_string(buffer);

    CU_ASSERT_EQUAL(uint8_, uint8);
    CU_ASSERT_EQUAL(uint32_, uint32);
    CU_ASSERT_STRING_EQUAL(unpackedStr, testString);

    free(unpackedStr);
}

void test_es_posible_empaquetar_y_desempaquetar_un_string(void) {
    buffer_pack_string(buffer, testString);
    char* actual = buffer_unpack_string(buffer);

    CU_ASSERT_STRING_EQUAL(actual, testString);

    free(actual);
}
