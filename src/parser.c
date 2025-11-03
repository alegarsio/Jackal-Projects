#include "parser.h"
#include <stdlib.h>
#include <string.h>

static void next(Parser* P) { P->current = lexer_next(P->lexer); }

void parser_init(Parser* P, Lexer* L) {
    P->lexer = L;
    next(P);
}

static Node* new_node(NodeKind kind) {
    Node* n = calloc(1, sizeof(Node));
    n->kind = kind;
    return n;
}
Node *parse(const char *source) {
    Lexer L;
    Parser P;
    lexer_init(&L, source);
    parser_init(&P, &L);

    Node* program_head = new_node(NODE_IDENT); 
    Node* current_stmt = program_head;

    while (P.current.kind != TOKEN_END) {
        Node* stmt = parse_stmt(&P);
        if (stmt) {
            
            current_stmt->right = stmt;
            current_stmt = stmt;
        }
    }

    Node* real_root = program_head->right;
    free(program_head); 
    
    return real_root; 
}

static Node* parse_expr(Parser* P);

static Node* parse_primary(Parser* P) {
    if (P->current.kind == TOKEN_NUMBER) {
        Node* n = new_node(NODE_NUMBER);
        n->value = P->current.number;
        next(P);
        return n;
    }
    if (P->current.kind == TOKEN_IDENT) {
        Node* n = new_node(NODE_IDENT);
        strcpy(n->name, P->current.text);
        next(P);
        return n;
    }
    if (P->current.kind == TOKEN_LPAREN) {
        next(P);
        Node* n = parse_expr(P);
        if (P->current.kind == TOKEN_RPAREN) next(P);
        return n;
    }
    return NULL;
}

static Node* parse_term(Parser* P) {
    Node* left = parse_primary(P);
    while (P->current.kind == TOKEN_STAR || P->current.kind == TOKEN_SLASH) {
        Node* n = new_node(NODE_BINOP);
        n->name[0] = P->current.text[0];
        next(P);
        n->left = left;
        n->right = parse_primary(P);
        left = n;
    }
    return left;
}

static Node* parse_expr(Parser* P) {
    Node* left = parse_term(P);
    while (P->current.kind == TOKEN_PLUS || P->current.kind == TOKEN_MINUS) {
        Node* n = new_node(NODE_BINOP);
        n->name[0] = P->current.text[0];
        next(P);
        n->left = left;
        n->right = parse_term(P);
        left = n;
    }
    return left;
}

Node* parse_stmt(Parser* P) {
    if (P->current.kind == TOKEN_LET) {
        next(P);
        Node* n = new_node(NODE_VARDECL);
        strcpy(n->name, P->current.text);
        next(P); 
        if (P->current.kind == TOKEN_ASSIGN) next(P);
        n->right = parse_expr(P);
        if (P->current.kind == TOKEN_SEMI) next(P);
        return n;
    }

    if (P->current.kind == TOKEN_PRINT) {
        next(P);
        Node* n = new_node(NODE_PRINT);
        n->right = parse_expr(P);
        if (P->current.kind == TOKEN_SEMI) next(P);
        return n;
    }

    Node* expr = parse_expr(P);
    if (P->current.kind == TOKEN_SEMI) next(P);
    return expr;
}

void free_node(Node* n) {
    if (!n) return;
    free_node(n->left);
    free_node(n->right);
    free(n);
}
