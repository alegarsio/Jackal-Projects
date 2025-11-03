#ifndef LEXER_H
#define LEXER_H

#include "common.h"

typedef struct {
    const char* src;
    size_t pos;
} Lexer;

void lexer_init(Lexer* L, const char* src);
Token lexer_next(Lexer* L);

#endif
