#include <errno.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>

#include "algoritmos.h"
#include "atender_cpu.h"
#include "atender_kernel.h"
#include "common_flags.h"
#include "connections.h"
#include "marcos.h"
#include "memoria_config.h"
#include "stream.h"
#include "tabla_nivel_1.h"
#include "tabla_nivel_2.h"
#include "tabla_suspendido.h"

#define MEMORIA_LOG_PATH "bin/memoria.log"
#define MEMORIA_MODULE_NAME "Memoria"
#define NUMBER_OF_ARGS_REQUIRED 2

t_memoria_data_holder* memoriaData;
pthread_mutex_t mutexMemoriaData;

static bool cpuSinAtender;
static bool kernelSinAtender;

void* __recibir_conexion(int socketEscucha, int* socketCliente, pthread_t* threadSuscripcion) {
    struct sockaddr cliente = {0};
    socklen_t len = sizeof(cliente);
    *socketCliente = accept(socketEscucha, &cliente, &len);
    if (*socketCliente == -1) {
        log_error(memoriaData->memoriaLogger, "Error al aceptar conexion de cliente: %s", strerror(errno));
        exit(-1);
    }
    uint8_t handshake = stream_recv_header(*socketCliente);
    stream_recv_empty_buffer(*socketCliente);
    void* (*funcion_suscripcion)(void*) = NULL;
    if (handshake == HANDSHAKE_cpu && cpuSinAtender) {
        log_info(memoriaData->memoriaLogger, "\e[1;92mSe acepta conexión de CPU en socket [%d]\e[0m", *socketCliente);
        t_buffer* buffer = buffer_create();
        uint32_t tamanioPagina = memoriaData->tamanioPagina;
        uint32_t entradasPorTabla = memoriaData->entradasPorTabla;
        buffer_pack(buffer, &tamanioPagina, sizeof(tamanioPagina));
        buffer_pack(buffer, &entradasPorTabla, sizeof(entradasPorTabla));
        stream_send_buffer(*socketCliente, HANDSHAKE_ok_continue, buffer);
        buffer_destroy(buffer);
        funcion_suscripcion = escuchar_peticiones_cpu;
        cpuSinAtender = false;
    } else if (handshake == HANDSHAKE_kernel && kernelSinAtender) {
        log_info(memoriaData->memoriaLogger, "\e[1;92mSe acepta conexión de Kernel en socket [%d]\e[0m", *socketCliente);
        stream_send_empty_buffer(*socketCliente, HANDSHAKE_ok_continue);
        funcion_suscripcion = escuchar_peticiones_kernel;
        kernelSinAtender = false;
    } else {
        log_error(memoriaData->memoriaLogger, "Error al recibir handshake de cliente: %s", strerror(errno));
        exit(-1);
    }
    return funcion_suscripcion;
}

void __recibir_conexiones(int socketEscucha) {
    pthread_t threadAtencion;
    int* socketCliente1 = malloc(sizeof(int));
    int* socketCliente2 = malloc(sizeof(int));
    void* (*funcion_suscripcion)(void) = NULL;
    funcion_suscripcion = __recibir_conexion(socketEscucha, socketCliente1, &threadAtencion);
    pthread_create(&threadAtencion, NULL, (void*)funcion_suscripcion, (void*)socketCliente1);
    pthread_detach(threadAtencion);
    funcion_suscripcion = __recibir_conexion(socketEscucha, socketCliente2, &threadAtencion);
    pthread_create(&threadAtencion, NULL, (void*)funcion_suscripcion, (void*)socketCliente2);
    pthread_join(threadAtencion, NULL);
}

int main(int argc, char* argv[]) {
    memoriaData = malloc(sizeof(*memoriaData));

    memoriaData->memoriaLogger = log_create(MEMORIA_LOG_PATH, MEMORIA_MODULE_NAME, true, LOG_LEVEL_INFO);
    if (argc != NUMBER_OF_ARGS_REQUIRED) {
        log_error(memoriaData->memoriaLogger, "Cantidad de argumentos inválida.\nArgumentos: <configPath>");
        log_destroy(memoriaData->memoriaLogger);
        return -1;
    }
    memoriaData->memoriaConfig = memoria_config_create(argv[1], memoriaData->memoriaLogger);

    int socketEscucha = iniciar_servidor(memoria_config_get_ip_escucha(memoriaData->memoriaConfig), memoria_config_get_puerto_escucha(memoriaData->memoriaConfig));
    log_info(memoriaData->memoriaLogger, "\e[1;92mA la escucha de Kernel y CPU en puerto %d\e[0m", socketEscucha);

    int tamanioMemoria = memoria_config_get_tamanio_memoria(memoriaData->memoriaConfig);
    memoriaData->memoriaPrincipal = malloc(tamanioMemoria);
    memset(memoriaData->memoriaPrincipal, 0, tamanioMemoria);

    memoriaData->cantidadProcesosMax = memoria_config_get_procesos_totales(memoriaData->memoriaConfig);
    memoriaData->entradasPorTabla = memoria_config_get_entradas_por_tabla(memoriaData->memoriaConfig);
    memoriaData->tamanioPagina = memoria_config_get_tamanio_pagina(memoriaData->memoriaConfig);
    memoriaData->cantidadMarcosMax = tamanioMemoria / memoriaData->tamanioPagina;
    memoriaData->cantidadMarcosProceso = memoria_config_get_marcos_por_proceso(memoriaData->memoriaConfig);
    memoriaData->inicio_archivo = NULL;
    memoriaData->archivo_swap = -1;
    memoriaData->pathSwap = memoria_config_get_path_swap(memoriaData->memoriaConfig);
    memoriaData->contadorTabla1 = 1;
    memoriaData->retardoSwap = memoria_config_get_retardo_swap(memoriaData->memoriaConfig);
    memoriaData->retardoMemoria = memoria_config_get_retardo_memoria(memoriaData->memoriaConfig);
    int paginasPorProceso = memoriaData->entradasPorTabla * memoriaData->entradasPorTabla;
    memoriaData->tamanioMaxArchivo = paginasPorProceso * memoriaData->tamanioPagina;
    if (memoria_config_es_algoritmo_sustitucion_clock(memoriaData->memoriaConfig)) {
        memoriaData->seleccionar_victima = seleccionar_victima_clock;
    } else if (memoria_config_es_algoritmo_sustitucion_clock_modificado(memoriaData->memoriaConfig)) {
        memoriaData->seleccionar_victima = seleccionar_victima_clock_modificado;
    } else {
        log_error(memoriaData->memoriaLogger, "No se reconocio el algoritmo de sustitucion");
        exit(-1);
    }
    memoriaData->tablasDeNivel1 = crear_tablas_de_nivel_1(memoriaData);
    memoriaData->tablasDeNivel2 = crear_tablas_de_nivel_2(memoriaData);
    memoriaData->marcos = crear_marcos(memoriaData);
    memoriaData->tablaSuspendidos = crear_tabla_de_suspendidos();

    pthread_mutex_init(&mutexMemoriaData, NULL);

    cpuSinAtender = true;
    kernelSinAtender = true;

    __recibir_conexiones(socketEscucha);

    return 0;
}
