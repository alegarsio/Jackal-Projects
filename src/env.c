#include "env.h"
#include "value.h"
#include "parser.h"
#include <stdlib.h>
#include <string.h>

Env* env_new(Env* outer) {
    Env* env = calloc(1, sizeof(Env));
    env->outer = outer;
    return env;
}

Var* find_var(Env* env, const char* name) {
    for (Var* v = env->vars; v; v = v->next) {
        if (strcmp(v->name, name) == 0) return v;
    }
    if (env->outer) {
        return find_var(env->outer, name);
    }
    return NULL;
}

void set_var(Env* env, const char* name, Value value) {
    Var* n = malloc(sizeof(Var));
    if (!n) return; 
    strcpy(n->name, name);
    n->value = copy_value(value); 
    n->next = env->vars;
    env->vars = n;
}

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