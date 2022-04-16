#include "consola_parser_tests.h"

#include <CUnit/Basic.h>
#include <stdio.h>
#include <stdlib.h>

#include "buffer.h"
#include "consola_parser.h"

static t_buffer* buffer = NULL;
static t_log* consolaParserTestLogger = NULL;

/* static uint8_t instrOpCode = 0;
static uint32_t op1 = 0;
static uint32_t op2 = 0; */

// @Before
void test_consola_parser_setup(void) {
    consolaParserTestLogger = log_create("../consolaParserTest.log", "Consola Parser Test", false, LOG_LEVEL_INFO);
    buffer = buffer_create();
}

// @After
void test_consola_parser_tear_down(void) {
    log_destroy(consolaParserTestLogger);
    buffer_destroy(buffer);
}

static void __assert_opCode_with_no_args(t_buffer* buffer, uint8_t expectedInstrOpCode) {
    uint8_t actualInstrOpCode = 0;
    buffer_unpack(buffer, &actualInstrOpCode, sizeof(actualInstrOpCode));
    CU_ASSERT_EQUAL(actualInstrOpCode, expectedInstrOpCode);
}

static void __assert_opCode_with_one_args(t_buffer* buffer, uint8_t expectedInstrOpCode, uint32_t expectedOp1) {
    __assert_opCode_with_no_args(buffer, expectedInstrOpCode);
    uint32_t actualOp1 = 0;
    buffer_unpack(buffer, &actualOp1, sizeof(actualOp1));
    CU_ASSERT_EQUAL(actualOp1, expectedOp1);
}

static void __assert_opCode_with_two_args(t_buffer* buffer, uint8_t expectedInstrOpCode, uint32_t expectedOp1, uint32_t expectedOp2) {
    __assert_opCode_with_one_args(buffer, expectedInstrOpCode, expectedOp1);
    uint32_t actualOp2 = 0;
    buffer_unpack(buffer, &actualOp2, sizeof(actualOp2));
    CU_ASSERT_EQUAL(actualOp2, expectedOp2);
}

void test_es_posible_parsear_un_archivo_valido_de_instrucciones(void) {
    CU_ASSERT_TRUE(consola_parser_parse_instructions(buffer, "./instrucciones/i0", consolaParserTestLogger));
    // NO_OP 5
    __assert_opCode_with_one_args(buffer, 0, 5);
    // I/O 3000
    __assert_opCode_with_one_args(buffer, 1, 3000);
    // READ 0
    __assert_opCode_with_one_args(buffer, 2, 0);
    // WRITE 4 42
    __assert_opCode_with_two_args(buffer, 4, 4, 42);
    // COPY 0 4
    __assert_opCode_with_two_args(buffer, 3, 0, 4);
    // EXIT
    __assert_opCode_with_no_args(buffer, 5);
}

void test_es_posible_parsear_un_archivo_valido_de_instrucciones_con_lineas_vacias(void) {
    CU_ASSERT_TRUE(consola_parser_parse_instructions(buffer, "./instrucciones/i1", consolaParserTestLogger));
    // NO_OP 5
    __assert_opCode_with_one_args(buffer, 0, 5);
    // I/O 3000
    __assert_opCode_with_one_args(buffer, 1, 3000);
    // READ 0
    __assert_opCode_with_one_args(buffer, 2, 0);
    // WRITE 4 42
    __assert_opCode_with_two_args(buffer, 4, 4, 42);
    // COPY 0 4
    __assert_opCode_with_two_args(buffer, 3, 0, 4);
    // EXIT
    __assert_opCode_with_no_args(buffer, 5);
}

void test_no_es_posible_parsear_un_archivo_invalido_de_instrucciones_debido_a_la_primer_instruccion(void) {
    CU_ASSERT_FALSE(consola_parser_parse_instructions(buffer, "./instrucciones/i2", consolaParserTestLogger));
}

void test_no_es_posible_parsear_un_archivo_invalido_de_instrucciones_debido_a_la_segunda_instruccion(void) {
    CU_ASSERT_FALSE(consola_parser_parse_instructions(buffer, "./instrucciones/i3", consolaParserTestLogger));
    // NO_OP 5
    __assert_opCode_with_one_args(buffer, 0, 5);
    CU_ASSERT_EQUAL(buffer->size, 0);
}

void test_no_es_posible_parsear_un_archivo_invalido_de_instrucciones_debido_a_la_ultima_instruccion(void) {
    CU_ASSERT_FALSE(consola_parser_parse_instructions(buffer, "./instrucciones/i4", consolaParserTestLogger));
    // NO_OP 5
    __assert_opCode_with_one_args(buffer, 0, 5);
    // I/O 3000
    __assert_opCode_with_one_args(buffer, 1, 3000);
    // READ 0
    __assert_opCode_with_one_args(buffer, 2, 0);
    // WRITE 4 42
    __assert_opCode_with_two_args(buffer, 4, 4, 42);
    // COPY 0 4
    __assert_opCode_with_two_args(buffer, 3, 0, 4);
    CU_ASSERT_EQUAL(buffer->size, 0);
}

void test_no_es_posible_parsear_un_archivo_invalido_de_instrucciones_con_lineas_vacias_debido_a_la_ultima_instruccion(void) {
    CU_ASSERT_FALSE(consola_parser_parse_instructions(buffer, "./instrucciones/i5", consolaParserTestLogger));
    // NO_OP 5
    __assert_opCode_with_one_args(buffer, 0, 5);
    // I/O 3000
    __assert_opCode_with_one_args(buffer, 1, 3000);
    // READ 0
    __assert_opCode_with_one_args(buffer, 2, 0);
    // WRITE 4 42
    __assert_opCode_with_two_args(buffer, 4, 4, 42);
    // COPY 0 4
    __assert_opCode_with_two_args(buffer, 3, 0, 4);
    CU_ASSERT_EQUAL(buffer->size, 0);
}
