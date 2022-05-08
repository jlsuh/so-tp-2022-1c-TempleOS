#include <commons/collections/list.h>
#include <commons/log.h>
#include <commons/string.h>
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/socket.h>
#include <sys/stat.h>
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
    uint32_t tamanio;
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
void* inicio_archivo;
int archivo_swap;
t_tabla_nivel_uno* tablasDeNivel1;
t_tabla_nivel_dos* tablasDeNivel2;
bool* marcosEnUso;
int cantTotalMarcos;
int marcosPorProceso;
int cantidadProcesosMax;
int entradasPorTabla;
int tamanioPagina;

int crear_tabla_de_nivel_2(void);
int main(int argc, char* argv[]);
int obtener_marco(uint32_t nroDeTabla2, uint32_t entradaDeTabla2);
int obtener_marco_libre(void);
int obtener_tabla_libre_de_nivel_1(void);
int obtener_tabla_libre_de_nivel_2(void);
uint32_t crear_nuevo_proceso(uint32_t tamanio);
void* escuchar_peticiones_cpu(void);
void* escuchar_peticiones_kernel(void);
void* recibir_conexion(int socketEscucha, pthread_t* threadSuscripcion);
void asignar_marcos_a_proceso(int nroTablaNivel1, int indiceMarco);
void crear_tablas_de_nivel_2(int nroTablaNivel1);
void dar_marco_cpu(int socket, t_buffer* buffer);
void imprimir_frames(void);
void imprimir_tabla_nivel_dos(void);
void imprimir_tabla_nivel_uno(void);
void inicializar_marcos(void);
void inicializar_tablas_de_nivel_1(void);
void inicializar_tablas_de_nivel_2(void);
void recibir_conexiones(int socketEscucha);
void imprimir_memoria(void);
void crear_archivo_de_proceso(int nroTablaNivel1);
void abrir_archivo(int nroTablaNivel1);
void cerrar_archivo(void);

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

    inicializar_tablas_de_nivel_1();
    inicializar_tablas_de_nivel_2();
    inicializar_marcos();

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
                    stream_send_empty_buffer(socket, HEADER_error);
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
            case HEADER_tabla_nivel_2: {
                uint32_t nroDeTabla1, entradaDeTabla1;
                buffer_unpack(buffer, &nroDeTabla1, sizeof(nroDeTabla1));
                buffer_unpack(buffer, &entradaDeTabla1, sizeof(entradaDeTabla1));

                uint32_t nroDeTabla2 = tablasDeNivel1[nroDeTabla1].nroTablaNivel2[entradaDeTabla1];
                t_buffer* buffer_rta = buffer_create();
                buffer_pack(buffer_rta, &nroDeTabla2, sizeof(nroDeTabla2));
                stream_send_buffer(socket, HEADER_tabla_nivel_2, buffer_rta);
                buffer_destroy(buffer_rta);

                break;
            }
            case HEADER_marco: {
                uint32_t nroDeTabla2, entradaDeTabla2;
                buffer_unpack(buffer, &nroDeTabla2, sizeof(nroDeTabla2));
                buffer_unpack(buffer, &entradaDeTabla2, sizeof(entradaDeTabla2));

                int indiceMarco = obtener_marco(nroDeTabla2, entradaDeTabla2);
                marco = indiceMarco * tamanioPagina;
                t_buffer* buffer_rta = buffer_create();
                buffer_pack(buffer_rta, &marco, sizeof(marco));
                stream_send_buffer(socket, HEADER_marco, buffer_rta);
                buffer_destroy(buffer_rta);

                break;
            }
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

uint32_t obtener_nro_de_tabla_1(uint32_t nroDeTabla2){
    for(int i = 0; i < cantidadProcesosMax; i++){
        for(int j = 0; j < entradasPorTabla; j++){
            if(tablasDeNivel1[i].nroTablaNivel2[j] == nroDeTabla2){
                return i;
            }
        }
    }
}

