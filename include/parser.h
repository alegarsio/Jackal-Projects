#pragma once
#include "common.h"
#include "lexer.h"


typedef enum {
    NODE_NUMBER,
    NODE_IDENT,
    NODE_BINOP,
    NODE_ASSIGN,
    NODE_PRINT,
    NODE_VARDECL
} NodeKind;

typedef struct Node {
    NodeKind kind;
    char name[64];
    double value;
    struct Node* left;
    struct Node* right;
} Node;

typedef struct {
    Lexer* lexer;
    Token current;
} Parser;

void parser_init(Parser* P, Lexer* L);
Node* parse_stmt(Parser* P);
void free_node(Node* n);


Node *parse(const char *source);

