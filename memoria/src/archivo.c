#include "archivo.h"

#include <commons/string.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

void abrir_archivo(uint32_t tamanio, int nroTablaNivel1, t_memoria_data_holder memoriaData) {
    char* pathArchivo = string_from_format("%s/%d.swap", memoriaData.pathSwap, nroTablaNivel1);
    memoriaData.archivo_swap = open(pathArchivo, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
    free(pathArchivo);

    ftruncate(memoriaData.archivo_swap, tamanio);

    memoriaData.inicio_archivo = mmap(NULL, tamanio, PROT_READ | PROT_WRITE, MAP_SHARED, memoriaData.archivo_swap, 0);
    if (memoriaData.inicio_archivo == MAP_FAILED) {
        puts("error en mmap");
    }
}

void crear_archivo_de_proceso(uint32_t tamanio, int nroTablaNivel1, t_memoria_data_holder memoriaData) {
    abrir_archivo(tamanio, nroTablaNivel1, memoriaData);
    memset(memoriaData.inicio_archivo, '\0', tamanio); //TODO esta generando problemas en runtime
    cerrar_archivo(memoriaData);
}

void cerrar_archivo(t_memoria_data_holder memoriaData) {
    free(memoriaData.inicio_archivo);
    close(memoriaData.archivo_swap);
}

void eliminar_archivo_de_proceso(int nroTablaNivel1, t_memoria_data_holder memoriaData){
    char* pathArchivo = string_from_format("%s/%d.swap", memoriaData.pathSwap, nroTablaNivel1);
    if(remove(pathArchivo) == -1)
        perror("Error in deleting a file");
}