int swap_marco(int nroDeTabla2ToSwap, int entradaDeTabla2ToSwap, uint32_t nroDeTabla2, uint32_t entradaDeTabla2) {
    uint32_t nroDeTabla1 = obtener_nro_de_tabla_1(nroDeTabla2);
    abrir_archivo(nroDeTabla1);
    int marco = tablasDeNivel2[nroDeTabla2ToSwap].entradaNivel2[entradaDeTabla2ToSwap].indiceMarco;
    tablasDeNivel2[nroDeTabla2ToSwap].entradaNivel2[entradaDeTabla2ToSwap].bitPresencia = 0;

    int paginaToSwap = nroDeTabla2ToSwap * entradasPorTabla + entradaDeTabla2ToSwap;
    int pagina = nroDeTabla2 * entradasPorTabla + entradaDeTabla2;
    int tiempoDeEspera = memoria_config_get_retardo_swap(memoriaConfig);

    // Escribir en archivo
    sleep(tiempoDeEspera);
    memcpy(inicio_archivo + sizeof(uint32_t) * paginaToSwap, memoriaPrincipal + marco * tamanioPagina, sizeof(uint32_t));
    // Leer de archivo
    sleep(tiempoDeEspera);
    memcpy(memoriaPrincipal + marco * tamanioPagina, inicio_archivo + sizeof(uint32_t) * pagina, sizeof(uint32_t));

    tablasDeNivel2[nroDeTabla2].entradaNivel2[entradaDeTabla2].indiceMarco = marco;
    tablasDeNivel2[nroDeTabla2].entradaNivel2[entradaDeTabla2].bitPresencia = 1;
    // TODO se necesita hacer algo con los bits del algoritmo?

    cerrar_archivo();
    return marco;
}

int obtener_marco(uint32_t nroDeTabla2, uint32_t entradaDeTabla2) {
    int marco = tablasDeNivel2[nroDeTabla2].entradaNivel2[entradaDeTabla2].indiceMarco;
    if (marco != -1) {
        return marco;
    }

    // int nroDeTabla2ToSwap;
    // int entradaDeTabla2ToSwap = seleccionar_marco_a_reemplazar(nroDeTabla1, &nroDeTabla2ToSwap); // TODO
    int nroDeTabla2ToSwap = 0, entradaDeTabla2ToSwap = 0;  // TODO Borrar, solo para que no tire error el compilador
    marco = swap_marco(nroDeTabla2ToSwap, entradaDeTabla2ToSwap, nroDeTabla2, entradaDeTabla2);

    return marco;
}

uint32_t crear_nuevo_proceso(uint32_t tamanio) {
    int indiceMarco = obtener_marco_libre();
    int tamanioMaximo = entradasPorTabla * entradasPorTabla * tamanioPagina;
    if (indiceMarco == -1) {
        return -1;
    } else if (tamanio >= tamanioMaximo) {
        return -1;
    }

    int nroTablaNivel1 = obtener_tabla_libre_de_nivel_1();
    crear_tablas_de_nivel_2(nroTablaNivel1);
    asignar_marcos_a_proceso(nroTablaNivel1, indiceMarco);
    tablasDeNivel1[nroTablaNivel1].tamanio = tamanio;

    crear_archivo_de_proceso(nroTablaNivel1);

    return (uint32_t)nroTablaNivel1;
}

void asignar_marcos_a_proceso(int nroTablaNivel1, int indiceMarco) {
    int cantMarcosRestantes = marcosPorProceso;
    int indiceMarcoActual = indiceMarco;
    for (int i = 0; i < entradasPorTabla; i++) {
        for (int j = 0; j < entradasPorTabla; j++) {
            int nroTablaNivel2 = tablasDeNivel1[nroTablaNivel1].nroTablaNivel2[i];
            tablasDeNivel2[nroTablaNivel2].entradaNivel2[j].indiceMarco = indiceMarcoActual;
            tablasDeNivel2[nroTablaNivel2].entradaNivel2[j].bitPresencia = true;

            cantMarcosRestantes--;
            marcosEnUso[indiceMarcoActual++] = true;

            if (cantMarcosRestantes == 0) {
                return;
            }
        }
    }
}

void crear_tablas_de_nivel_2(int nroTablaNivel1) {
    for (int i = 0; i < entradasPorTabla; i++) {
        int indiceTablaSegundoNivel = crear_tabla_de_nivel_2();
        tablasDeNivel1[nroTablaNivel1].nroTablaNivel2[i] = indiceTablaSegundoNivel;
    }
}

int crear_tabla_de_nivel_2(void) {
    int tablaSegundoNivelLibre = obtener_tabla_libre_de_nivel_2();
    tablasDeNivel2[tablaSegundoNivelLibre].entradaNivel2 = malloc(entradasPorTabla * sizeof(t_entrada_nivel_dos));
    for (int i = 0; i < entradasPorTabla; i++) {
        tablasDeNivel2[tablaSegundoNivelLibre].entradaNivel2[i] =
            (t_entrada_nivel_dos){.indiceMarco = -1, .bitPresencia = false, .bitUso = false, .bitModificado = false};
    }

    return tablaSegundoNivelLibre;
}

int obtener_tabla_libre_de_nivel_1(void) {
    int i;
    for (i = 0; i < cantidadProcesosMax; i++) {
        if (tablasDeNivel1[i].nroTablaNivel2[0] == -1) {
            break;
        }
    }
    return i;
}

