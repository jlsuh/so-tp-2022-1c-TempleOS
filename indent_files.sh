#!/usr/bin/env bash

readarray -t array < <(find . -name \*.h -print -o -name \*.c -print)

for i in "${array[@]}"; do
    echo -e "\n\e[1;92m>>>>>>>>>>>>>>>>>>>> Indenting ($i) <<<<<<<<<<<<<<<<<<<<\e[0m"
    clang-format -style=file "$i"
done
