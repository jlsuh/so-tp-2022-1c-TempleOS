#!/bin/sh
ctags -o - --kinds-C=f -x --_xformat="%{typeref} %{name}%{signature};" $1 | tr ':' ' ' | sed -e 's/^typename //'

# Ejemplo de uso:
# sh ctags.sh ./unModulo/src/archivoQueQuieroExtraerLasDeclaraciones.c
