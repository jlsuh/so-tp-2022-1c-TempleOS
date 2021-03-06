#include "archivo.h"

#include <commons/string.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

void abrir_archivo(uint32_t tamanio, int nroTablaNivel1, t_memoria_data_holder* memoriaData) {
    char* pathArchivo = string_from_format("%s/%d.swap", memoriaData->pathSwap, nroTablaNivel1);
    memoriaData->archivo_swap = open(pathArchivo, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
    if (memoriaData->archivo_swap == -1)
        log_error(memoriaData->memoriaLogger, "Error al abrir el archivo swap: %s", strerror(errno));
    free(pathArchivo);
    if (ftruncate(memoriaData->archivo_swap, tamanio) == -1) {
        log_error(memoriaData->memoriaLogger, "Error al truncar el archivo swap: %s", strerror(errno));
        exit(-1);
    }
    memoriaData->inicio_archivo = mmap(NULL, tamanio, PROT_READ | PROT_WRITE, MAP_SHARED, memoriaData->archivo_swap, 0);
    if (memoriaData->inicio_archivo == MAP_FAILED)
        log_error(memoriaData->memoriaLogger, "Error al mapear el archivo swap: %s", strerror(errno));
}

void crear_archivo_de_proceso(uint32_t tamanio, int nroTablaNivel1, t_memoria_data_holder* memoriaData) {
    abrir_archivo(tamanio, nroTablaNivel1, memoriaData);
    memset(memoriaData->inicio_archivo, '\0', tamanio);
    cerrar_archivo(tamanio, memoriaData);
}

void cerrar_archivo(uint32_t tamanio, t_memoria_data_holder* memoriaData) {
    munmap(memoriaData->inicio_archivo, tamanio);
    close(memoriaData->archivo_swap);
}

void eliminar_archivo_de_proceso(int nroTablaNivel1, t_memoria_data_holder* memoriaData) {
    char* pathArchivo = string_from_format("%s/%d.swap", memoriaData->pathSwap, nroTablaNivel1);
    if (remove(pathArchivo) == -1)
        log_error(memoriaData->memoriaLogger, "Error al eliminar el archivo swap: %s", strerror(errno));
    free(pathArchivo);
}