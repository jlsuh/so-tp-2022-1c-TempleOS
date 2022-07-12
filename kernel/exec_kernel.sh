#!/usr/bin/env bash

if (($# != 1)); then
    echo -e "\e[1;91mParameters: <configPathname>\e[0m"
    exit 1
fi

CONFIG_PATHNAME=$1

export LD_LIBRARY_PATH="./../utils/bin/"
./bin/kernel.out "$CONFIG_PATHNAME"
