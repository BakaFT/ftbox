#ifndef PARSER_H
#define PARSER_H

#include "../common.h"
#include <argp.h>


void ParseCommandline(int argc, char** argv, RunConfig* config);

#endif // PARSER_H