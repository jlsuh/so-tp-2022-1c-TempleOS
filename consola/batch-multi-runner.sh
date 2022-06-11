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

declare -r utils_path="./../utils/bin/"
declare -r opts=":n:s:p:"

function pick_random_element() {
  local elems=("$@")
  local index=$((RANDOM % ${#elems[@]}))
  echo "${elems[$index]}"
}

function main() {
  local processes
  local sizes
  local paths

  while getopts $opts opt; do
    case $opt in
    n) processes="$OPTARG" ;;
    s) sizes+=("$OPTARG") ;;
    p) paths+=("$OPTARG") ;;
    esac
  done

  read -r -a sizes <<<"${sizes[@]}"
  read -r -a paths <<<"${paths[@]}"

  export LD_LIBRARY_PATH=$utils_path

  for ((i = 0; i < processes; i++)); do
    random_size=$(pick_random_element "${sizes[@]}")
    random_path=$(pick_random_element "${paths[@]}")
    echo "Running: ./batch-multi-runner.sh $random_size $random_path"
    ./bin/consola.out "$random_size" "$random_path"
  done
}

main "$@"
