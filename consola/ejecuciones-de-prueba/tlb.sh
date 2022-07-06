#!/usr/bin/env bash

# Actividades:
# 1) Iniciar los módulos
# 2) Ejecutar 2 módulos consola con los siguientes scripts en el orden indicado:
SIZE=2048
export LD_LIBRARY_PATH=./../utils/bin
./bin/consola.out $SIZE ./instrucciones-de-prueba/TLB_1
./bin/consola.out $SIZE ./instrucciones-de-prueba/TLB_2
# 3) Esperar su finalización
# 4) Cambiar el algoritmo de la TLB a LRU y volver a ejecutar los scripts en el mismo orden

# Resultados esperados:
# *) Ambos procesos finalizan correctamente pero el programa 1 ejecuta IO lo que da espacio a la entrada del proceso 2 a la CPU
# *) En el caso de LRU, se tendrán menos accesos a memoria antes de ejecutar la I/O del proceso TLB_1 y por lo tanto tardará menos en finalizar

# Ejemplo de uso:
# Ubicado en subdirectorio consola, ejecutar: bash ejecuciones-de-prueba/tlb.sh