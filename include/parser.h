#pragma once
#include "common.h"
#include "lexer.h"

typedef struct Env Env;

struct Env {
    Var* vars;
    struct Env* outer;
};

typedef enum {
    NODE_NUMBER,
    NODE_IDENT,
    NODE_BINOP,
    NODE_ASSIGN,
    NODE_PRINT,
    NODE_VARDECL,
    NODE_STRING,
    NODE_IF_STMT,
    NODE_FUNC_DEF,
    NODE_FUNC_CALL,
    NODE_RETURN_STMT,
    NODE_BLOCK,
    NODE_WHILE_STMT,
    NODE_FOR_STMT,
    NODE_POST_INC,
    NODE_POST_DEC,
    NODE_ARRAY_LITERAL,
    NODE_ARRAY_ACCESS,
    NODE_ARRAY_ASSIGN,
    NODE_CLASS_DEF,
    NODE_THIS,
    NODE_GET,
    NODE_SET
} NodeKind;

typedef struct Node {
    NodeKind kind;
    TokenKind op;
    
    char name[64];
    double value;
    struct Node* left;
    struct Node* right;
    
    // --- TAMBAHAN BARU ---
    struct Node* next; // Pointer khusus untuk linked list
    // --------------------
    
    int arity;
} Node;

typedef struct {
    Lexer* lexer;
    Token current;
} Parser;

void parser_init(Parser* P, Lexer* L);
Node* parse_stmt(Parser* P);
void free_node(Node* n);
Node *parse(const char *source);