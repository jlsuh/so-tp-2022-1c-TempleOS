#include <commons/log.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>

#include "common_flags.h"
#include "connections.h"
#include "cpu_config.h"
#include "cpu_kernel_server.h"
#include "stream.h"
#include "tlb.h"

#define CPU_CONFIG_PATH "cfg/cpu_config.cfg"
#define CPU_LOG_PATH "bin/cpu.log"
#define CPU_MODULE_NAME "CPU"

t_tlb* tlb;

extern t_log* cpuLogger;
extern t_cpu_config* cpuConfig;

int main(int argc, char* argv[]) {
    cpuLogger = log_create(CPU_LOG_PATH, CPU_MODULE_NAME, true, LOG_LEVEL_INFO);
    cpuConfig = cpu_config_create(CPU_CONFIG_PATH, cpuLogger);

    // Conexión con Memoria
    const int memoriaSocket = conectar_a_servidor(cpu_config_get_ip_memoria(cpuConfig), cpu_config_get_puerto_memoria(cpuConfig));
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
    }
    log_info(cpuLogger, "Conexión con Memoria establecida");

    // Servidor de Kernel
    int socketEscuchaDispatch = iniciar_servidor(cpu_config_get_ip_cpu(cpuConfig), cpu_config_get_puerto_dispatch(cpuConfig));
    int socketEscuchaInterrupt = iniciar_servidor(cpu_config_get_ip_cpu(cpuConfig), cpu_config_get_puerto_interrupt(cpuConfig));

    struct sockaddr cliente = {0};
    socklen_t len = sizeof(cliente);

    // Conexión con Kernel en canal Dispatch
    int kernelDispatchSocket = accept(socketEscuchaDispatch, &cliente, &len);
    if (kernelDispatchSocket == -1) {
        log_error(cpuLogger, "Error al intentar establecer conexión inicial módulo Kernel por canal Dispatch");
        log_destroy(cpuLogger);
        return -1;
    }
    cpu_config_set_socket_dispatch(cpuConfig, kernelDispatchSocket);

    uint8_t kernelDispatchResponse = stream_recv_header(kernelDispatchSocket);
    stream_recv_empty_buffer(kernelDispatchSocket);
    if (kernelDispatchResponse != HANDSHAKE_dispatch) {
        log_error(cpuLogger, "Error al intentar establecer Handshake inicial con módulo Kernel por canal Dispatch");
        log_destroy(cpuLogger);
        return -1;
    }
    stream_send_empty_buffer(kernelDispatchSocket, HANDSHAKE_ok_continue);
    log_info(cpuLogger, "Conexión con Kernel por canal Dispatch establecida");

    // Conexión con Kernel en canal Interrupt
    int kernelInterruptSocket = accept(socketEscuchaInterrupt, &cliente, &len);
    if (kernelInterruptSocket == -1) {
        log_error(cpuLogger, "Error al intentar establecer conexión inicial módulo Kernel por canal Interrupt");
        log_destroy(cpuLogger);
        return -1;
    }
    cpu_config_set_socket_interrupt(cpuConfig, kernelInterruptSocket);

    uint8_t kernelInterruptResponse = stream_recv_header(kernelInterruptSocket);
    stream_recv_empty_buffer(kernelInterruptSocket);
    if (kernelInterruptResponse != HANDSHAKE_interrupt) {
        log_error(cpuLogger, "Error al intentar establecer Handshake inicial con módulo Kernel por canal Interrupt");
        log_destroy(cpuLogger);
        return -1;
    }
    stream_send_empty_buffer(kernelInterruptSocket, HANDSHAKE_ok_continue);
    log_info(cpuLogger, "Conexión con Kernel por canal Interrupt establecida");

    tlb = tlb_create(cpu_config_get_entradas_tlb(cpuConfig), cpu_config_get_reemplazo_tlb(cpuConfig));

    atender_peticiones_de_kernel();

    return 0;
}
