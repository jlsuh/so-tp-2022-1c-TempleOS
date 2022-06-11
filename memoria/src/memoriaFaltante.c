// #include <commons/collections/list.h>
// #include <commons/log.h>
// #include <commons/string.h>
// #include <errno.h>
// #include <fcntl.h>
// #include <pthread.h>
// #include <stdint.h>
// #include <stdio.h>
// #include <stdlib.h>
// #include <string.h>
// #include <sys/mman.h>
// #include <sys/socket.h>
// #include <sys/stat.h>
// #include <unistd.h>

// #include "algoritmos.h"
// #include "common_flags.h"
// #include "connections.h"
// #include "memoria_config.h"
// #include "stream.h"

// #define MEMORIA_CONFIG_PATH "cfg/memoria_config.cfg"
// #define MEMORIA_LOG_PATH "bin/memoria.log"
// #define MEMORIA_MODULE_NAME "Memoria"

// typedef struct
// {
//     uint32_t tamanio;
//     int* nroTablaNivel2;
// } t_tabla_nivel_uno;
// typedef struct
// {
//     int indiceMarco;
//     bool bitPresencia;
//     bool bitUso;
//     bool bitModificado;
// } t_entrada_nivel_dos;
// typedef struct
// {
//     t_entrada_nivel_dos* entradaNivel2;
// } t_tabla_nivel_dos;

// typedef struct
// {
//     bool bitPresencia;
//     bool bitUso;
//     bool bitModificado;
// } t_entrada_pagina_suspendida;

// typedef struct {
//     uint32_t pid;
//     uint32_t tamanio;
//     t_entrada_pagina_suspendida* paginasSuspendidas;
// } t_proceso_suspendido;

// t_log* memoriaLogger;
// t_memoria_config* memoriaConfig;
// void* memoriaPrincipal;
// void* inicio_archivo;
// int archivo_swap;
// t_tabla_nivel_uno* tablasDeNivel1;
// t_tabla_nivel_dos* tablasDeNivel2;
// t_proceso_suspendido* tablaSuspendidos;
// bool* marcosEnUso;
// int cantTotalMarcos;
// int marcosPorProceso;
// int cantidadProcesosMax;
// int entradasPorTabla;
// int tamanioPagina;

// int crear_tabla_de_nivel_2(void);
// int main(int argc, char* argv[]);
// int obtener_marco(uint32_t nroDeTabla2, uint32_t entradaDeTabla2);
// int obtener_marco_libre(void);
// int obtener_tabla_libre_de_nivel_1(void);
// int obtener_tabla_libre_de_nivel_2(void);
// uint32_t crear_nuevo_proceso(uint32_t tamanio);
// void* escuchar_peticiones_cpu(void);
// void* escuchar_peticiones_kernel(void);
// void* recibir_conexion(int socketEscucha, pthread_t* threadSuscripcion);
// void asignar_marcos_a_proceso(int nroTablaNivel1, int indiceMarco);
// void crear_tablas_de_nivel_2(int nroTablaNivel1);
// void dar_marco_cpu(int socket, t_buffer* buffer);
// void imprimir_frames(void);
// void imprimir_tabla_nivel_dos(void);
// void imprimir_tabla_nivel_uno(void);
// void inicializar_marcos(void);
// void inicializar_tablas_de_nivel_1(void);
// void inicializar_tablas_de_nivel_2(void);
// void recibir_conexiones(int socketEscucha);
// void imprimir_memoria(void);
// void crear_archivo_de_proceso(int nroTablaNivel1);
// void abrir_archivo(int nroTablaNivel1);
// void cerrar_archivo(void);

// int main(int argc, char* argv[]) {
//     memoriaLogger = log_create(MEMORIA_LOG_PATH, MEMORIA_MODULE_NAME, true, LOG_LEVEL_INFO);
//     memoriaConfig = memoria_config_create(MEMORIA_CONFIG_PATH, memoriaLogger);

//     int socketEscucha = iniciar_servidor(memoria_config_get_ip_escucha(memoriaConfig), memoria_config_get_puerto_escucha(memoriaConfig));
//     log_info(memoriaLogger, "Memoria(%s): A la escucha de Kernel y CPU en puerto %d", __FILE__, socketEscucha);

