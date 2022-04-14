#include "stream_connections_tests.h"

#include <CUnit/Basic.h>
#include <commons/config.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>

#include "buffer.h"
#include "connections.h"
#include "stream.h"

static char* ip;
static char* puerto;
static int sockServ;
static int sockServDelCliente;
static int sockClienteDelServ;
static struct sockaddr sockAddr;
static socklen_t addrSize;

static char* testString = "TEST_STRING";

// @Before
void test_stream_conexiones_setup(void) {
    ip = "127.0.0.1";
    puerto = "5001";
    sockServ = iniciar_servidor(ip, puerto);
    sockServDelCliente = conectar_a_servidor(ip, puerto);
    addrSize = sizeof(sockAddr);
    sockClienteDelServ = accept(sockServ, &sockAddr, &addrSize);
}

// @After
void test_stream_conexiones_tear_down(void) {
    close(sockClienteDelServ);
    close(sockServDelCliente);
    close(sockServ);
}

void test_es_posible_establecer_una_conexion(void) {
    CU_ASSERT(sockClienteDelServ > 0);
}

void test_es_posible_serializar_un_string_enviarlo_y_deserializarlo(void) {
    t_buffer* senderBuffer = buffer_create();
    buffer_pack_string(senderBuffer, testString);
    stream_send_buffer(sockClienteDelServ, 0, senderBuffer);
    buffer_destroy(senderBuffer);

    t_buffer* recvdBuffer = buffer_create();
    stream_recv_header(sockServDelCliente);
    stream_recv_buffer(sockServDelCliente, recvdBuffer);
    char* recvdString = buffer_unpack_string(recvdBuffer);
    buffer_destroy(recvdBuffer);

    CU_ASSERT_STRING_EQUAL(recvdString, testString);

    free(recvdString);
}
