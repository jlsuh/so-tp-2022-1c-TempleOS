#!/usr/bin/env bash

declare -r RULE="release"
declare -r GREEN="\e[1;92m"
declare -r ENDCOLOR="\e[0m"
declare -r COMMONS="so-commons-library"
declare -r DEPENDENCIES=("utils")
declare -r PROJECTS=("consola" "kernel" "memoria" "cpu")

function cd() { command cd "$@" && printf 'Changing directory: %s -> %s\n' "${OLDPWD}" "${PWD}"; }

function install_commons_library() {
    echo -e "${GREEN}Installing commons libraries...${ENDCOLOR}\n"
    rm -rf $COMMONS
    git clone "https://${TOKEN}@github.com/sisoputnfrba/${COMMONS}.git" $COMMONS
    cd $COMMONS || exit
    sudo make uninstall
    make all
    sudo make install
    cd "$CWD" || exit
}

function build_dependencies() {
    echo -e "${GREEN}Building dependencies...${ENDCOLOR}\n"
    for i in "${DEPENDENCIES[@]}"; do
        echo -e "${GREEN}Building dependency ${i}${ENDCOLOR}\n"
        cd "$i" || exit
        make install
        cd "$CWD" || exit
    done
}

function build_projects() {
    echo -e "${GREEN}Building projects...${ENDCOLOR}\n"
    for i in "${PROJECTS[@]}"; do
        echo -e "${GREEN}Building project ${i}${ENDCOLOR}\n"
        cd "$i" || exit
        make "$RULE"
        cd "$CWD" || exit
    done
}

function main() {
    local -r CWD=$PWD

    echo -n "Personal Access Token: "
    read -r TOKEN

    install_commons_library
    build_dependencies
    build_projects

    cd ..
    sudo mkdir "swap/" || exit
    sudo chmod 777 "swap/" || exit
    echo -e "${GREEN}Swap subdirectory created...${ENDCOLOR}\n"

    echo -e "${GREEN}Deploy finished${ENDCOLOR}\n"
}

main