//     int tamanioMemoria = memoria_config_get_tamanio_memoria(memoriaConfig);
//     tamanioPagina = memoria_config_get_tamanio_pagina(memoriaConfig);
//     marcosPorProceso = memoria_config_get_marcos_por_proceso(memoriaConfig);
//     entradasPorTabla = memoria_config_get_entradas_por_tabla(memoriaConfig);
//     cantTotalMarcos = tamanioMemoria / tamanioPagina;
//     cantidadProcesosMax = tamanioMemoria / (marcosPorProceso * tamanioPagina);

//     memoriaPrincipal = malloc(tamanioMemoria);
//     memset(memoriaPrincipal, 0, tamanioMemoria);

//     inicializar_tablas_de_nivel_1();
//     inicializar_tablas_de_nivel_2();
//     inicializar_marcos();
//     // TODO inicializar tabla suspendidos

//     recibir_conexiones(socketEscucha);

//     return 0;
// }

// int swap_marco(int nroDeTabla2ToSwap, int entradaDeTabla2ToSwap, uint32_t nroDeTabla2, uint32_t entradaDeTabla2) {
//     uint32_t nroDeTabla1 = obtener_nro_de_tabla_1(nroDeTabla2);
//     abrir_archivo(nroDeTabla1);
//     int marco = tablasDeNivel2[nroDeTabla2ToSwap].entradaNivel2[entradaDeTabla2ToSwap].indiceMarco;
//     tablasDeNivel2[nroDeTabla2ToSwap].entradaNivel2[entradaDeTabla2ToSwap].bitPresencia = 0;

//     int paginaToSwap = nroDeTabla2ToSwap * entradasPorTabla + entradaDeTabla2ToSwap;
//     int pagina = nroDeTabla2 * entradasPorTabla + entradaDeTabla2;
//     int tiempoDeEspera = memoria_config_get_retardo_swap(memoriaConfig);

//     // Escribir en archivo
//     sleep(tiempoDeEspera);
//     memcpy(inicio_archivo + tamanioPagina * paginaToSwap, memoriaPrincipal + marco * tamanioPagina, tamanioPagina);
//     // Leer de archivo
//     sleep(tiempoDeEspera);
//     memcpy(memoriaPrincipal + marco * tamanioPagina, inicio_archivo + tamanioPagina * pagina, tamanioPagina);

//     tablasDeNivel2[nroDeTabla2].entradaNivel2[entradaDeTabla2].indiceMarco = marco;
//     tablasDeNivel2[nroDeTabla2].entradaNivel2[entradaDeTabla2].bitPresencia = 1;
//     // TODO se necesita hacer algo con los bits del algoritmo?

//     cerrar_archivo();
//     return marco;
// }

// int obtener_marco(uint32_t nroDeTabla2, uint32_t entradaDeTabla2) {
//     int marco = tablasDeNivel2[nroDeTabla2].entradaNivel2[entradaDeTabla2].indiceMarco;
//     if (marco != -1) {
//         return marco;
//     }

//     // int nroDeTabla2ToSwap;
//     // int entradaDeTabla2ToSwap = seleccionar_marco_a_reemplazar(nroDeTabla1, &nroDeTabla2ToSwap); // TODO
//     int nroDeTabla2ToSwap = 0, entradaDeTabla2ToSwap = 0;  // TODO Borrar, solo para que no tire error el compilador
//     marco = swap_marco(nroDeTabla2ToSwap, entradaDeTabla2ToSwap, nroDeTabla2, entradaDeTabla2);

//     return marco;
// }



// void asignar_marcos_a_proceso(int nroTablaNivel1, int indiceMarco) {
//     int cantMarcosRestantes = marcosPorProceso;
//     int indiceMarcoActual = indiceMarco;
//     for (int i = 0; i < entradasPorTabla; i++) {
//         for (int j = 0; j < entradasPorTabla; j++) {
//             int nroTablaNivel2 = tablasDeNivel1[nroTablaNivel1].nroTablaNivel2[i];
//             tablasDeNivel2[nroTablaNivel2].entradaNivel2[j].indiceMarco = indiceMarcoActual;
//             tablasDeNivel2[nroTablaNivel2].entradaNivel2[j].bitPresencia = true;

//             cantMarcosRestantes--;
//             marcosEnUso[indiceMarcoActual++] = true;

//             if (cantMarcosRestantes == 0) {
//                 return;
//             }
//         }
//     }
// }

