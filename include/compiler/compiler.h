#ifndef COMPILER_H
#define COMPILER_H

#include "../parser.h"

void compile_to_binary(Node* ast, const char* filename);

#endif