#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

/* Token kinds */
typedef enum {
    TOKEN_END, TOKEN_NUMBER, TOKEN_IDENT,
    TOKEN_LET, TOKEN_PRINT,
    TOKEN_PLUS, TOKEN_MINUS, TOKEN_STAR, TOKEN_SLASH,
    TOKEN_ASSIGN, TOKEN_LPAREN, TOKEN_RPAREN, TOKEN_SEMI,
    TOKEN_INVALID
} TokenKind;

typedef struct Var {
    char name[64];
    double value;
    struct Var* next;
} Var;

void print_error(const char *message);

/* Token structure */
typedef struct {
    TokenKind kind;
    char text[64];
    double number;
} Token;

/* Expression kinds */
typedef enum {
    EXPR_NUMBER, EXPR_VAR, EXPR_BINARY
} ExprKind;

typedef struct Expr Expr;

struct Expr {
    ExprKind kind;
    union {
        double value;
        char name[64];
        struct {
            char op;
            Expr* left;
            Expr* right;
        } binary;
    };
};

/* Statement kinds */
typedef enum {
    STMT_LET, STMT_PRINT, STMT_EXPR
} StmtKind;

typedef struct Stmt Stmt;

struct Stmt {
    StmtKind kind;
    union {
        struct { char name[64]; Expr* expr; } let_stmt;
        struct { Expr* expr; } print_stmt;
        struct { Expr* expr; } expr_stmt;
    };
    Stmt* next;
};



#endif
