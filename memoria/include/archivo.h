#ifndef ARCHIVO_H_INCLUDED
#define ARCHIVO_H_INCLUDED

#include <stdint.h>

void abrir_archivo(uint32_t tamanio, char* pathSwap, int nroTablaNivel1);
void crear_archivo_de_proceso(uint32_t tamanio, char* pathSwap, int nroTablaNivel1);
void cerrar_archivo();

#endif /* ARCHIVO_H_INCLUDED */