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

t_log* memoriaLogger;
t_memoria_config* memoriaConfig;

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
            case HEADER_nuevo_proceso:
                uint32_t tamanio;
                buffer_unpack(buffer, &tamanio, sizeof(tamanio));
                // Crear estructura para nuevo proceso...
                // Send nro de pagina
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

// Estructuras
t_list* lista_de_tablas_de_paginas;
t_list* lista_de_frames;
void* memoriaPrincipal;

typedef struct {
    uint32_t pid;
    t_list* paginas;
} t_tabla_de_paginas;

typedef struct {
    uint32_t nroFrame;
    uint8_t estaLibre;
} t_frame;

// pthread_mutex_t MUTEX_MP;
// pthread_mutex_t MUTEX_FRAME;
// pthread_mutex_t MUTEX_TABLAS_DE_PAGINAS;
// pthread_mutex_t MUTEX_TLB;

// void iniciar_mutex() {
//     pthread_mutex_init(&MUTEX_MP, NULL);
//     pthread_mutex_init(&MUTEX_FRAME, NULL);
//     pthread_mutex_init(&MUTEX_TLB, NULL);
//     pthread_mutex_init(&MUTEX_TABLAS_DE_PAGINAS, NULL);
// }

// void finalizar_mutex() {
//     pthread_mutex_destroy(&MUTEX_MP);
//     pthread_mutex_destroy(&MUTEX_FRAME);
//     pthread_mutex_destroy(&MUTEX_TLB);
//     pthread_mutex_destroy(&MUTEX_TABLAS_DE_PAGINAS);
// }

void memwrite_pagina_en_frame(uint32_t numeroFrame, uint32_t desplazamiento, void* data, uint32_t size) {
    uint32_t tamanioPagina = memoria_config_get_tamanio_pagina(memoriaConfig);
    uint32_t offset = numeroFrame * tamanioPagina + desplazamiento;
    memcpy(memoriaPrincipal + offset, data, size);
}

void memread_pagina_en_frame(uint32_t numeroFrame, uint32_t desplazamiento, void* data, uint32_t size) {
    uint32_t tamanioPagina = memoria_config_get_tamanio_pagina(memoriaConfig);
    uint32_t offset = numeroFrame * tamanioPagina + desplazamiento;
    memcpy(data, memoriaPrincipal + offset, size);
}

void memcpy_pagina_en_frame(uint32_t numeroFrameDestino, uint32_t numeroFrameOrigen) {
}

void crear_lista_de_tablas_de_paginas() {
    lista_de_tablas_de_paginas = list_create();
}

void crear_lista_de_frames(void* inicio_memoria) {
    lista_de_frames = list_create();
    int cantidadFrames = memoria_config_get_cantidad_frames(memoriaConfig);

    for (int i = 0; i < cantidadFrames; i++) {
        t_frame* frameVacio = malloc(sizeof(t_frame));
        frameVacio->nroFrame = i;
        frameVacio->estaLibre = 1;
        list_add(lista_de_frames, frameVacio);
    }
}

t_frame* encontrar_frame_vacio() {
    t_list* framesVacios = list_create();
    for (int i = 0; i < list_size(lista_de_frames); i++) {
        t_frame* aux = list_get(lista_de_frames, i);
        if (aux->estaLibre == 1) {
            list_add(framesVacios, aux);
        }
    }

    t_frame* frame = malloc(sizeof(t_frame));
    frame = list_get(framesVacios, 0);
    frame->estaLibre = 0;
    return frame;
}

void list_add_tabla_de_paginas(t_tabla_de_paginas* elem) {
    pthread_mutex_lock(&MUTEX_TABLAS_DE_PAGINAS);
    list_add(lista_de_tablas_de_paginas, (void*)elem);
    pthread_mutex_unlock(&MUTEX_TABLAS_DE_PAGINAS);
}

bool tabla_de_paginas_tiene_pid(void* x) {
    t_tabla_de_paginas* elem = x;
    return elem->pid == pid;
}

t_tabla_de_paginas* list_remove_by_pid(uint32_t pid) {
    pthread_mutex_lock(&MUTEX_TABLAS_DE_PAGINAS);
    static_pid = pid;
    t_tabla_de_paginas* elem = list_remove_by_condition(lista_de_tablas_de_paginas, &tabla_de_paginas_tiene_pid);
    pthread_mutex_unlock(&MUTEX_TABLAS_DE_PAGINAS);
    return elem;
}

void free_tabla_de_paginas(void* x) {
    if (x == NULL)
        return;
    t_tabla_de_paginas* elem = (t_tabla_de_paginas*)x;
    list_destroy_and_destroy_elements(elem->paginas, (void*)free);
    free(elem);
}

void finalizar_lista_de_tablas_de_paginas1() {
    pthread_mutex_lock(&MUTEX_TABLAS_DE_PAGINAS);
    list_destroy_and_destroy_elements(lista_de_tablas_de_paginas1, &free_tabla_de_paginas);
    pthread_mutex_unlock(&MUTEX_TABLAS_DE_PAGINAS);
}

void finalizar_lista_de_tablas_de_paginas2() {
    list_destroy_and_destroy_elements(lista_de_tablas_de_paginas2, &free_tabla_de_paginas);
}

void finalizar_lista_de_frames() {
    pthread_mutex_lock(&MUTEX_FRAME);
    list_destroy_and_destroy_elements(lista_de_frames, free);
    pthread_mutex_unlock(&MUTEX_FRAME);
}

int main(int argc, char* argv[]) {
    memoriaLogger = log_create(MEMORIA_LOG_PATH, MEMORIA_MODULE_NAME, true, LOG_LEVEL_INFO);
    memoriaConfig = memoria_config_create(MEMORIA_CONFIG_PATH, memoriaLogger);

    int socketEscucha = iniciar_servidor(memoria_config_get_ip_escucha(memoriaConfig), memoria_config_get_puerto_escucha(memoriaConfig));
    log_info(memoriaLogger, "Memoria(%s): A la escucha de Kernel y CPU en puerto %d", __FILE__, socketEscucha);

    // Estructuras memoria
    memoriaPrincipal = malloc(memoria_config_get_tamanio_memoria(memoriaConfig));
    memset(memoriaPrincipal, 0, memoria_config_get_tamanio_memoria(memoriaConfig));
    crear_lista_de_tablas_de_paginas();
    crear_lista_de_frames(memoriaPrincipal);

    recibir_conexion(socketEscucha);
    recibir_conexion(socketEscucha);

    return 0;
}
