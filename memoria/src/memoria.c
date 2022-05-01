#include <commons/collections/list.h>
#include <commons/log.h>
#include <errno.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "common_flags.h"
#include "connections.h"
#include "memoria_config.h"
#include "stream.h"

#define MEMORIA_CONFIG_PATH "cfg/memoria_config.cfg"
#define MEMORIA_LOG_PATH "bin/memoria.log"
#define MEMORIA_MODULE_NAME "Memoria"

typedef struct
{
    int* nroTablaNivel2;
} t_tabla_nivel_uno;
typedef struct
{
    int indiceMarco;
    bool bitPresencia;
    bool bitUso;
    bool bitModificado;
} t_entrada_nivel_dos;
typedef struct
{
    t_entrada_nivel_dos* entradaNivel2;
} t_tabla_nivel_dos;

t_log* memoriaLogger;
t_memoria_config* memoriaConfig;
void* memoriaPrincipal;
t_tabla_nivel_uno* tablasNivel1;
t_tabla_nivel_dos* tablasNivel2;
bool* marcosEnUso;
int cantTotalMarcos;
int cantidadProcesosMax;
int entradasPorTabla;
int tamanioPagina;
int marcosPorProceso;

int main(int argc, char* argv[]);
int obtener_marco_disponible(void);
int obtener_tabla_primer_nivel_libre(void);
int obtener_tabla_segundo_nivel_libre(void);
int crear_tabla_segundo_nivel(void);
uint32_t crear_nuevo_proceso(uint32_t tamanio);
void* escuchar_peticiones_cpu(void);
void* escuchar_peticiones_kernel(void);
void asignar_marcos_a_proceso(int numeroTablaPrimerNivel, int indiceMarco);
void crear_tablas_de_segundo_nivel(int numeroTablaPrimerNivel);
void inicializar_array_de_tablas_de_primer_nivel(void);
void inicializar_array_de_tablas_de_segundo_nivel(void);
void inicializar_marcos(void);
void* recibir_conexion(int socketEscucha, pthread_t* threadSuscripcion);
void recibir_conexiones(int socketEscucha);

void imprimir_tabla_nivel_uno(void) {
    printf("\n");
    for (int i = 0; i < cantidadProcesosMax; i++) {
        printf("Tabla %d: [ ", i);
        for (int j = 0; j < entradasPorTabla; j++) {
            printf("%d ", tablasNivel1[i].nroTablaNivel2[j]);
        }
        printf("]\n");
    }
    printf("\n");
}
void imprimir_tabla_nivel_dos(void) {
    printf("\n");
    for (int i = 0; i < cantidadProcesosMax; i++) {
        printf("Tabla %d\n", i);
        for (int j = 0; j < entradasPorTabla; j++) {
            printf("Tabla Nivel Dos %d\n", tablasNivel1[i].nroTablaNivel2[j]);
            int numerosTablaNivelDos = tablasNivel1[i].nroTablaNivel2[j];
            for (int r = 0; r < entradasPorTabla; r++) {
                printf("[ %d, %d, %d, %d ]\n", tablasNivel2[numerosTablaNivelDos].entradaNivel2[r].indiceMarco, tablasNivel2[numerosTablaNivelDos].entradaNivel2[r].bitPresencia, tablasNivel2[numerosTablaNivelDos].entradaNivel2[r].bitUso, tablasNivel2[numerosTablaNivelDos].entradaNivel2[r].bitModificado);
            }
        }
        printf("\n\n\n");
    }
    printf("\n");
}
void imprimir_frames(void) {
    printf("\n");
    for (int i = 0; i < cantTotalMarcos; i++) {
        printf("Marco %d: %d\n", i, marcosEnUso[i]);
    }
    printf("\n");
}

