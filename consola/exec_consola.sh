#!/usr/bin/env bash

if (($# != 2)); then
    echo -e "\e[1;91mParameters: <tamanioConsola> <configPathname>\e[0m"
    exit 1
fi

SIZE=$1
CONFIG_PATHNAME=$2

export LD_LIBRARY_PATH="./../utils/bin/"
./bin/consola.out "$SIZE" "$CONFIG_PATHNAME"
