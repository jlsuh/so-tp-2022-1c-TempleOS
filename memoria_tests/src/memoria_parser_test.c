
#include "memoria_parser_test.h"
#include "tabla_nivel_1.h"

int cantidadProcesosMax;
int entradasPorTabla;

// @Before
void test_memoria_parser_setup(void) {
    cantidadProcesosMax = 8;
    entradasPorTabla = 2;
}

// @After
void test_memoria_parser_tear_down(void) {

}

void test_se_crea_correctamente(void){
    t_tabla_nivel_1* t = crear_tablas_de_nivel_1(cantidadProcesosMax, entradasPorTabla);
    int tablaNivel1 = obtener_tabla_de_nivel_1(6, cantidadProcesosMax, entradasPorTabla, t);
    CU_ASSERT_EQUAL(tablaNivel1, 2);
}