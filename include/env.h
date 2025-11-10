#pragma once
#include "common.h"
#include "parser.h" 

/**
 * @typedef @struct ENV
 * Represents an environment (scope) in the Jackal programming language.
 */
Env* env_new(Env* outer);

/**
 * Finds a variable by name in the given environment or its outer environments.
 * @param env The environment to search.
 * @param name The name of the variable to find.
 * @return A pointer to the Var if found, otherwise NULL.
 */
Var* find_var(Env* env, const char* name);

/**
 * Sets a variable in the given environment.
 * @param env The environment to set the variable in.
 * @param name The name of the variable.
 * @param value The value to assign to the variable.
 * @param is_const Boolean indicating if the variable is constant.
 */
void set_var(Env* env, const char* name, Value value,bool is_const);


