#!/usr/bin/env bash

declare -r COMMONS="so-commons-library"
declare -r DEPENDENCIES=("utils")
declare -r PROJECTS=("consola" "kernel" "memoria" "cpu")
declare -r RULE="release"
declare -r SWAP="swap/"

declare -r ENDCOLOR="\e[0m"
declare -r GREEN="\e[1;92m"

function log_green() { echo -e "${GREEN}$1${ENDCOLOR}"; }

function cd() { command cd "$@" && printf "Changing directory: %s -> %s\n" "${OLDPWD}" "${PWD}"; }

function install_commons() {
    local -r cwd=$1
    local -r commons=$2
    local -r token=$3
    log_green "Installing ${commons}..."
    rm -rf "$commons"
    git clone "https://${token}@github.com/sisoputnfrba/${commons}.git" "$commons"
    cd "$commons" || exit
    sudo make uninstall
    make all
    sudo make install
    cd "$cwd" || exit
}

function install_dependencies() {
    local -r cwd=$1
    shift
    local -r dependencies=("$@")
    for d in "${dependencies[@]}"; do
        log_green "Installing dependency ${d}..."
        cd "$d" || exit
        make install
        cd "$cwd" || exit
    done
}

function build_projects() {
    local -r cwd=$1
    shift
    local -r rule=$1
    shift
    local -r projects=("$@")
    for p in "${projects[@]}"; do
        log_green "Building project ${p} with rule ${rule}..."
        cd "$p" || exit
        make "$rule"
        cd "$cwd" || exit
    done
}

function main() {
    local -r CWD=$PWD

    echo -n "Personal Access Token: "
    read -r TOKEN

    install_commons "$CWD" "$COMMONS" "$TOKEN"
    install_dependencies "$CWD" "${DEPENDENCIES[@]}"
    build_projects "$CWD" "$RULE" "${PROJECTS[@]}"

    cd ..
    sudo mkdir $SWAP || exit
    sudo chmod 777 $SWAP || exit
    log_green "Swap subdirectory created..."

    log_green "Deploy finished"
}

main
