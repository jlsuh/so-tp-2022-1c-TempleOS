#include "archivo.h"
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h> // TODO necesario?
#include <sys/types.h> // TODO necesario?

extern void* inicio_archivo;
extern int archivo_swap;

void abrir_archivo(uint32_t tamanio, char* pathSwap, int nroTablaNivel1) {
    char* pathArchivo = string_from_format("%s/%d.swap", pathSwap, nroTablaNivel1);
    archivo_swap = open(pathArchivo, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
    free(pathArchivo);

    ftruncate(archivo_swap, tamanio);

    inicio_archivo = mmap(NULL, tamanio, PROT_READ | PROT_WRITE, MAP_SHARED, archivo_swap, 0);
    if (inicio_archivo == MAP_FAILED) {
        puts("error en mmap");
    }
}

void crear_archivo_de_proceso(uint32_t tamanio, char* pathSwap, int nroTablaNivel1) {
    abrir_archivo(tamanio, pathSwap, nroTablaNivel1);
    memset(inicio_archivo, '\0', tamanio);
    cerrar_archivo();
}

void cerrar_archivo() {
    free(inicio_archivo);
    close(archivo_swap);
}
