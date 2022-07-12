#!/usr/bin/env bash

# Actividades:
# 1) Iniciar los módulos
# 2) Ejecutar 3 módulos consola con los siguientes scripts en el orden indicado:
if (($# != 3)); then
    echo -e "\e[1;91mParameters: <tamanioPrimerConsola> <tamanioSegundaConsola> <tamanioTercerConsola>\e[0m"
    exit 1
fi

export LD_LIBRARY_PATH=./../utils/bin
./bin/consola.out "$1" ./instrucciones-de-prueba/SUSPE_1 &
./bin/consola.out "$2" ./instrucciones-de-prueba/SUSPE_2 &
./bin/consola.out "$3" ./instrucciones-de-prueba/SUSPE_3
# 3) Esperar su finalización
# 4) Cambiar el algoritmo a SRT y volver a ejecutar como los scripts en el mismo orden

# Resultados esperados:
# *) Todos los procesos finalizan correctamente
# *) Se suspenden los procesos SUSPE_1 y SUSPE_2
# *) En el caso de SRT, la de-suspensión de los mismos forzarán una re-planificación y los scripts finalizarán en el orden que fueron ejecutados

# Ejemplo de uso:
# Ubicado en subdirectorio consola, ejecutar: bash ejecuciones-de-prueba/kernel_suspension.sh 200 400 350