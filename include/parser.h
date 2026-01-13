#pragma once
#include "common.h"
#include "lexer.h"

typedef struct Env Env;

/**
 * @typedef @struct ENV
 * Represents an environment (scope) in the Jackal programming language.
 */
struct Env {
    Var* vars;
    struct Env* outer;
};

/**
 * @typedef @enum NODEKIND
 * Represents the kinds of AST nodes in the Jackal programming language.
 */
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
    NODE_IMPORT,
    NODE_MATCH_STMT, 
    NODE_MATCH_CASE,
    NODE_SET,
    NODE_MAP_LITERAL,
    NODE_ENUM_DEF,
    NODE_FUNC_EXPR,
    NODE_TRY_STMT,    
    NODE_THROW_STMT,
    NODE_CONSTDECL,
    NODE_UNARY,
    NODE_BREAK_STMT,
    NODE_CONTINUE_STMT,
    NODE_WHEN_EXPR,
    NODE_WHEN_CASE,
    NODE_INTERFACE_DEF,
    NODE_EVERY_LOOP,
    NODE_OBSERVE_STMT,
    NODE_OBSERVE_CASE, 
    NODE_FOR_IN,
    NODE_FOR_EACH,
    NODE_RANGE_EXPR,
    NODE_DESTRUCTURE,
    NODE_STRUCT_DEF,
    NODE_WITH,
    NODE_NAMESPACES,
    NODE_USING
} NodeKind;

/**
 * @typedef @struct NODE
 * Represents a node in the Abstract Syntax Tree (AST) of the Jackal programming language.
 */
typedef struct Node {

    NodeKind kind;
    TokenKind op;
    
    char name[256];
    char super_name[64];
    char interface_name[64];
    char type_name[64];
    double value;
    struct Node* left;
    struct Node* right;
    struct Node* next; 
    bool is_private;
    bool is_singleton;
    bool is_override;
    bool is_deprecated;
    bool is_record;
    bool is_main;
    bool is_memoize;
    bool is_paralel;
    bool is_static;
    
    char* deprecated_message;

    struct Node* super_template_types;
    struct Node* template_types;

    bool is_platform_specific;
    char* target_os;
    int arity;
    char return_type[64];
    bool is_async;
    
} Node;

typedef struct {
    struct Env* env;
    struct Node* body;
    Value result;
} ThreadArgs;

/**
 * @typedef @struct PARSER
 * Represents a parser for the Jackal programming language.
 */
typedef struct {
    Lexer* lexer;
    Token current;
} Parser;

/**
 * Initializes the parser with the given lexer.
 * @param P Pointer to the Parser to be initialized.
 * @param L Pointer to the Lexer to be used by the parser.
 */
void parser_init(Parser* P, Lexer* L);

/**
 * Frees the memory associated with an AST node and its children.
 * @param n Pointer to the Node to be freed.
 */
Node* parse_stmt(Parser* P);

/**
 * Parses the given source code and returns the root of the AST.
 * @param source The source code to be parsed.
 * @return Pointer to the root Node of the AST.
 */
void free_node(Node* n);

/**
 * Parses the given source code and returns the root of the AST.
 * @param source The source code to be parsed.
 * @return Pointer to the root Node of the AST.
 */
Node *parse(const char *source);


Node *new_node(NodeKind kind);