int obtener_tabla_libre_de_nivel_2(void) {
    int i;
    for (i = 0; i < cantidadProcesosMax * entradasPorTabla; i++) {
        if (tablasDeNivel2[i].entradaNivel2 == NULL) {
            break;
        }
    }
    return i;
}

int obtener_marco_libre(void) {
    for (int i = 0; i < cantTotalMarcos; i++) {
        if (!marcosEnUso[i]) {
            return i;
        }
    }
    return -1;
}

void inicializar_tablas_de_nivel_1(void) {
    tablasDeNivel1 = malloc(cantidadProcesosMax * sizeof(t_tabla_nivel_uno));

    for (int i = 0; i < cantidadProcesosMax; i++) {
        tablasDeNivel1[i].nroTablaNivel2 = malloc(entradasPorTabla * sizeof(int));
        tablasDeNivel1[i].tamanio = 0;
        for (int j = 0; j < entradasPorTabla; j++) {
            tablasDeNivel1[i].nroTablaNivel2[j] = -1;
        }
    }
}

void inicializar_tablas_de_nivel_2(void) {
    tablasDeNivel2 = calloc(cantidadProcesosMax * entradasPorTabla, sizeof(t_tabla_nivel_dos));
}

void inicializar_marcos(void) {
    marcosEnUso = calloc(cantTotalMarcos, sizeof(bool));
}

// Funciones archivo

void abrir_archivo(int nroTablaNivel1) {
    uint32_t tamanio = tablasDeNivel1[nroTablaNivel1].tamanio;
    char* pathArchivo = string_from_format("%s/%d.swap", memoria_config_get_path_swap(memoriaConfig), nroTablaNivel1);
    archivo_swap = open(pathArchivo, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
    free(pathArchivo);

    ftruncate(archivo_swap, tamanio);

    inicio_archivo = mmap(NULL, tamanio, PROT_READ | PROT_WRITE, MAP_SHARED, archivo_swap, 0);
    if (inicio_archivo == MAP_FAILED) {
        puts("error en mmap");
    }
}

void crear_archivo_de_proceso(int nroTablaNivel1) {
    uint32_t tamanio = tablasDeNivel1[nroTablaNivel1].tamanio;
    abrir_archivo(nroTablaNivel1);
    memset(inicio_archivo, '\0', tamanio);
    cerrar_archivo();
}

void cerrar_archivo() {
    free(inicio_archivo);
    close(archivo_swap);
}

// Funciones de trackeo

void imprimir_tabla_nivel_uno(void) {
    printf("\n");
    for (int i = 0; i < cantidadProcesosMax; i++) {
        printf("Tabla %d: [ ", i);
        for (int j = 0; j < entradasPorTabla; j++) {
            printf("%d ", tablasDeNivel1[i].nroTablaNivel2[j]);
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
            printf("Tabla Nivel Dos %d\n", tablasDeNivel1[i].nroTablaNivel2[j]);
            int numerosTablaNivelDos = tablasDeNivel1[i].nroTablaNivel2[j];
            for (int r = 0; r < entradasPorTabla; r++) {
                printf("[ %d, %d, %d, %d ]\n", tablasDeNivel2[numerosTablaNivelDos].entradaNivel2[r].indiceMarco, tablasDeNivel2[numerosTablaNivelDos].entradaNivel2[r].bitPresencia, tablasDeNivel2[numerosTablaNivelDos].entradaNivel2[r].bitUso, tablasDeNivel2[numerosTablaNivelDos].entradaNivel2[r].bitModificado);
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

void imprimir_memoria(void) {
    uint32_t valor = 2561;
    uint32_t valor2 = 423;
    uint32_t valor3 = 543;
    uint32_t valor4 = 6540;
    uint32_t valor5 = 12;
    memcpy(memoriaPrincipal + tamanioPagina * 5, &valor, sizeof(uint32_t));
    memcpy(memoriaPrincipal + tamanioPagina * 10, &valor2, sizeof(uint32_t));
    memcpy(memoriaPrincipal + tamanioPagina * 23, &valor3, sizeof(uint32_t));
    memcpy(memoriaPrincipal + tamanioPagina * 2, &valor4, sizeof(uint32_t));
    memcpy(memoriaPrincipal, &valor5, sizeof(uint32_t));

    printf("\n");
    for (int i = 0; i < cantTotalMarcos; i++) {
        printf("Marco %d: con valor %d\n", i, *((uint32_t*)(memoriaPrincipal + i * tamanioPagina)));
    }
    printf("\n");
}