// // Funciones archivo


// //---------------------ALGORITMOS-------------------------------------------

// int seleccionar_victima_clock(int paginaPuntero, uint32_t nroDeTabla1) {
//     int indiceNivel1 = obtener_indice_nivel_uno(paginaPuntero);
//     int indiceNivel2 = obtener_indice_nivel_dos(paginaPuntero);

//     for (int i = indiceNivel1; i < entradasPorTabla; i++) {
//         int nroTabla2 = tablasDeNivel1[nroDeTabla1].nroTablaNivel2[i];

//         for (int j = indiceNivel2; j < entradasPorTabla; j++) {
//             bool bitPresencia = tablasDeNivel2[nroTabla2].entradaNivel2[j].bitPresencia;
//             bool bitUso = tablasDeNivel2[nroTabla2].entradaNivel2[j].bitUso;

//             if (bitPresencia && !bitUso) {
//                 log_trace(memoriaLogger, "Se elige la pagina victima");
//                 int pagina_victima = obtener_pagina(i, j);
//                 paginaPuntero++; // TODO: ver donde guardar
//                 return pagina_victima;
//             }

//             if (bitPresencia && bitUso) {
//                 log_trace(memoriaLogger, "Cambiando bitUso de la pagina");
//                 tablasDeNivel2[nroTabla2].entradaNivel2[j].bitUso = false;
//             }
//         }

//         if (i == entradasPorTabla) {
//             i = 0;
//         }
//     }

//     return -1;
// }

// int seleccionar_victima_clock_modificado(int paginaPuntero, uint32_t nroDeTabla1) {
//     int cantidad_paginas_proceso = obtener_paginas_proceso(); 
//     int paginas_leidas = 0;

//     int indiceNivel1 = obtener_indice_nivel_uno(paginaPuntero);
//     int indiceNivel2 = obtener_indice_nivel_dos(paginaPuntero);

//     // PRIMERA BUSQUEDA
//     for (int i = indiceNivel1; i < entradasPorTabla; i++) {
//         int nroTabla2 = tablasDeNivel1[nroDeTabla1].nroTablaNivel2[i];

//         for (int j = indiceNivel2; j < entradasPorTabla; j++) {
//             bool bitPresencia = tablasDeNivel2[nroTabla2].entradaNivel2[j].bitPresencia;
//             bool bitUso = tablasDeNivel2[nroTabla2].entradaNivel2[j].bitUso;
//             bool bitModificado = tablasDeNivel2[nroTabla2].entradaNivel2[j].bitModificado;

//             if (bitPresencia && !bitUso && !bitModificado) {
//                 log_trace(memoriaLogger, "Se elige la pagina victima");
//                 int pagina_victima = obtener_pagina(i, j);
//                 paginaPuntero++; //TODO: guardar este valor en la estructura que corresponda
//                 return pagina_victima;
//             }

//             paginas_leidas++;
//             if (paginas_leidas == cantidad_paginas_proceso) {
//                 break;
//             }
//         }

//         if (i == entradasPorTabla) {
//             i = 0;
//         }

//         if (paginas_leidas == cantidad_paginas_proceso) {
//             break;
//         }
//     }

//     paginas_leidas = 0;

//     // SEGUNDA BUSQUEDA
//     for (int i = indiceNivel1; i < entradasPorTabla; i++) {
//         int nroTabla2 = tablasDeNivel1[nroDeTabla1].nroTablaNivel2[i];

//         for (int j = indiceNivel2; j < entradasPorTabla; j++) {
//             bool bitPresencia = tablasDeNivel2[nroTabla2].entradaNivel2[j].bitPresencia;
//             bool bitUso = tablasDeNivel2[nroTabla2].entradaNivel2[j].bitUso;
//             bool bitModificado = tablasDeNivel2[nroTabla2].entradaNivel2[j].bitModificado;

//             if (bitPresencia && !bitUso && bitModificado) {
//                 log_trace(memoriaLogger, "Se elige la pagina victima");
//                 int pagina_victima = obtener_pagina(i, j);
//                 paginaPuntero++; //TODO: ver donde guardar este valor
//                 return pagina_victima;
//             }

//             log_trace(memoriaLogger, "Cambiando bitUso de la pagina");
//             tablasDeNivel2[nroTabla2].entradaNivel2[j].bitUso = false;

