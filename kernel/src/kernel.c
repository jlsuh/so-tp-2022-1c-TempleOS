#include <commons/log.h>
#include <errno.h>
#include <pthread.h>
#include <signal.h>  //TODO borrar
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdnoreturn.h>
#include <string.h>
#include <sys/socket.h>

#include "buffer.h"
#include "common_flags.h"
#include "connections.h"
#include "kernel_config.h"
#include "stream.h"

#define KERNEL_CONFIG_PATH "cfg/kernel_config.cfg"
#define KERNEL_LOG_PATH "bin/kernel.log"
#define KERNEL_MODULE_NAME "Kernel"

t_log* kernelLogger;
t_kernel_config* kernelConfig;
static int* socketUltimoCliente;

static void* testTemporal(void* socket) {
    int* socketProceso = (int*)socket;
    uint8_t response = stream_recv_header(*socketProceso);
    uint32_t tamanio = 0;
    if (response != HANDSHAKE_consola) {
        log_error(kernelLogger, "Error al intentar establecer conexión con proceso %d", *socketProceso);
        pthread_exit(NULL);
    } else {
        t_buffer* buffer = buffer_create();
        stream_recv_buffer(*socketProceso, buffer);
        buffer_unpack(buffer, &tamanio, sizeof(uint32_t));
        buffer_destroy(buffer);
        log_info(kernelLogger, "Se establece handshake con proceso %d de tamaño %d", *socketProceso, tamanio);
        stream_send_empty_buffer(*socketProceso, HANDSHAKE_ok_continue);
        stream_recv_header(*socketProceso);  // Header lista de instrucciones solamente
    }

    t_buffer* instructionsBuffer = buffer_create();
    stream_recv_buffer(*socketProceso, instructionsBuffer);

    uint8_t instruction = -1;
    uint32_t op1 = -1;
    uint32_t op2 = -1;
    buffer_unpack(instructionsBuffer, &instruction, sizeof(instruction));
    for (; instruction != INSTRUCCION_exit;) {
        switch (instruction) {
            case INSTRUCCION_no_op:
                buffer_unpack(instructionsBuffer, &op1, sizeof(op1));
                break;
            case INSTRUCCION_io:
                buffer_unpack(instructionsBuffer, &op1, sizeof(op1));
                break;
            case INSTRUCCION_read:
                buffer_unpack(instructionsBuffer, &op1, sizeof(op1));
                break;
            case INSTRUCCION_copy:
                buffer_unpack(instructionsBuffer, &op1, sizeof(op1));
                buffer_unpack(instructionsBuffer, &op2, sizeof(op2));
                break;
            case INSTRUCCION_write:
                buffer_unpack(instructionsBuffer, &op1, sizeof(op1));
                buffer_unpack(instructionsBuffer, &op2, sizeof(op2));
                break;
            default:
                log_error(kernelLogger, "Error al intentar establecer conexión con proceso %d", *socketProceso);
                pthread_exit(NULL);
        }
        printf("Instrucción: %d con argumento/s %d %d\n", instruction, op1, op2);
        op1 = -1;
        op2 = -1;
        buffer_unpack(instructionsBuffer, &instruction, sizeof(instruction));
    }
    puts("Proceso terminado");
    buffer_destroy(instructionsBuffer);
    free(socketProceso);  // TODO socketProceso iria en pcb
    pthread_exit(NULL);
}

static void __crear_hilo_handler_conexion_entrante(int* socket) {
    pthread_t threadSuscripcion;
    pthread_create(&threadSuscripcion, NULL, testTemporal, (void*)socket);  // TODO modificar
    pthread_detach(threadSuscripcion);
    return;
}

static noreturn void __aceptar_conexiones_kernel(int socketEscucha, struct sockaddr cliente, socklen_t len) {
    log_info(kernelLogger, "Kernel: A la escucha de nuevas conexiones en puerto %d", socketEscucha);
    int* socketCliente;
    for (;;) {
        socketCliente = malloc(sizeof(*socketCliente));
        socketUltimoCliente = socketCliente;
        *socketCliente = accept(socketEscucha, &cliente, &len);
        if (*socketCliente > 0) {
            __crear_hilo_handler_conexion_entrante(socketCliente);
        } else {
            log_error(kernelLogger, "Kernel: Error al aceptar conexión: %s", strerror(errno));
        }
    }
}

static void __kernel_destroy(t_kernel_config* kernelConfig, t_log* kernelLogger) {
    kernel_config_destroy(kernelConfig);
    log_destroy(kernelLogger);
}

void sig_int(int _) {  // TODO borrar
    __kernel_destroy(kernelConfig, kernelLogger);
    free(socketUltimoCliente);
    exit(EXIT_SUCCESS);
}

int main(int argc, char* argv[]) {
    kernelLogger = log_create(KERNEL_LOG_PATH, KERNEL_MODULE_NAME, true, LOG_LEVEL_INFO);
    kernelConfig = kernel_config_create(KERNEL_CONFIG_PATH, kernelLogger);

    int socketEscucha = iniciar_servidor(kernel_config_get_ip_escucha(kernelConfig), kernel_config_get_puerto_escucha(kernelConfig));

    // TODO borrar
    struct sigaction sigInt;
    sigInt.sa_handler = sig_int;
    sigemptyset(&sigInt.sa_mask);
    sigInt.sa_flags = SA_RESTART;
    sigaction(SIGINT, &sigInt, NULL);

    struct sockaddr proceso;
    socklen_t len = 0;

    __aceptar_conexiones_kernel(socketEscucha, proceso, len);
    //__kernel_destroy(kernelConfig, kernelLogger);
    return 0;
}
