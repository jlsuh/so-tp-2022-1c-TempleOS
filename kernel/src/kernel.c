#include <commons/log.h>
#include <errno.h>
#include <pthread.h>
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
#include "scheduler.h"
#include "stream.h"

#define KERNEL_LOG_PATH "bin/kernel.log"
#define KERNEL_MODULE_NAME "Kernel"
#define NUMBER_OF_ARGS_REQUIRED 2

extern t_log* kernelLogger;
extern t_kernel_config* kernelConfig;

static void __crear_hilo_handler_conexion_entrante(int* socket) {
    pthread_t threadSuscripcion;
    pthread_create(&threadSuscripcion, NULL, encolar_en_new_a_nuevo_pcb_entrante, (void*)socket);
    pthread_detach(threadSuscripcion);
}

static void noreturn __aceptar_conexiones_kernel(int socketEscucha) {
    struct sockaddr cliente = {0};
    socklen_t len = sizeof(cliente);
    log_info(kernelLogger, "A la escucha de nuevas conexiones en puerto %d", socketEscucha);
    for (;;) {
        int clienteAceptado = accept(socketEscucha, &cliente, &len);
        if (clienteAceptado > -1) {
            int* socketCliente = malloc(sizeof(*socketCliente));
            *socketCliente = clienteAceptado;
            __crear_hilo_handler_conexion_entrante(socketCliente);
        } else {
            log_error(kernelLogger, "Error al aceptar conexión: %s", strerror(errno));
        }
    }
}

static void __kernel_destroy(t_kernel_config* kernelConfig, t_log* kernelLogger) {
    kernel_config_destroy(kernelConfig);
    log_destroy(kernelLogger);
}

int main(int argc, char* argv[]) {
    kernelLogger = log_create(KERNEL_LOG_PATH, KERNEL_MODULE_NAME, true, LOG_LEVEL_DEBUG);
    if (argc != NUMBER_OF_ARGS_REQUIRED) {
        log_error(kernelLogger, "Cantidad de argumentos inválida.\nArgumentos: <configPath>");
        log_destroy(kernelLogger);
        return -1;
    }
    kernelConfig = kernel_config_create(argv[1], kernelLogger);

    // Conexión con CPU en canal Dispatch
    const int socketCPUDispatch = conectar_a_servidor(kernel_config_get_ip_cpu(kernelConfig), kernel_config_get_puerto_cpu_dispatch(kernelConfig));
    if (socketCPUDispatch == -1) {
        log_error(kernelLogger, "Error al intentar establecer conexión inicial módulo CPU por canal Dispatch");
        __kernel_destroy(kernelConfig, kernelLogger);
        exit(-1);
    }
    kernel_config_set_socket_dispatch_cpu(kernelConfig, socketCPUDispatch);

    stream_send_empty_buffer(socketCPUDispatch, HANDSHAKE_dispatch);
    uint8_t cpuDispatchResponse = stream_recv_header(socketCPUDispatch);
    stream_recv_empty_buffer(socketCPUDispatch);
    if (cpuDispatchResponse != HANDSHAKE_ok_continue) {
        log_error(kernelLogger, "Error al intentar establecer conexión con CPU Dispatch");
        __kernel_destroy(kernelConfig, kernelLogger);
        exit(-1);
    }
    log_info(kernelLogger, "Conexión con CPU por canal Dispatch establecida");

    // Conexión con CPU en canal Interrupt
    const int socketCPUInterrupt = conectar_a_servidor(kernel_config_get_ip_cpu(kernelConfig), kernel_config_get_puerto_cpu_interrupt(kernelConfig));
    if (socketCPUInterrupt == -1) {
        log_error(kernelLogger, "Error al intentar conectar con CPU Interrupt");
        __kernel_destroy(kernelConfig, kernelLogger);
        exit(-1);
    }
    kernel_config_set_socket_interrupt_cpu(kernelConfig, socketCPUInterrupt);

    stream_send_empty_buffer(socketCPUInterrupt, HANDSHAKE_interrupt);
    uint8_t cpuInterruptResponse = stream_recv_header(socketCPUInterrupt);
    stream_recv_empty_buffer(socketCPUInterrupt);
    if (cpuInterruptResponse != HANDSHAKE_ok_continue) {
        log_error(kernelLogger, "Error al intentar establecer conexión con CPU Interrupt");
        __kernel_destroy(kernelConfig, kernelLogger);
        exit(-1);
    }
    log_info(kernelLogger, "Conexión con CPU por canal Interrupt establecida");

    // Conexión con Memoria
    const int socketMemoria = conectar_a_servidor(kernel_config_get_ip_memoria(kernelConfig), kernel_config_get_puerto_memoria(kernelConfig));
    if (socketCPUInterrupt == -1) {
        log_error(kernelLogger, "Error al intentar conectar con módulo Memoria");
        __kernel_destroy(kernelConfig, kernelLogger);
        exit(-1);
    }
    kernel_config_set_socket_memoria(kernelConfig, socketMemoria);

    stream_send_empty_buffer(socketMemoria, HANDSHAKE_kernel);
    uint8_t memoriaResponse = stream_recv_header(socketMemoria);
    stream_recv_empty_buffer(socketMemoria);
    if (memoriaResponse != HANDSHAKE_ok_continue) {
        log_error(kernelLogger, "Error al intentar establecer conexión con módulo Memoria");
        __kernel_destroy(kernelConfig, kernelLogger);
        exit(-1);
    }
    log_info(kernelLogger, "Conexión con módulo Memoria establecida");

    // Levantar servidor de instancias Consola
    int socketEscucha = iniciar_servidor(kernel_config_get_ip_escucha(kernelConfig), kernel_config_get_puerto_escucha(kernelConfig));
    if (socketEscucha == -1) {
        log_error(kernelLogger, "Error al intentar iniciar servidor");
        __kernel_destroy(kernelConfig, kernelLogger);
        exit(-1);
    }

    inicializar_estructuras();
    __aceptar_conexiones_kernel(socketEscucha);

    return 0;
}
