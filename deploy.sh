#!/usr/bin/env bash

declare -r COMMONS="so-commons-library"
declare -r DEPENDENCIES=("utils")
declare -r PROJECTS=("consola" "kernel" "memoria" "cpu")
declare -r RULE="release"
declare -r SWAP="swap/"

declare -r ENDCOLOR="\e[0m"
declare -r GREEN="\e[1;92m"

function log_green() { echo -e "${GREEN}$1${ENDCOLOR}"; }

function cd() { command cd "$@" && printf 'Changing directory: %s -> %s\n' "${OLDPWD}" "${PWD}"; }

function install_commons() {
    log_green "Installing ${COMMONS}..."
    rm -rf $COMMONS
    git clone "https://${TOKEN}@github.com/sisoputnfrba/${COMMONS}.git" $COMMONS
    cd $COMMONS || exit
    sudo make uninstall
    make all
    sudo make install
    cd "$CWD" || exit
}

function install_dependencies() {
    for i in "${DEPENDENCIES[@]}"; do
        log_green "Installing dependency ${i}..."
        cd "$i" || exit
        make install
        cd "$CWD" || exit
    done
}

function build_projects() {
    for i in "${PROJECTS[@]}"; do
        log_green "Building project ${i} with rule ${RULE}..."
        cd "$i" || exit
        make "$RULE"
        cd "$CWD" || exit
    done
}

function main() {
    local -r CWD=$PWD

    echo -n "Personal Access Token: "
    read -r TOKEN

    install_commons
    install_dependencies
    build_projects

    cd ..
    sudo mkdir $SWAP || exit
    sudo chmod 777 $SWAP || exit
    log_green "Swap subdirectory created..."

    log_green "Deploy finished"
}

main
