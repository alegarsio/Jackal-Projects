
/**
 * (c) Alegrarsio gifta Lesmana
 */

#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <stdbool.h>
#include <stdarg.h>
#include <stddef.h>

#define DEBUG_TRACE_EXECUTION 1



/**
 * @typedef @enum TOKENKIND
 * Represents the kinds of tokens in the Jackal programming language.
 */
typedef enum {

    TOKEN_END, TOKEN_NUMBER, TOKEN_IDENT,
    TOKEN_LET, TOKEN_CONST, TOKEN_PRINT,
    TOKEN_PLUS, TOKEN_MINUS, TOKEN_STAR, TOKEN_SLASH,
    TOKEN_ASSIGN, TOKEN_LPAREN, TOKEN_RPAREN, TOKEN_SEMI,
    TOKEN_STRING, 

    
    TOKEN_LBRACE,
    TOKEN_RBRACE,

    /**
     * Token For if & else and else if
     */

    TOKEN_IF,
    TOKEN_ELSE,

    /**
     * Token For boolean value e.g., true / false
     */
    TOKEN_TRUE,
    TOKEN_FALSE,

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
    TOKEN_PIPE_PIPE,
    TOKEN_PIPELINE,
    TOKEN_PLUS_PLUS,
    TOKEN_MINUS_MINUS,
    TOKEN_LBRACKET,
    TOKEN_RBRACKET,
    TOKEN_CLASS,
    TOKEN_THIS,
    TOKEN_DOT,
    TOKEN_IMPORT,
    TOKEN_MATCH,
    TOKEN_DEFAULT,
    TOKEN_ARROW, 
    TOKEN_EXTENDS,
    TOKEN_INTERFACE,
    TOKEN_IMPLEMENTS,
    TOKEN_COLON,
    TOKEN_ENUM,
    TOKEN_TRY,
    TOKEN_CATCH,
    TOKEN_THROW,
    TOKEN_PERCENT,
    TOKEN_BREAK,
    TOKEN_CONTINUE,
    TOKEN_AT,
    TOKEN_WHEN,
    TOKEN_INVALID,
    TOKEN_RECORD,

    TOKEN_EVERY,
    TOKEN_UNTIL,
    TOKEN_STRUCT,
    TOKEN_OBSERVE, 
    TOKEN_ON,
    TOKEN_TYPE,

    TOKEN_PRIVATE,
    TOKEN_OBJECT,
    TOKEN_OF,
    TOKEN_ARROW_TO_RETURN,
    TOKEN_LEFT_TYPE,
    TOKEN_RIGHT_TYPE,
    TOKEN_IN,
    TOKEN_DOT_DOT,
    TOKEN_TO,
    TOKEN_STEP,
    TOKEN_SUPER,
    TOKEN_TO_UNDERSCORE,
    TOKEN_RANGE,
    TOKEN_ANY,
    TOKEN_WITH,
    TOKEN_TRAIT,
    TOKEN_NAMESPACE,
    TOKEN_USING,
    TOKEN_FINAL,
    TOKEN_OR,
    
    
} TokenKind;

struct Node;
struct Env;
struct Value;


/**
 * track memory usage
 */
extern size_t bytesAllocated;

/**
 * forward declaration 
 */
void* jackal_malloc(size_t size);

/**
 * 
 */
void jackal_free(void* ptr, size_t size);


/**
 * @typedef @struct List
 * Forwarded declaration of List Data Strcutre
 */
typedef struct LinkedList LinkedList;


/**
 * @typedef @struct INTERFACE
 * Forwared declaration of Interface struct
 */
typedef struct Interface Interface;

/**
 * @typedef @struct CLASS
 * Forwared declaration of Class struct
 */
typedef struct Class Class;

/**
 * @typedef @struct ENUM
 * Forwared declaration of Enum struct
 */
typedef struct Enum Enum;

/**
 * @typedef @struct INSTANCE
 * Represents an instance of a class in the Jackal programming language.
 */


/**
 *  @typedef @struct NATIVEFN
 *  Represents a native function in the Jackal programming language.
 */
typedef struct Value (*NativeFn)(int argCount, struct Value* args);

typedef struct {
    char name[100];
    char **field_names;
    int field_count;
    struct Node *computed_body;
} StructDefinition;
typedef struct {
    StructDefinition *definition;
    struct Value *values; 
} StructInstance;