int obtener_marco(uint32_t nroDeTabla1, uint32_t nroDeTabla2, uint32_t entradaDeTabla2) {
    int marco = tablasNivel2[nroDeTabla2].entradaNivel2[entradaDeTabla2].indiceMarco;
    if (marco != -1) {
        return marco;
    }

    // TODO swap con algoritmo
    return -1;
}
void dar_marco_cpu(int socket, t_buffer* buffer) {
    uint32_t nroDeTabla1, entradaDeTabla1;

    buffer_unpack(buffer, &nroDeTabla1, sizeof(nroDeTabla1));
    buffer_unpack(buffer, &entradaDeTabla1, sizeof(entradaDeTabla1));
    buffer_destroy(buffer);

    uint32_t nroDeTabla2 = tablasNivel1[nroDeTabla1].nroTablaNivel2[entradaDeTabla1];
    t_buffer* buffer_rta = buffer_create();
    buffer_pack(buffer_rta, &nroDeTabla2, sizeof(nroDeTabla2));
    stream_send_buffer(socket, HEADER_rta_tabla_segundo_nivel, buffer_rta);
    buffer_destroy(buffer_rta);

    uint32_t header = stream_recv_header(socket);
    stream_recv_buffer(socket, buffer);
    uint32_t entradaDeTabla2;
    if (header == HEADER_solicitud_marco) {
        buffer_unpack(buffer, &nroDeTabla2, sizeof(nroDeTabla2));
        buffer_unpack(buffer, &entradaDeTabla2, sizeof(entradaDeTabla2));
        buffer_destroy(buffer);
    } 
    int indiceMarco = obtener_marco(nroDeTabla1, nroDeTabla2, entradaDeTabla2);
    uint32_t marco = indiceMarco * tamanioPagina;
    buffer_pack(buffer_rta, &marco, sizeof(marco));
    stream_send_buffer(socket, HEADER_rta_marco, buffer_rta);
    buffer_destroy(buffer_rta);
}

int main(int argc, char* argv[]) {
    memoriaLogger = log_create(MEMORIA_LOG_PATH, MEMORIA_MODULE_NAME, true, LOG_LEVEL_INFO);
    memoriaConfig = memoria_config_create(MEMORIA_CONFIG_PATH, memoriaLogger);

    int socketEscucha = iniciar_servidor(memoria_config_get_ip_escucha(memoriaConfig), memoria_config_get_puerto_escucha(memoriaConfig));
    log_info(memoriaLogger, "Memoria(%s): A la escucha de Kernel y CPU en puerto %d", __FILE__, socketEscucha);

    int tamanioMemoria = memoria_config_get_tamanio_memoria(memoriaConfig);
    tamanioPagina = memoria_config_get_tamanio_pagina(memoriaConfig);
    marcosPorProceso = memoria_config_get_marcos_por_proceso(memoriaConfig);
    entradasPorTabla = memoria_config_get_entradas_por_tabla(memoriaConfig);
    cantTotalMarcos = tamanioMemoria / tamanioPagina;
    cantidadProcesosMax = tamanioMemoria / (marcosPorProceso * tamanioPagina);

    memoriaPrincipal = malloc(tamanioMemoria);
    memset(memoriaPrincipal, 0, tamanioMemoria);

    inicializar_array_de_tablas_de_primer_nivel();
    inicializar_array_de_tablas_de_segundo_nivel();
    inicializar_marcos();

    crear_nuevo_proceso(5);
    crear_nuevo_proceso(20);
    crear_nuevo_proceso(34);

    imprimir_frames();
    imprimir_tabla_nivel_uno();
    imprimir_tabla_nivel_dos();

    recibir_conexiones(socketEscucha);

    return 0;
}

void* escuchar_peticiones_kernel(void) {
    int socket = memoria_config_get_kernel_socket(memoriaConfig);
    uint32_t header;
    for (;;) {
        header = stream_recv_header(socket);
        t_buffer* buffer = buffer_create();
        stream_recv_buffer(socket, buffer);

        switch (header) {
            case HEADER_solicitud_tabla_paginas: {
                uint32_t tamanio;
                buffer_unpack(buffer, &tamanio, sizeof(tamanio));

                int procesoNuevo = crear_nuevo_proceso(tamanio);

                if (procesoNuevo == -1) {
                    log_error(memoriaLogger, "No se pudo crear el proceso");
                    stream_send_empty_buffer(socket, HEADER_error_proceso_no_creado);
                } else {
                    uint32_t nroTablaNivel1 = procesoNuevo;
                    t_buffer* buffer_rta = buffer_create();
                    buffer_pack(buffer_rta, &nroTablaNivel1, sizeof(nroTablaNivel1));
                    stream_send_buffer(socket, HEADER_tabla_de_paginas, buffer_rta);
                }

                break;
            }
            case HEADER_proceso_suspendido:
                // Liberar memoria del proceso con swap...
                stream_send_empty_buffer(socket, HANDSHAKE_ok_continue);
                break;
            case HEADER_proceso_terminado:
                // Liberar al proceso de memoria y de swap...
                stream_send_empty_buffer(socket, HANDSHAKE_ok_continue);
                break;
            default:
                break;
        }
    }

    return NULL;
}

