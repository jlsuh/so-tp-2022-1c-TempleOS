# Original project path
PROJ_PATH=../cpu

# Excluded source files (eg: main() function)
EXCLUDE=cpu.c

# Libraries
LIBS=cunit utils commons

# Custom libraries' paths
SHARED_LIBPATHS=../utils
STATIC_LIBPATHS=

# Compiler flags
CDEBUG=-g -Wall -DDEBUG
CRELEASE=-O3 -Wall -DNDEBUG

# Arguments when executing with start, memcheck or helgrind
ARGS=

# Valgrind flags
MEMCHECK_FLAGS=--track-origins=yes --log-file="memcheck.log"
HELGRIND_FLAGS=--log-file="helgrind.log"
