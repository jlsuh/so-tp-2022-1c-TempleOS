# Libraries
LIBS=utils commons

# Custom libraries' paths
SHARED_LIBPATHS=../utils
STATIC_LIBPATHS=

# Compiler flags
CDEBUG=-g -Wall -DDEBUG
CRELEASE=-O3 -Wall -DNDEBUG

# Arguments when executing with start, memcheck or helgrind
ARGS= 500 /home/juani/Github/testfile

# Valgrind flags
MEMCHECK_FLAGS=--track-origins=yes --log-file="memcheck.log"
HELGRIND_FLAGS=--log-file="helgrind.log"
