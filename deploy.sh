#!/usr/bin/env bash

declare -r COMMONS="so-commons-library"
declare -r DEPENDENCIES=("utils")
declare -r PROJECTS=("consola" "kernel" "memoria" "cpu")
declare -r RULE="release"
declare -r SWAP="swap/"

declare -r ENDCOLOR="\e[0m"
declare -r GREEN="\e[1;92m"
declare -r RED="\e[1;91m"

function cd() { command cd "$@" && printf "Changing directory: %s -> %s\n" "$OLDPWD" "$PWD"; }

function fail() { exit 1; }

function log_green() { echo -e "${GREEN}${1}${ENDCOLOR}"; }

function install_commons() {
    local -r cwd=$1
    local -r commons=$2
    local -r token=$3
    log_green "Installing ${commons}..."
    rm -rf "$commons"
    git clone "https://${token}@github.com/sisoputnfrba/${commons}.git" "$commons"
    cd "$commons" || fail
    sudo make uninstall
    make all
    sudo make install
    cd "$cwd" || fail
}

function install_dependencies() {
    local -r cwd=$1
    shift
    local -r dependencies=("$@")
    for d in "${dependencies[@]}"; do
        log_green "Installing dependency ${d}..."
        cd "$d" || fail
        make install
        cd "$cwd" || fail
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
        cd "$p" || fail
        make "$rule"
        cd "$cwd" || fail
    done
}

function main() {
    if (($# != 1)); then
        echo -e "${RED}Parameters: <PersonalAccessToken>${ENDCOLOR}"
        fail
    fi

    local -r CWD=$PWD
    local -r TOKEN=$1

    install_commons "$CWD" "$COMMONS" "$TOKEN"
    install_dependencies "$CWD" "${DEPENDENCIES[@]}"
    build_projects "$CWD" "$RULE" "${PROJECTS[@]}"

    cd ..
    sudo mkdir $SWAP || fail
    sudo chmod 777 $SWAP || fail
    log_green "Swap subdirectory created..."

    log_green "Deploy finished"
}

main "$@"
