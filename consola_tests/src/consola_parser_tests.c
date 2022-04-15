#include "consola_parser_tests.h"

#include <CUnit/Basic.h>
#include <stdio.h>
#include <stdlib.h>

#include "buffer.h"
#include "consola_parser.h"

static t_buffer* buffer = NULL;
static t_log* consolaParseTestLogger = NULL;

static uint8_t instrOpCode = 0;
static uint32_t op1 = 0;
static uint32_t op2 = 0;

// @Before
void test_consola_parser_setup(void) {
    consolaParseTestLogger = log_create("../consola_parse_test.log", "Consola Parse Test", false, LOG_LEVEL_INFO);
    buffer = buffer_create();
}

// @After
void test_consola_parser_tear_down(void) {
    log_destroy(consolaParseTestLogger);
    buffer_destroy(buffer);
}

void test_es_posible_parsear_un_archivo_valido_de_instrucciones(void) {
    CU_ASSERT_TRUE(consola_parser_parse_instructions(buffer, "./instrucciones/i0", consolaParseTestLogger));

    // NO_OP 5
    buffer_unpack(buffer, &instrOpCode, sizeof(instrOpCode));
    buffer_unpack(buffer, &op1, sizeof(op1));
    CU_ASSERT_EQUAL(instrOpCode, 0);
    CU_ASSERT_EQUAL(op1, 5);

    // I/O 3000
    buffer_unpack(buffer, &instrOpCode, sizeof(instrOpCode));
    buffer_unpack(buffer, &op1, sizeof(op1));
    CU_ASSERT_EQUAL(instrOpCode, 1);
    CU_ASSERT_EQUAL(op1, 3000);

    // READ 0
    buffer_unpack(buffer, &instrOpCode, sizeof(instrOpCode));
    buffer_unpack(buffer, &op1, sizeof(op1));
    CU_ASSERT_EQUAL(instrOpCode, 2);
    CU_ASSERT_EQUAL(op1, 0);

    // WRITE 4 42
    buffer_unpack(buffer, &instrOpCode, sizeof(instrOpCode));
    buffer_unpack(buffer, &op1, sizeof(op1));
    buffer_unpack(buffer, &op2, sizeof(op2));
    CU_ASSERT_EQUAL(instrOpCode, 4);
    CU_ASSERT_EQUAL(op1, 4);
    CU_ASSERT_EQUAL(op2, 42);

    // COPY 0 4
    buffer_unpack(buffer, &instrOpCode, sizeof(instrOpCode));
    buffer_unpack(buffer, &op1, sizeof(op1));
    buffer_unpack(buffer, &op2, sizeof(op2));
    CU_ASSERT_EQUAL(instrOpCode, 3);
    CU_ASSERT_EQUAL(op1, 0);
    CU_ASSERT_EQUAL(op2, 4);

    // EXIT
    buffer_unpack(buffer, &instrOpCode, sizeof(instrOpCode));
    CU_ASSERT_EQUAL(instrOpCode, 5);
}

void test_es_posible_parsear_un_archivo_valido_de_instrucciones_con_lineas_vacias(void) {
    CU_ASSERT_TRUE(consola_parser_parse_instructions(buffer, "./instrucciones/i1", consolaParseTestLogger));

    // NO_OP 5
    buffer_unpack(buffer, &instrOpCode, sizeof(instrOpCode));
    buffer_unpack(buffer, &op1, sizeof(op1));
    CU_ASSERT_EQUAL(instrOpCode, 0);
    CU_ASSERT_EQUAL(op1, 5);

    // I/O 3000
    buffer_unpack(buffer, &instrOpCode, sizeof(instrOpCode));
    buffer_unpack(buffer, &op1, sizeof(op1));
    CU_ASSERT_EQUAL(instrOpCode, 1);
    CU_ASSERT_EQUAL(op1, 3000);

    // READ 0
    buffer_unpack(buffer, &instrOpCode, sizeof(instrOpCode));
    buffer_unpack(buffer, &op1, sizeof(op1));
    CU_ASSERT_EQUAL(instrOpCode, 2);
    CU_ASSERT_EQUAL(op1, 0);

    // WRITE 4 42
    buffer_unpack(buffer, &instrOpCode, sizeof(instrOpCode));
    buffer_unpack(buffer, &op1, sizeof(op1));
    buffer_unpack(buffer, &op2, sizeof(op2));
    CU_ASSERT_EQUAL(instrOpCode, 4);
    CU_ASSERT_EQUAL(op1, 4);
    CU_ASSERT_EQUAL(op2, 42);

    // COPY 0 4
    buffer_unpack(buffer, &instrOpCode, sizeof(instrOpCode));
    buffer_unpack(buffer, &op1, sizeof(op1));
    buffer_unpack(buffer, &op2, sizeof(op2));
    CU_ASSERT_EQUAL(instrOpCode, 3);
    CU_ASSERT_EQUAL(op1, 0);
    CU_ASSERT_EQUAL(op2, 4);

    // EXIT
    buffer_unpack(buffer, &instrOpCode, sizeof(instrOpCode));
    CU_ASSERT_EQUAL(instrOpCode, 5);
}

