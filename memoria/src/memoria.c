#include <commons/log.h>
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <pthread.h>

#include "common_flags.h"
#include "connections.h"
#include "memoria_config.h"
#include "stream.h"

#define MEMORIA_CONFIG_PATH "cfg/memoria_config.cfg"
#define MEMORIA_LOG_PATH "bin/memoria.log"
#define MEMORIA_MODULE_NAME "Memoria"

t_log* memoriaLogger;
t_memoria_config* memoriaConfig;

int main(int argc, char* argv[]) {
    memoriaLogger = log_create(MEMORIA_LOG_PATH, MEMORIA_MODULE_NAME, true, LOG_LEVEL_INFO);
    memoriaConfig = memoria_config_create(MEMORIA_CONFIG_PATH, memoriaLogger);

    int socketEscucha = iniciar_servidor(memoria_config_get_ip_escucha(memoriaConfig), memoria_config_get_puerto_escucha(memoriaConfig));
    log_info(memoriaLogger, "Memoria(%s): A la escucha de Kernel y CPU en puerto %d", __FILE__, socketEscucha);

    while (memoria_config_get_kernel_socket(memoriaConfig) != -1 || memoria_config_get_cpu_socket(memoriaConfig) != -1) {
        struct sockaddr cliente;
        socklen_t len = sizeof(cliente);
        int socketCliente = accept(socketEscucha, &cliente, &len);

        if (socketCliente > 0) {
            uint32_t response = stream_recv_header(socketCliente);
            recv_empty_buffer(socketCliente);
            if (response == HANDSHAKE_kernel) {
                send_empty_buffer(HANDSHAKE_ok_continue, socketCliente);
                memoria_config_set_kernel_socket(memoriaConfig, socketCliente);
                crear_hilo_handler_peticiones_de_kernel();
            } else if (response == HANDSHAKE_cpu) {
                send_empty_buffer(HANDSHAKE_ok_continue, socketCliente);
                memoria_config_set_cpu_socket(memoriaConfig, socketCliente);
                crear_hilo_handler_peticiones_de_cpu();
            } else {
                log_error(memoriaLogger, "Memoria(%s): Error al intentar establecer Handshake inicial", __FILE__);
                close(socketCliente);
            }
        } else {
            log_error(memoriaLogger, "Memoria(%s): Error al aceptar conexión: %s", __FILE__, strerror(errno));
        }
    }

    return 0;
}

void crear_hilo_handler_peticiones_de_kernel(){
    pthread_t threadSuscripcion;
    pthread_create(&threadSuscripcion, NULL, (void*) escuchar_peticiones_kernel, NULL);
    pthread_detach(threadSuscripcion);
}

void crear_hilo_handler_peticiones_de_cpu(){
    pthread_t threadSuscripcion;
    pthread_create(&threadSuscripcion, NULL, (void*) escuchar_peticiones_cpu, NULL);
    pthread_detach(threadSuscripcion);
}

void* escuchar_peticiones_kernel(){
    int socket = memoria_config_get_kernel_socket(memoriaConfig);
}

void* escuchar_peticiones_cpu(){
    int socket = memoria_config_get_cpu_socket(memoriaConfig);
}