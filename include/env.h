#ifndef ENV_H
#define ENV_H

#include "common.h" 


typedef struct Env Env;
struct Var;


/**
 * Make new environment.
 * @param outer Parent scope 
 */
Env* env_new(Env* outer);

/**
 * read variable by name
 * @param env environment
 * @param name variable name
 */
struct Var* find_var(Env* env, const char* name);

/**
 * set variable
 * @param env environment
 * @param name name of then variable
 * @param value value of the varible e.g., String, Number
 * @param is_const this true if the variable is const
 */
void set_var(Env* env, const char* name, Value value, bool is_const);

/**
 * clear the environment 
 */
void env_free(Env* env);

int assign_var(Env* env, const char* name, Value value);
#endif