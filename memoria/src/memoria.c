#include <commons/log.h>
#include <errno.h>
#include <pthread.h>
#include <stdint.h>
#include <sys/socket.h>

#include "common_flags.h"
#include "connections.h"
#include "marcos.h"
#include "memoria_config.h"
#include "stream.h"
#include "tabla_nivel_1.h"
#include "tabla_nivel_2.h"
#include "tabla_suspendido.h"
#include "atender_cpu.h"

#define MEMORIA_CONFIG_PATH "cfg/memoria_config.cfg"
#define MEMORIA_LOG_PATH "bin/memoria.log"
#define MEMORIA_MODULE_NAME "Memoria"

t_log* memoriaLogger;
t_memoria_config* memoriaConfig;
void* memoriaPrincipal;
void* inicio_archivo;
int archivo_swap;
t_tabla_nivel_1* tablasDeNivel1;
t_tabla_nivel_2* tablasDeNivel2;
t_proceso_suspendido* tablaSuspendidos;
t_marcos* marcos;
bool cpuSinAtender = true;
bool kernelSinAtender = true;
int tamanioPagina;
int entradasPorTabla;
int cantidadProcesosMax;
int cantidadMarcosMax;
int cantidadMarcosProceso;
char* pathSwap;

int main(int argc, char* argv[]) {
    t_log* memoriaLogger = log_create(MEMORIA_LOG_PATH, MEMORIA_MODULE_NAME, true, LOG_LEVEL_INFO);
    t_memoria_config* memoriaConfig = memoria_config_create(MEMORIA_CONFIG_PATH, memoriaLogger);

    int socketEscucha = iniciar_servidor(memoria_config_get_ip_escucha(memoriaConfig), memoria_config_get_puerto_escucha(memoriaConfig));
    log_info(memoriaLogger, "Memoria(%s): A la escucha de Kernel y CPU en puerto %d", __FILE__, socketEscucha);

    int tamanioMemoria = memoria_config_get_tamanio_memoria(memoriaConfig);
    memoriaPrincipal = malloc(tamanioMemoria);
    memset(memoriaPrincipal, 0, tamanioMemoria);

    cantidadProcesosMax = memoria_config_get_procesos_totales(memoriaConfig);
    entradasPorTabla = memoria_config_get_entradas_por_tabla(memoriaConfig);
    tamanioPagina = memoria_config_get_tamanio_pagina(memoriaConfig);
    cantidadMarcosMax = tamanioMemoria / tamanioPagina;
    cantidadMarcosProceso = memoria_config_get_marcos_por_proceso(memoriaConfig);
    inicio_archivo = NULL;
    archivo_swap = -1;
    pathSwap = memoria_config_get_path_swap(memoriaConfig);
    
    tablasDeNivel1 = crear_tablas_de_nivel_1(cantidadProcesosMax, entradasPorTabla);
    tablasDeNivel2 = crear_tablas_de_nivel_2(cantidadProcesosMax, entradasPorTabla);
    marcos = crear_marcos(cantidadMarcosMax);
    tablaSuspendidos = crear_tabla_de_suspendidos();  // TODO que argumento pasar?

    __recibir_conexiones(socketEscucha);

    return 0;
}

void __recibir_conexiones(int socketEscucha) {
    pthread_t threadAtencion;
    int* socketCliente1 = malloc(sizeof(int));
    int* socketCliente2 = malloc(sizeof(int));
    void* (*funcion_suscripcion)(void) = NULL;
    funcion_suscripcion = __recibir_conexion(socketEscucha, socketCliente1, &threadAtencion);
    pthread_create(&threadAtencion, NULL, (void*)funcion_suscripcion, (void*) socketCliente1);
    pthread_detach(threadAtencion);

    funcion_suscripcion = __recibir_conexion(socketEscucha, socketCliente2, &threadAtencion);
    pthread_create(&threadAtencion, NULL, (void*)funcion_suscripcion, (void*) socketCliente2);
    pthread_join(threadAtencion, NULL);
}

void* __recibir_conexion(int socketEscucha, int* socketCliente, pthread_t* threadSuscripcion) {
    struct sockaddr cliente = {0};
    socklen_t len = sizeof(cliente);
    *socketCliente = accept(socketEscucha, &cliente, &len);

    if (socket == -1) {
        log_error(memoriaLogger, "Error al aceptar conexion de cliente: %s", strerror(errno));
        exit(-1);
    }

    uint8_t handshake = stream_recv_header(socket);
    stream_recv_empty_buffer(socket);

    void* (*funcion_suscripcion)(void) = NULL;
    if (handshake == HANDSHAKE_cpu && cpuSinAtender) {
        log_info(memoriaLogger, "Se acepta conexión de CPU en socket %d", socket);
        stream_send_empty_buffer(socket, HANDSHAKE_ok_continue);
        funcion_suscripcion = escuchar_peticiones_cpu;
        cpuSinAtender = false;
    } else if (handshake == HANDSHAKE_kernel && kernelSinAtender) {
        log_info(memoriaLogger, "Se acepta conexión de Kernel en socket %d", socket);
        stream_send_empty_buffer(socket, HANDSHAKE_ok_continue);
        funcion_suscripcion = escuchar_peticiones_kernel;
        kernelSinAtender = false;
    } else {
        log_error(memoriaLogger, "Error al recibir handshake de cliente: %s", strerror(errno));
        exit(-1);
    }

    return funcion_suscripcion;
}