void* escuchar_peticiones_cpu(void) {
    int socket = memoria_config_get_cpu_socket(memoriaConfig);
    uint32_t header, marco, valor;
    t_buffer* buffer;
    for (;;) {
        header = stream_recv_header(socket);
        buffer = buffer_create();
        stream_recv_buffer(socket, buffer);
        switch (header) {
            case HEADER_read:
                buffer_unpack(buffer, &marco, sizeof(marco));
                memcpy(&valor, memoriaPrincipal + marco, sizeof(valor));
                t_buffer* buffer_rta = buffer_create();
                buffer_pack(buffer_rta, &valor, sizeof(valor));
                stream_send_buffer(socket, HEADER_read, buffer_rta);
                buffer_destroy(buffer_rta);
                break;
            case HEADER_write:
                buffer_unpack(buffer, &marco, sizeof(marco));
                buffer_unpack(buffer, &valor, sizeof(valor));
                memcpy(memoriaPrincipal + marco, &valor, sizeof(valor));
                break;

            case HEADER_copy:
                buffer_unpack(buffer, &marco, sizeof(marco));
                uint32_t marcoOrigen;
                buffer_unpack(buffer, &marcoOrigen, sizeof(marcoOrigen));
                memcpy(memoriaPrincipal + marco, memoriaPrincipal + marcoOrigen, sizeof(uint32_t));
                break;
            case HEADER_solicitud_tabla_segundo_nivel:
                dar_marco_cpu(socket, buffer);
                break;
            default:
                break;
        }
        buffer_destroy(buffer);
    }

    return NULL;
}

void recibir_conexiones(int socketEscucha) {
    pthread_t threadAtencion;
    void* (*funcion_suscripcion)(void) = NULL;
    funcion_suscripcion = recibir_conexion(socketEscucha, &threadAtencion);
    pthread_create(&threadAtencion, NULL, (void*)funcion_suscripcion, NULL);
    pthread_detach(threadAtencion);

    funcion_suscripcion = recibir_conexion(socketEscucha, &threadAtencion);
    pthread_create(&threadAtencion, NULL, (void*)funcion_suscripcion, NULL);
    pthread_join(threadAtencion, NULL);
}

void* recibir_conexion(int socketEscucha, pthread_t* threadSuscripcion) {
    struct sockaddr cliente = {0};
    socklen_t len = sizeof(cliente);
    int socket = accept(socketEscucha, &cliente, &len);

    if (socket == -1) {
        log_error(memoriaLogger, "Error al aceptar conexion de cliente: %s", strerror(errno));
        exit(-1);
    }

    uint8_t handshake = stream_recv_header(socket);
    stream_recv_empty_buffer(socket);

    void* (*funcion_suscripcion)(void) = NULL;
    if (handshake == HANDSHAKE_cpu) {
        log_info(memoriaLogger, "Se acepta conexión de CPU en socket %d", socket);
        stream_send_empty_buffer(socket, HANDSHAKE_ok_continue);
        memoria_config_set_cpu_socket(memoriaConfig, socket);
        funcion_suscripcion = escuchar_peticiones_cpu;
    } else if (handshake == HANDSHAKE_kernel) {
        log_info(memoriaLogger, "Se acepta conexión de Kernel en socket %d", socket);
        stream_send_empty_buffer(socket, HANDSHAKE_ok_continue);
        memoria_config_set_kernel_socket(memoriaConfig, socket);
        funcion_suscripcion = escuchar_peticiones_kernel;
    }
    return funcion_suscripcion;
}

