#!/usr/bin/env bash

# Actividades:
# 1) Iniciar los módulos
# 2) Ejecutar 3 módulos consola con los siguientes scripts en el orden indicado:
export LD_LIBRARY_PATH=./../utils/bin
./bin/consola.out $1 ./instrucciones-de-prueba/BASE_1
./bin/consola.out $2 ./instrucciones-de-prueba/BASE_2
./bin/consola.out $3 ./instrucciones-de-prueba/BASE_2
# 3) Esperar su finalización

# Resultados esperados:
# *) Todos los procesos finalizan correctamente

# Ejemplo de uso:
# Ubicado en subdirectorio consola, ejecutar: bash ./ejecuciones-de-prueba/prueba_base.sh 200 400 350