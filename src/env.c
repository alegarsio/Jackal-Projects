#include "env.h"
#include "value.h"
#include "parser.h"
#include <stdlib.h>
#include <string.h>

/**
 * @typedef @struct ENV
 * Represents an environment (scope) in the Jackal programming language.
 */
Env* env_new(Env* outer) {
    Env* env = calloc(1, sizeof(Env));
    env->outer = outer;
    return env;
}
/**
 * Finds a variable by name in the given environment or its outer environments.
 * @param env The environment to search.
 * @param name The name of the variable to find.
 * @return A pointer to the Var if found, otherwise NULL.
 */
Var* find_var(Env* env, const char* name) {
    for (Var* v = env->vars; v; v = v->next) {
        if (strcmp(v->name, name) == 0) return v;
    }
    if (env->outer) {
        return find_var(env->outer, name);
    }
    return NULL;
}
/**
 * Sets a variable in the given environment.
 * @param env The environment to set the variable in.
 * @param name The name of the variable.
 * @param value The value to assign to the variable.
 */
void set_var(Env* env, const char* name, Value value) {
    Var* n = malloc(sizeof(Var));
    if (!n) return; 
    strcpy(n->name, name);
    n->value = copy_value(value); 
    n->next = env->vars;
    env->vars = n;
}
/**
 * Frees the memory associated with an environment and its variables.
 * @param env The environment to be freed.
 * 
 */
void env_free(Env* env) {
    if (env == NULL) return;
    Var* v = env->vars;
    while (v) {
        Var* next = v->next;
        free_value(v->value); 
        free(v);
        v = next;
    }
    free(env);
}