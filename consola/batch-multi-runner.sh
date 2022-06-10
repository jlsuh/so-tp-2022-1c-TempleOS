#!/usr/bin/env bash

# Ejecutar desde subdirectorio de Consola:
# bash batch-multi-runner.sh -n <cantidadDeProcesosAEjecutar> -s <listaDeTamañosPosibles> -p <pseudoAssemblersQuePuedenAdoptar>

# Ejemplo:
# bash batch-multi-runner.sh -n 100 -s "100 250 300" -p "./instrucciones-de-prueba/BASE_1 ./instrucciones-de-prueba/SUSPE_2"

# En caso de querer ejecutar mediante:
# ./batch-multi-runner.sh -n <cantidadDeProcesosAEjecutar> -s <listaDeTamañosPosibles> -p <pseudoAssemblersQuePuedenAdoptar>
# En vez de:
# bash batch-multi-runner.sh -n <cantidadDeProcesosAEjecutar> -s <listaDeTamañosPosibles> -p <pseudoAssemblersQuePuedenAdoptar>
# Puede que la ejecución falle por falta de permisos de ejecución a los scripts.
# En dicho caso ejecutar (desde subdirectorio Consola):
# chmod -v u+x batch-multi-runner.sh

pick_random_element() {
  local array=("$@")
  local index=$((RANDOM % ${#array[@]}))
  echo "${array[$index]}"
}

main() {
  local OPTS=":n:s:p:"

  local PROCESSES
  local SIZES
  local PATHS

  while getopts $OPTS opt
  do
    case $opt in
      n) PROCESSES+=("$OPTARG") ;;
      s) SIZES+=("$OPTARG") ;;
      p) PATHS+=("$OPTARG") ;;
    esac
  done

  PROCESSES=($PROCESSES)
  SIZES=($SIZES)
  PATHS=($PATHS)

  export LD_LIBRARY_PATH="./../utils/bin/"

  for ((i = 0; i < $PROCESSES; i++))
  do
    random_size=$(pick_random_element "${SIZES[@]}")
    random_path=$(pick_random_element "${PATHS[@]}")
    echo "Running: ./batch-multi-runner.sh $random_size $random_path"
    ./bin/consola.out $random_size $random_path
  done
}

main "$@"