/**
 * @typedef @struct FUNC
 * Represents a function in the Jackal programming language.
 */
typedef struct Func {

    struct Node* params_head;
    struct Node* body_head;
    struct Env* env;
    int arity;
    bool is_memoized;
    bool is_deprecated;
    bool is_private;
    bool is_parallel;
    char* deprecated_message;
    struct HashMap* static_vars;
    struct HashMap* cache;
    char return_type[64];
    bool is_platform_specific;
    char* target_os;
    bool is_async;
    bool is_final;
} Func;

struct Value;

/**
 * @typedef @struct VALUEARRAY
 * Represents an array of values in the Jackal programming language.
 */
typedef struct {
    int capacity;
    int count;
    struct Value* values;
    char element_type[64];
} ValueArray;

/**
 * @typedef @struct CLASS
 * Represents a class in the Jack
 */
struct Class {               
    char name[64];
    struct Env* methods;
    Class* superclass;   
    Interface* interface;  
    char type_name[64];
    bool is_record;  
};

/**
 * @typedef @struct ENUM
 * Represents an enum in the Jackal programming language.
 */
struct Enum {
    char name[64];
    struct Env* values; 
};

/**
 * @typedef @struct INTERFACE
 * Represents an interface in the Jackal programming language.
 */
struct Interface {
    char name[64];
    struct Env* methods; 
};

/**
 * @typedef @struct INSTANCE
 * Represents an instance of a class in the Jack
 */
typedef struct {
    struct Value* class_val;
    struct Env* fields;
    char template_type[64];
} Instance;

/**
 * @typedef @enum VALUETYPE
 * Represents the type of a value in the Jackal programming language.
 */
typedef enum {
    VAL_NIL,
    VAL_NUMBER,
    VAL_STRING,
    VAL_FUNCTION,
    VAL_RETURN,
    VAL_ARRAY,
    VAL_CLASS,
    VAL_NATIVE,
    VAL_INSTANCE,
    VAL_MAP,
    VAL_ENUM,
    VAL_FILE,
    VAL_BREAK,    
    VAL_CONTINUE,
    VAL_LINKEDLIST,
    VAL_INTERFACE,
    VAL_SOCKET,
    VAL_STRUCT_DEF,     
    VAL_STRUCT_INSTANCE ,
    VAL_NAMESPACE
} ValueType;

typedef struct GCObject {
    bool is_marked;
    struct GCObject* next;
} GCObject;

/**
 * @typedef @struct VALUE
 * Represents a value in the Jackal programming language.
 */
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
        NativeFn native;
        struct HashMap* map;
        Interface* interface_obj;
        LinkedList* list;
        Enum* enum_obj;
        FILE* file;
        StructDefinition *struct_def;       
        StructInstance *struct_instance;
        struct Env* env;
    } as;
    struct GCObject* gc_info;
} Value;

extern GCObject* head ;

/**
 * @typedef @struct VAR
 * Represents a variable in the Jackal programming language.
 */
typedef struct Var {
    char name[64];
    Value value;
    bool is_const;
    bool is_final;
    struct Var* next;
    char expected_type[64];
} Var;


/**
 * Prints an error message to stderr.
 * @param message Error message to be printed.
 */
void print_value(Value value);

/**
 * Frees the memory associated with a Value.
 * @param value The Value to be freed.
 */
void free_value(Value value);

/**
 * Creates a copy of a Value.
 * @param value The Value to be copied.
 * @return A new Value that is a copy of the input.
 */
ValueArray* array_new(void);

/**
 * Appends a Value to a ValueArray.
 * @param arr The ValueArray to append to.
 * @param val The Value to append.
 */
void array_append(ValueArray* arr, Value val);

/**
 * Frees the memory associated with a ValueArray.
 * @param arr The ValueArray to be freed.
 */
void array_free(ValueArray* arr);

/**
 * Prints an error message to stderr.
 * @param message Error message to be printed.
 */
void print_error(const char *format, ...);
/**
 * @struct TOKEN
 * Represents a token in the Jackal programming language.
 */
typedef struct {
    TokenKind kind;
    char text[64];
    double number;
} Token;

typedef struct {
    Func* func;
    int arg_count;
    Value* args;
} AsyncData;


void* jackal_allocate_gc(size_t size);




#endif