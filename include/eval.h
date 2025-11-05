#pragma once
#include "parser.h" 

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