//             paginas_leidas++;
//             if (paginas_leidas == cantidad_paginas_proceso) {
//                 break;
//             }
//         }

//         if (i == entradasPorTabla) {
//             i = 0;
//         }

//         if (paginas_leidas == cantidad_paginas_proceso) {
//             break;
//         }
//     }

//     return -1;
// }

// int obtener_pagina(int indiceNivel1, int indiceNivel2) {
//     int pagina;
//     pagina = indiceNivel1 * entradasPorTabla + indiceNivel2;
//     return pagina;
// }

// int obtener_indice_tabla_uno(int paginaGlobal){
//     return paginaGlobal / cantidadProcesosMax;
// }

// int obtener_indice_tabla_dos(int pagina) {
//     int entradaNivel1;
//     int residuo = pagina % entradasPorTabla;
//     int entero = pagina / entradasPorTabla;
//     entradaNivel1 = entero - residuo;
//     return entradaNivel1;
// }

// int obtener_indice_entrada(int pagina) {
//     int entradaNivel2;

//     int entero = obtener_indice_tabla_dos(pagina);
//     entradaNivel2 = pagina - (entero * entradasPorTabla);
//     return entradaNivel2;
// }

// int obtener_paginas_proceso(){
//     int paginas_proceso;

//     paginas_proceso = entradasPorTabla * entradasPorTabla;

//     return paginas_proceso;
// }



// //-----------------------------SUSPENSION PROCESO-------------------------------

// void suspender_proceso(uint32_t nroDeTabla1) {
//     int contPagSuspend = 0;
//     abrir_archivo(nroDeTabla1);

//     // TODO guardar tamaño y pid en tabla suspendidos. Si es lista crear. Si es array buscar vacio y colocar
//     // TODO crear la cantidad de entradasPorTabla*entradasPorTabla  de paginas para la suspension.
//     for (int i = 0; i < entradasPorTabla; i++) {
//         int nroDeTabla2 = tablasDeNivel1[nroDeTabla1].nroTablaNivel2[i];

//         for (int j = 0; j < entradasPorTabla; j++) {
//             bool bitPresencia = tablasDeNivel2[nroDeTabla2].entradaNivel2[j].bitPresencia;
//             bool bitUso = tablasDeNivel2[nroDeTabla2].entradaNivel2[j].bitUso;
//             bool bitModificado = tablasDeNivel2[nroDeTabla2].entradaNivel2[j].bitModificado;
//             int marco = tablasDeNivel2[nroDeTabla2].entradaNivel2[j].indiceMarco;

//             // TODO lista de paginas suspendidas
//             tablaSuspendidos[nroDeTabla1].paginasSuspendidas[contPagSuspend].bitPresencia = bitPresencia;
//             tablaSuspendidos[nroDeTabla1].paginasSuspendidas[contPagSuspend].bitUso = bitUso;
//             tablaSuspendidos[nroDeTabla1].paginasSuspendidas[contPagSuspend].bitModificado = bitModificado;
//             contPagSuspend++;

//             if (bitPresencia && bitModificado) {  // TODO bitmodificado entonces aunque tengamos algoritmo de clock hay que cambiar el bit de modificado cuando corresponda.
//                 log_trace(memoriaLogger, "La pagina esta en memoria");
//                 int pagina = nroDeTabla2 * entradasPorTabla + j;
//                 memcpy(inicio_archivo + tamanioPagina * pagina, memoriaPrincipal + marco * tamanioPagina, tamanioPagina);
//                 memset(memoriaPrincipal + marco * tamanioPagina, 0, tamanioPagina);
//             }
//         }
//         tablasDeNivel1[nroDeTabla1].nroTablaNivel2[i] = -1;
//         // TODO liberar cada tabla de nivel 2 - con un solo free() es suficiente?
//         free(tablasDeNivel2[nroDeTabla2].entradaNivel2);  // TODO asi? O eliminar entrada por entrada?
//     }
//     cerrar_archivo();
// }

// // ---------------------------- DESPERTAR PROCESO -----------------------------
// // TODO usuar crear proceso, y luego pisar en la tabla recibida por crear proceso los bits con los de la tabla de suspendidos y luego liberar la tabla de suspendidos.
// // TODO asigna los marcos a los bit de presencia 1 trayendo del archivo la pagina correspondiente.