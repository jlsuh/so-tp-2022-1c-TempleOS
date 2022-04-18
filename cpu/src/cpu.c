#include <commons/log.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>

#include "common_flags.h"
#include "connections.h"
#include "cpu_config.h"
#include "stream.h"

#define CPU_CONFIG_PATH "cfg/cpu_config.cfg"
#define CPU_LOG_PATH "bin/cpu.log"
#define CPU_MODULE_NAME "CPU"

t_log* cpuLogger;
t_cpu_config* cpuConfig;

void* cpu_dispatch_handler(void* socketEscuchaDispatchVoid) {
    int* socketEscuchaDispatch = (int*)socketEscuchaDispatchVoid;
    printf("[CPU] Escuchando en el socket %d\n", *socketEscuchaDispatch);
    struct sockaddr cliente = {0};
    socklen_t len = sizeof(cliente);
    int socketDispatch = accept(*socketEscuchaDispatch, &cliente, &len);
    if (socketDispatch == -1) {
        log_error(cpuLogger, "Error al intentar aceptar conexión con CPU Dispatch");
        log_destroy(cpuLogger);
        return NULL;
    }
    cpu_config_set_socket_dispatch(cpuConfig, socketDispatch);
    printf("[CPU] Conexión con CPU Dispatch aceptada\n");
    return NULL;
}

void* cpu_interrupt_handler(void* socketEscuchaInterruptVoid) {
    int* socketEscuchaInterrupt = (int*)socketEscuchaInterruptVoid;
    printf("[CPU] Escuchando en el socket %d\n", *socketEscuchaInterrupt);
    struct sockaddr cliente = {0};
    socklen_t len = sizeof(cliente);
    int socketInterrupt = accept(*socketEscuchaInterrupt, &cliente, &len);
    if (socketInterrupt == -1) {
        log_error(cpuLogger, "Error al intentar aceptar conexión con CPU Interrupt");
        log_destroy(cpuLogger);
        return NULL;
    }
    cpu_config_set_socket_interrupt(cpuConfig, socketInterrupt);
    printf("[CPU] Conexión con CPU Interrupt aceptada\n");
    return NULL;
}

int main(int argc, char* argv[]) {
    cpuLogger = log_create(CPU_LOG_PATH, CPU_MODULE_NAME, true, LOG_LEVEL_INFO);
    cpuConfig = cpu_config_create(CPU_CONFIG_PATH, cpuLogger);

    /* const int memoriaSocket = conectar_a_servidor(cpu_config_get_ip_memoria(cpuConfig), cpu_config_get_puerto_memoria(cpuConfig));
    if (memoriaSocket == -1) {
        log_error(cpuLogger, "Error al intentar establecer conexión inicial con módulo Memoria");
        log_destroy(cpuLogger);
        return -1;
    }
    cpu_config_set_socket_memoria(cpuConfig, memoriaSocket);

    stream_send_empty_buffer(memoriaSocket, HANDSHAKE_cpu);

    uint8_t memoriaResponse = stream_recv_header(memoriaSocket);
    stream_recv_empty_buffer(memoriaSocket);

    if (memoriaResponse != HANDSHAKE_ok_continue) {
        log_error(cpuLogger, "Error al intentar establecer Handshake inicial con módulo Memoria");
        log_destroy(cpuLogger);
        return -1;
    } */

    int* socketEscuchaDispatch = malloc(sizeof(int));
    int* socketEscuchaInterrupt = malloc(sizeof(int));

    *socketEscuchaDispatch = iniciar_servidor(cpu_config_get_ip_cpu(cpuConfig), cpu_config_get_puerto_dispatch(cpuConfig));
    *socketEscuchaInterrupt = iniciar_servidor(cpu_config_get_ip_cpu(cpuConfig), cpu_config_get_puerto_interrupt(cpuConfig));

    pthread_t dispatchTh;
    pthread_create(&dispatchTh, NULL, cpu_dispatch_handler, (void*)socketEscuchaDispatch);

    pthread_t interruptTh;
    pthread_create(&interruptTh, NULL, cpu_interrupt_handler, (void*)socketEscuchaInterrupt);

    pthread_join(dispatchTh, NULL);
    pthread_join(interruptTh, NULL);

    return 0;
}
