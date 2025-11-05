#ifndef LEXER_H
#define LEXER_H

#include "common.h"

/**
 * @struct TOKEN
 * Represents a token in the Jackal programming language.
 */
typedef struct {
    const char* src;
    size_t pos;
} Lexer;

/**
 * Initializes the lexer with the given source code.
 * @param L Pointer to the Lexer to be initialized.
 * @param src The source code to be lexed.
 */
void lexer_init(Lexer* L, const char* src);

/**
 * Retrieves the next token from the source code.
 * @param L Pointer to the Lexer.
 * @return The next Token.
 */
Token lexer_next(Lexer* L);

#endif
