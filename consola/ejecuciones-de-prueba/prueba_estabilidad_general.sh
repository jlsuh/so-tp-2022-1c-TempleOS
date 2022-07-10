#!/usr/bin/env bash

# Actividades:
# 1) Iniciar los módulos
# 2) Ejecutar 5 módulos consola con los siguientes scripts en el orden indicado:
SIZE=2048
export LD_LIBRARY_PATH=./../utils/bin
./bin/consola.out $SIZE ./instrucciones-de-prueba/INTEGRAL_1 &
./bin/consola.out $SIZE ./instrucciones-de-prueba/INTEGRAL_2 &
./bin/consola.out $SIZE ./instrucciones-de-prueba/INTEGRAL_3 &
./bin/consola.out $SIZE ./instrucciones-de-prueba/INTEGRAL_4 &
./bin/consola.out $SIZE ./instrucciones-de-prueba/INTEGRAL_5
# 3) Esperar su finalización

# Resultados esperados:
# *) Todos los procesos finalizan

# Ejemplo de uso:
# Ubicado en subdirectorio consola, ejecutar: bash ejecuciones-de-prueba/prueba_estabilidad_general.sh