void test_no_es_posible_parsear_un_archivo_invalido_de_instrucciones_debido_a_la_primer_instruccion(void) {
    CU_ASSERT_FALSE(consola_parser_parse_instructions(buffer, "./instrucciones/i2", consolaParseTestLogger));
}

void test_no_es_posible_parsear_un_archivo_invalido_de_instrucciones_debido_a_la_segunda_instruccion(void) {
    CU_ASSERT_FALSE(consola_parser_parse_instructions(buffer, "./instrucciones/i3", consolaParseTestLogger));

    // NO_OP 5
    buffer_unpack(buffer, &instrOpCode, sizeof(instrOpCode));
    buffer_unpack(buffer, &op1, sizeof(op1));
    CU_ASSERT_EQUAL(instrOpCode, 0);
    CU_ASSERT_EQUAL(op1, 5);

    CU_ASSERT_EQUAL(buffer->size, 0);
}

void test_no_es_posible_parsear_un_archivo_invalido_de_instrucciones_debido_a_la_ultima_instruccion(void) {
    CU_ASSERT_FALSE(consola_parser_parse_instructions(buffer, "./instrucciones/i4", consolaParseTestLogger));

    // NO_OP 5
    buffer_unpack(buffer, &instrOpCode, sizeof(instrOpCode));
    buffer_unpack(buffer, &op1, sizeof(op1));
    CU_ASSERT_EQUAL(instrOpCode, 0);
    CU_ASSERT_EQUAL(op1, 5);

    // I/O 3000
    buffer_unpack(buffer, &instrOpCode, sizeof(instrOpCode));
    buffer_unpack(buffer, &op1, sizeof(op1));
    CU_ASSERT_EQUAL(instrOpCode, 1);
    CU_ASSERT_EQUAL(op1, 3000);

    // READ 0
    buffer_unpack(buffer, &instrOpCode, sizeof(instrOpCode));
    buffer_unpack(buffer, &op1, sizeof(op1));
    CU_ASSERT_EQUAL(instrOpCode, 2);
    CU_ASSERT_EQUAL(op1, 0);

    // WRITE 4 42
    buffer_unpack(buffer, &instrOpCode, sizeof(instrOpCode));
    buffer_unpack(buffer, &op1, sizeof(op1));
    buffer_unpack(buffer, &op2, sizeof(op2));
    CU_ASSERT_EQUAL(instrOpCode, 4);
    CU_ASSERT_EQUAL(op1, 4);
    CU_ASSERT_EQUAL(op2, 42);

    // COPY 0 4
    buffer_unpack(buffer, &instrOpCode, sizeof(instrOpCode));
    buffer_unpack(buffer, &op1, sizeof(op1));
    buffer_unpack(buffer, &op2, sizeof(op2));
    CU_ASSERT_EQUAL(instrOpCode, 3);
    CU_ASSERT_EQUAL(op1, 0);
    CU_ASSERT_EQUAL(op2, 4);

    CU_ASSERT_EQUAL(buffer->size, 0);
}

void test_no_es_posible_parsear_un_archivo_invalido_de_instrucciones_con_lineas_vacias_debido_a_la_ultima_instruccion(void) {
    CU_ASSERT_FALSE(consola_parser_parse_instructions(buffer, "./instrucciones/i5", consolaParseTestLogger));

    // NO_OP 5
    buffer_unpack(buffer, &instrOpCode, sizeof(instrOpCode));
    buffer_unpack(buffer, &op1, sizeof(op1));
    CU_ASSERT_EQUAL(instrOpCode, 0);
    CU_ASSERT_EQUAL(op1, 5);

    // I/O 3000
    buffer_unpack(buffer, &instrOpCode, sizeof(instrOpCode));
    buffer_unpack(buffer, &op1, sizeof(op1));
    CU_ASSERT_EQUAL(instrOpCode, 1);
    CU_ASSERT_EQUAL(op1, 3000);

    // READ 0
    buffer_unpack(buffer, &instrOpCode, sizeof(instrOpCode));
    buffer_unpack(buffer, &op1, sizeof(op1));
    CU_ASSERT_EQUAL(instrOpCode, 2);
    CU_ASSERT_EQUAL(op1, 0);

    // WRITE 4 42
    buffer_unpack(buffer, &instrOpCode, sizeof(instrOpCode));
    buffer_unpack(buffer, &op1, sizeof(op1));
    buffer_unpack(buffer, &op2, sizeof(op2));
    CU_ASSERT_EQUAL(instrOpCode, 4);
    CU_ASSERT_EQUAL(op1, 4);
    CU_ASSERT_EQUAL(op2, 42);

    // COPY 0 4
    buffer_unpack(buffer, &instrOpCode, sizeof(instrOpCode));
    buffer_unpack(buffer, &op1, sizeof(op1));
    buffer_unpack(buffer, &op2, sizeof(op2));
    CU_ASSERT_EQUAL(instrOpCode, 3);
    CU_ASSERT_EQUAL(op1, 0);
    CU_ASSERT_EQUAL(op2, 4);

    CU_ASSERT_EQUAL(buffer->size, 0);
}

/*
typedef enum {
    INSTRUCCION_no_op, 0
    INSTRUCCION_io,    1
    INSTRUCCION_read,  2
    INSTRUCCION_copy,  3
    INSTRUCCION_write, 4
    INSTRUCCION_exit   5
} t_instruccion;
*/