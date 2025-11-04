#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

typedef enum {
    TOKEN_END, TOKEN_NUMBER, TOKEN_IDENT,
    TOKEN_LET, TOKEN_PRINT,
    TOKEN_PLUS, TOKEN_MINUS, TOKEN_STAR, TOKEN_SLASH,
    TOKEN_ASSIGN, TOKEN_LPAREN, TOKEN_RPAREN, TOKEN_SEMI,
    TOKEN_STRING, 
    TOKEN_LBRACE,
    TOKEN_RBRACE,
    TOKEN_IF,
    TOKEN_ELSE,
    TOKEN_BANG,
    TOKEN_BANG_EQUAL,
    TOKEN_EQUAL_EQUAL,
    TOKEN_LESS,
    TOKEN_LESS_EQUAL,
    TOKEN_GREATER,
    TOKEN_GREATER_EQUAL,
    TOKEN_AND_AND,
    TOKEN_FUNCTION,
    TOKEN_RETURN,
    TOKEN_COMMA,
    TOKEN_WHILE,
    TOKEN_FOR,
    TOKEN_PLUS_PLUS,
    TOKEN_MINUS_MINUS,
    TOKEN_LBRACKET,
    TOKEN_RBRACKET,
    TOKEN_CLASS,
    TOKEN_THIS,
    TOKEN_DOT,
    TOKEN_INVALID
} TokenKind;

struct Node;
struct Env;

typedef struct Func {
    struct Node* params_head;
    struct Node* body_head;
    struct Env* env;
    int arity;
} Func;

struct Value;

typedef struct {
    int capacity;
    int count;
    struct Value* values;
} ValueArray;

typedef struct {
    char name[64];
    struct Env* methods;
} Class;

typedef struct {
    struct Value* class_val;
    struct Env* fields;
} Instance;


typedef enum {
    VAL_NIL,
    VAL_NUMBER,
    VAL_STRING,
    VAL_FUNCTION,
    VAL_RETURN,
    VAL_ARRAY,
    VAL_CLASS,
    VAL_INSTANCE
} ValueType;

typedef struct Value {
    ValueType type;
    union {
        double number;
        char* string;
        Func* function;
        struct Value* return_val;
        ValueArray* array;
        Class* class_obj;
        Instance* instance;
    } as;
} Value;


typedef struct Var {
    char name[64];
    Value value;
    struct Var* next;
} Var;

void print_value(Value value);
void free_value(Value value);

ValueArray* array_new(void);
void array_append(ValueArray* arr, Value val);
void array_free(ValueArray* arr);

void print_error(const char *message);

typedef struct {
    TokenKind kind;
    char text[64];
    double number;
} Token;

#endif