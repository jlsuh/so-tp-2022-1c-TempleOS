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
int obtener_marco_disponible();
int obtener_tabla_primer_nivel_libre();
int obtener_tabla_segundo_nivel_libre();
int crear_tabla_segundo_nivel();
uint32_t crear_nuevo_proceso(uint32_t tamanio);
void* escuchar_peticiones_cpu(void);
void* escuchar_peticiones_kernel(void);
void asignar_marcos_a_proceso(int numeroTablaPrimerNivel, int indiceMarco);
void crear_hilo_handler_peticiones_de_cpu(void);
void crear_hilo_handler_peticiones_de_kernel(void);
void crear_tablas_de_segundo_nivel(int numeroTablaPrimerNivel);
void inicializar_array_de_tablas_de_primer_nivel();
void inicializar_array_de_tablas_de_segundo_nivel();
void iniciar_marcos();
void recibir_conexion(int socketEscucha);

void imprimir_tabla_nivel_uno() {
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
void imprimir_tabla_nivel_dos() {
    printf("\n");
    for (int i = 0; i < cantidadProcesosMax; i++) {
        printf("Tabla %d\n", i);
        for (int j = 0; j < entradasPorTabla; j++) {
            printf("Tabla Nivel Dos %d\n", tablasNivel1[i].nroTablaNivel2[j]);
            for (int r = 0; r < entradasPorTabla; r++) {
                printf("[ %d, %d, %d, %d ]\n", tablasNivel2[j].entradaNivel2[r].indiceMarco, tablasNivel2[j].entradaNivel2[r].bitPresencia, tablasNivel2[j].entradaNivel2[r].bitUso, tablasNivel2[j].entradaNivel2[r].bitModificado);
            }
        }
        printf("\n\n\n");
    }
    printf("\n");
}

void imprimir_frames() {
    printf("\n");
    for (int i = 0; i < cantTotalMarcos; i++) {
        printf("Marco %d: %d\n", i, marcosEnUso[i]);
    }
    printf("\n");
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

    memoriaPrincipal = malloc(tamanioMemoria);
    memset(memoriaPrincipal, 0, tamanioMemoria);
    cantidadProcesosMax = tamanioMemoria / (marcosPorProceso * tamanioPagina);

    inicializar_array_de_tablas_de_primer_nivel();
    inicializar_array_de_tablas_de_segundo_nivel();
    iniciar_marcos();

    crear_nuevo_proceso(5);
    crear_nuevo_proceso(20);
    crear_nuevo_proceso(34);

    imprimir_frames();
    imprimir_tabla_nivel_uno();
    imprimir_tabla_nivel_dos();

    recibir_conexion(socketEscucha);
    recibir_conexion(socketEscucha);

    return 0;
}

void* escuchar_peticiones_kernel(void) {
    int socket = memoria_config_get_kernel_socket(memoriaConfig);
    uint32_t header;
    for (;;) {
        header = stream_recv_header(socket);
        t_buffer* buffer = buffer_create();
        stream_recv_buffer(socket, buffer);
        uint32_t pid;
        buffer_unpack(buffer, &pid, sizeof(pid));

        switch (header) {
            case HEADER_solicitud_tabla_paginas:
                uint32_t tamanio;
                buffer_unpack(buffer, &tamanio, sizeof(tamanio));
                int procesoNuevo = crear_nuevo_proceso(tamanio);
                if (procesoNuevo == -1) {
                    log_error(memoriaLogger, "No se pudo crear el proceso %d", pid);
                    // Enviar error
                } else {
                    uint32_t numeroDeTablaDePrimerNivel = procesoNuevo;
                    t_buffer* buffer_respuesta = buffer_create();
                    buffer_pack(buffer_respuesta, &numeroDeTablaDePrimerNivel, sizeof(numeroDeTablaDePrimerNivel));
                    stream_send_buffer(socket, HEADER_tabla_de_paginas, buffer_respuesta);
                }
                break;
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
    uint32_t header;

    for (;;) {
        header = stream_recv_header(socket);
        t_buffer* buffer = buffer_create();
        stream_recv_buffer(socket, buffer);
        uint32_t pid;
        buffer_unpack(buffer, &pid, sizeof(pid));
        uint32_t direccion_logica;
        buffer_unpack(buffer, &direccion_logica, sizeof(direccion_logica));
        switch (header) {
            case HEADER_read:
                // Devolver el valor que hay en la dirección logica
                break;
            case HEADER_write:
                uint32_t valor;
                buffer_unpack(buffer, &valor, sizeof(valor));
                // Leer el la direccion, traducirla
                // Y mandar a memoria la direccion traducida y los datos a guardar
                stream_send_empty_buffer(socket, HANDSHAKE_ok_continue);
                break;
            case HEADER_copy:
                uint32_t direccion_logica_destino;
                buffer_unpack(buffer, &direccion_logica_destino, sizeof(direccion_logica_destino));
                /*Se deberá escribir en memoria el valor ubicado en la dirección lógica pasada
                como segundo parámetro, en la dirección lógica pasada como primer parámetro.
                A efectos de esta etapa, el accionar es similar a la instrucción WRITE ya que el valor
                a escribir ya se debería haber obtenido en la etapa anterior*/
                stream_send_empty_buffer(socket, HANDSHAKE_ok_continue);
                break;
            default:
                break;
        }
    }

    return NULL;
}

void crear_hilo_handler_peticiones_de_kernel(void) {
    pthread_t threadSuscripcion;
    pthread_create(&threadSuscripcion, NULL, (void*)escuchar_peticiones_kernel, NULL);
    pthread_detach(threadSuscripcion);
}

void crear_hilo_handler_peticiones_de_cpu(void) {
    pthread_t threadSuscripcion;
    pthread_create(&threadSuscripcion, NULL, (void*)escuchar_peticiones_cpu, NULL);
    pthread_detach(threadSuscripcion);
}

void recibir_conexion(int socketEscucha) {
    struct sockaddr cliente = {0};
    socklen_t len = sizeof(cliente);
    int socket = accept(socketEscucha, &cliente, &len);
    if (socket == -1) {
        log_error(memoriaLogger, "Error al aceptar conexion de cliente: %s", strerror(errno));
        exit(-1);
    }

    uint8_t handshake = stream_recv_header(socket);
    if (handshake == HANDSHAKE_cpu) {
        log_info(memoriaLogger, "Se acepta conexión de CPU en socket %d", socket);
        stream_send_empty_buffer(socket, HANDSHAKE_ok_continue);
        memoria_config_set_cpu_socket(memoriaConfig, socket);
        crear_hilo_handler_peticiones_de_cpu();
    } else if (handshake == HANDSHAKE_kernel) {
        log_info(memoriaLogger, "Se acepta conexión de Kernel en socket %d", socket);
        stream_send_empty_buffer(socket, HANDSHAKE_ok_continue);
        memoria_config_set_kernel_socket(memoriaConfig, socket);
        crear_hilo_handler_peticiones_de_kernel();
    }
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

int obtener_tabla_primer_nivel_libre() {
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

int crear_tabla_segundo_nivel() {
    int tablaSegundoNivelLibre = obtener_tabla_segundo_nivel_libre();
    tablasNivel2[tablaSegundoNivelLibre].entradaNivel2 = malloc(entradasPorTabla * sizeof(t_entrada_nivel_dos));
    for (int i = 0; i < entradasPorTabla; i++) {
        tablasNivel2[tablaSegundoNivelLibre].entradaNivel2[i] =
            (t_entrada_nivel_dos){.indiceMarco = -1, .bitPresencia = false, .bitUso = false, .bitModificado = false};
    }

    return tablaSegundoNivelLibre;
}

int obtener_tabla_segundo_nivel_libre() {
    int i;
    for (i = 0; i < cantidadProcesosMax * entradasPorTabla; i++) {
        if (tablasNivel2[i].entradaNivel2 == NULL) {
            break;
        }
    }
    return i;
}

void inicializar_array_de_tablas_de_primer_nivel() {
    tablasNivel1 = malloc(cantidadProcesosMax * sizeof(t_tabla_nivel_uno));

    for (int i = 0; i < cantidadProcesosMax; i++) {
        tablasNivel1[i].nroTablaNivel2 = malloc(entradasPorTabla * sizeof(int));
        for (int j = 0; j < entradasPorTabla; j++) {
            tablasNivel1[i].nroTablaNivel2[j] = -1;
        }
    }
}

void inicializar_array_de_tablas_de_segundo_nivel() {
    tablasNivel2 = calloc(cantidadProcesosMax * entradasPorTabla, sizeof(t_tabla_nivel_dos));
}

void iniciar_marcos() {
    marcosEnUso = calloc(cantTotalMarcos, sizeof(bool));
}

int obtener_marco_disponible() {
    for (int i = 0; i < cantTotalMarcos; i++) {
        if (!marcosEnUso[i]) {
            return i;
        }
    }
    return -1;
}

// void memwrite_pagina_en_Marco(uint32_t numeroMarco, uint32_t desplazamiento, void* data, uint32_t size) {
//     uint32_t tamanioPagina = memoria_config_get_tamanio_pagina(memoriaConfig);
//     uint32_t offset = numeroMarco * tamanioPagina + desplazamiento;
//     memcpy(memoriaPrincipal + offset, data, size);
// }

// void memread_pagina_en_Marco(uint32_t numeroMarco, uint32_t desplazamiento, void* data, uint32_t size) {
//     uint32_t tamanioPagina = memoria_config_get_tamanio_pagina(memoriaConfig);
//     uint32_t offset = numeroMarco * tamanioPagina + desplazamiento;
//     memcpy(data, memoriaPrincipal + offset, size);
// }

// void memcpy_pagina_en_Marco(uint32_t numeroMarcoDestino, uint32_t numeroMarcoOrigen) {
// }