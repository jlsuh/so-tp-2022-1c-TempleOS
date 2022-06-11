#!/usr/bin/env bash
ctags -o - --kinds-C=f -x --_xformat="%{typeref} %{name}%{signature};" $1 | tr ':' ' ' | sed -e 's/^typename //'

# Ejemplo de uso:
# bash ctags.sh ./unModulo/src/archivoQueQuieroExtraerLasDeclaraciones.c
