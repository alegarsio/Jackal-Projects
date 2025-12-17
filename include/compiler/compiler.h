#ifndef JACKAL_COMPILER_H
#define JACKAL_COMPILER_H

#include "parser.h"
#include "vm/chunk.h"
#include <stdbool.h>

bool compile(Node* program_ast, Chunk* chunk);
void compile_to_binary(Node* root, const char* path); // Fungsi dari main.c

#endif