uint32_t crear_nuevo_proceso(uint32_t tamanio) {
    int indiceMarco = obtener_marco_disponible();
    int tamanioMaximo = entradasPorTabla * entradasPorTabla * tamanioPagina;
    if (indiceMarco == -1) {
        return -1;
    } else if (tamanio >= tamanioMaximo) {
        return -1;
    }

    int numeroTablaPrimerNivel = obtener_tabla_primer_nivel_libre();
    crear_tablas_de_segundo_nivel(numeroTablaPrimerNivel);
    asignar_marcos_a_proceso(numeroTablaPrimerNivel, indiceMarco);

    // TODO crear estructura para swap

    return numeroTablaPrimerNivel;
}

void asignar_marcos_a_proceso(int numeroTablaPrimerNivel, int indiceMarco) {
    int cantidadMarcosRestantes = marcosPorProceso;
    int indiceMarcoActual = indiceMarco;
    for (int i = 0; i < entradasPorTabla; i++) {
        for (int j = 0; j < entradasPorTabla; j++) {
            int nroTablaNivel2 = tablasNivel1[numeroTablaPrimerNivel].nroTablaNivel2[i];
            tablasNivel2[nroTablaNivel2].entradaNivel2[j].indiceMarco = indiceMarcoActual;
            tablasNivel2[nroTablaNivel2].entradaNivel2[j].bitPresencia = true;

            cantidadMarcosRestantes--;
            marcosEnUso[indiceMarcoActual++] = true;

            if (cantidadMarcosRestantes == 0) {
                return;
            }
        }
    }
}

int obtener_tabla_primer_nivel_libre(void) {
    int i;
    for (i = 0; i < cantidadProcesosMax; i++) {
        if (tablasNivel1[i].nroTablaNivel2[0] == -1) {
            break;
        }
    }
    return i;
}

void crear_tablas_de_segundo_nivel(int numeroTablaPrimerNivel) {
    for (int i = 0; i < entradasPorTabla; i++) {
        int indiceTablaSegundoNivel = crear_tabla_segundo_nivel();
        tablasNivel1[numeroTablaPrimerNivel].nroTablaNivel2[i] = indiceTablaSegundoNivel;
    }
}

int crear_tabla_segundo_nivel(void) {
    int tablaSegundoNivelLibre = obtener_tabla_segundo_nivel_libre();
    tablasNivel2[tablaSegundoNivelLibre].entradaNivel2 = malloc(entradasPorTabla * sizeof(t_entrada_nivel_dos));
    for (int i = 0; i < entradasPorTabla; i++) {
        tablasNivel2[tablaSegundoNivelLibre].entradaNivel2[i] =
            (t_entrada_nivel_dos){.indiceMarco = -1, .bitPresencia = false, .bitUso = false, .bitModificado = false};
    }

    return tablaSegundoNivelLibre;
}

int obtener_tabla_segundo_nivel_libre(void) {
    int i;
    for (i = 0; i < cantidadProcesosMax * entradasPorTabla; i++) {
        if (tablasNivel2[i].entradaNivel2 == NULL) {
            break;
        }
    }
    return i;
}

void inicializar_array_de_tablas_de_primer_nivel(void) {
    tablasNivel1 = malloc(cantidadProcesosMax * sizeof(t_tabla_nivel_uno));

    for (int i = 0; i < cantidadProcesosMax; i++) {
        tablasNivel1[i].nroTablaNivel2 = malloc(entradasPorTabla * sizeof(int));
        for (int j = 0; j < entradasPorTabla; j++) {
            tablasNivel1[i].nroTablaNivel2[j] = -1;
        }
    }
}

void inicializar_array_de_tablas_de_segundo_nivel(void) {
    tablasNivel2 = calloc(cantidadProcesosMax * entradasPorTabla, sizeof(t_tabla_nivel_dos));
}

void inicializar_marcos(void) {
    marcosEnUso = calloc(cantTotalMarcos, sizeof(bool));
}

int obtener_marco_disponible(void) {
    for (int i = 0; i < cantTotalMarcos; i++) {
        if (!marcosEnUso[i]) {
            return i;
        }
    }
    return -1;
}