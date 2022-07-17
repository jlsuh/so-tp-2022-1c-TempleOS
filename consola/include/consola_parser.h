#ifndef CONSOLA_PARSER_H_INCLUDED
#define CONSOLA_PARSER_H_INCLUDED

#include <commons/log.h>

#include "buffer.h"

bool consola_parser_parse_instructions(t_buffer *, const char *pathInstrucciones, t_log *);

#endif
