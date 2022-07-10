#!/bin/bash

echo "Personal Access Token: "
read -r TOKEN

cd() { command cd "$@" && printf 'Changing directory: %s -> %s\n' "${OLDPWD}" "${PWD}"; }

bold=$(tput bold)
normal=$(tput sgr0)
underline=$(tput smul)
nounderline=$(tput rmul)

if [[ "$@" =~ (-h|-H|--help) ]]; then
    echo "
${bold}NAME${normal}
    ${bold}deploy.sh${normal} - script to deploy sisoputnfrba's TP.
${bold}SYNOPSIS${normal}
    ${bold}deploy.sh${normal} [${bold}--lib${normal}=${underline}library${nounderline}] [${bold}--dependency${normal}=${underline}dependency${nounderline}] [${bold}--project${normal}=${underline}project${nounderline}] ${underline}repository${nounderline}
${bold}DESCRIPTION${normal}
    The ${bold}deploy.sh${normal} utility is to ease the deploy process.
    The options are as follows:
    ${bold}-t | --target${normal}       Changes the directory where the script is executed. By default it will be the current directory.
    ${bold}-m | --make${normal}         Changes the makefile rule for building projects. By default it will be empty.
    ${bold}-l | --lib${normal}          Adds an external dependency to build and install.
    ${bold}-d | --dependency${normal}   Adds an internal dependency to build and install from the repository.
    ${bold}-p | --project${normal}      Adds a project to build from the repository.
${bold}COMPATIBILITY${normal}
    The project must have the following scructure:
      project1/
       ╰─ makefile
      project2/
       ╰─ makefile
      sharedlibrary/
       ╰─ makefile
${bold}EXAMPLE${normal}
      ${bold}deploy.sh${normal} ${bold}-l${normal}=sisoputnfrba/ansisop-parser ${bold}-d${normal}=sockets ${bold}-p${normal}=consola ${bold}-p${normal}=kernel ${bold}-p${normal}=memoria ${underline}tp-2017-1C-exampleRepo${nounderline}
  "
    exit
fi

CWD=$PWD
case $1 in
-t=* | --target=*)
    case ${1#*=} in
    /*) CWD="${1#*=}" ;;
    *) CWD+="/${1#*=}" ;;
    esac
    shift
    ;;
*) ;;

esac
cd $CWD

RULE=""
case $1 in
-m=* | --make=*)
    RULE="${1#*=}"
    shift
    ;;
*) ;;

esac

echo -e "\n\nInstalling commons libraries...\n\n"

COMMONS="so-commons-library"

rm -rf $COMMONS
git clone "https://${TOKEN}@github.com/sisoputnfrba/${COMMONS}.git" $COMMONS
cd $COMMONS
sudo make uninstall
make all
sudo make install
cd $CWD

length=$(($# - 1))
OPTIONS=${@:1:length}
REPONAME="${!#}"

LIBRARIES=()
DEPENDENCIES=()
PROJECTS=()

for i in $OPTIONS; do
    case $i in
    -l=* | --lib=*)
        LIBRARIES+=("${i#*=}")
        ;;
    -d=* | --dependency=*)
        DEPENDENCIES+=("${i#*=}")
        ;;
    -p=* | --project=*)
        PROJECTS+=("${i#*=}")
        ;;
    *) ;;

    esac
done

if [ ${#LIBRARIES[@]} -ne 0 ]; then
    echo -e "\n\nCloning external libraries..."
    for i in "${LIBRARIES[@]}"; do
        echo -e "\n\nBuilding ${i}\n\n"
        rm -rf $i
        git clone "https://${TOKEN}@github.com/${i}.git" $i
        cd $i
        make install
        cd $CWD
    done
else
    echo -e "\n\nNo external libraries were indicated..."
fi

if [ -n "${REPONAME}" ]; then
    echo -e "\n\nCloning project repo...\n\n"
    rm -rf $REPONAME
    git clone "https://${TOKEN}@github.com/sisoputnfrba/${REPONAME}.git" $REPONAME
    cd $REPONAME
else
    echo -e "\n\nNo repository was indicated..."
fi

if [ ${#DEPENDENCIES[@]} -ne 0 ]; then
    echo -e "\n\nBuilding dependencies..."
    for i in "${DEPENDENCIES[@]}"; do
        echo -e "\n\nBuilding ${i}\n\n"
        cd $i
        make install
        cd $PROJECTROOT
    done
else
    echo -e "\n\nNo dependencies were indicated..."
fi

if [ ${#PROJECTS[@]} -ne 0 ]; then
    echo -e "\n\nBuilding projects..."
    for i in "${PROJECTS[@]}"; do
        echo -e "\n\nBuilding ${i}\n\n"
        cd $i
        make $RULE
        cd $PROJECTROOT
    done
else
    echo -e "\n\nNo projects were indicated..."
fi

echo -e "\n\nDeploy finished\n\n"
