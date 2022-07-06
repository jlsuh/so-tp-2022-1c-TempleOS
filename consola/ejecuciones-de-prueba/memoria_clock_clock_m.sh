#!/usr/bin/env bash

# Actividades:
# 1) Iniciar los módulos
# 2) Ejecutar 1 módulo consola con el siguiente script indicado:
export LD_LIBRARY_PATH=./../utils/bin
./bin/consola.out 4096 ./instrucciones-de-prueba/MEMORIA_1
# 3) Esperar su finalización
# 4) Cambiar el algoritmo de reemplazo a Clock-M y volver a ejecutar los scripts en el mismo orden

# Resultados esperados:
# *) El proceso MEMORIA_1 finaliza correctamente y tiene un paso por el estado de suspendido
# *) En el caso de Clock-M, se cambiarán otras páginas y se tendrán menos accesos a Swap y por lo tanto tardará menos en finalizar el proceso

# Ejemplo de uso:
# Ubicado en subdirectorio consola, ejecutar: bash ejecuciones-de-prueba/memoria_clock_clock_m.sh