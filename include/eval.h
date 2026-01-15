#pragma once
#include "parser.h" 
#include <setjmp.h>


/**
 * @typedef @struct EXCEPTIONSTATE
 * Represents the state of exceptions in the Jackal programming language.
 */
typedef struct {
    jmp_buf buf;
    int active;
    Value error_val; 
} ExceptionState;


/**
 * Global exception state for the interpreter.
 */
extern ExceptionState global_ex_state;


/**
 * @typedef @struct ENV
 * Represents an environment (scope) in the Jackal programming language.
 */
Value eval_node(Env* env, struct Node* n);

/**
 * Frees the memory associated with an environment and its variables.
 * @param env The environment to be freed.
 */
void env_free(Env* env);

Value call_jackal_function(Env *env, Value func_val, int arg_count, Value *args);

const char *get_value_type_name(Value val);
