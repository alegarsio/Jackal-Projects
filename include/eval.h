#pragma once
#include "parser.h"


typedef struct Env {
    Var* vars;
} Env;

double eval_node(Env* env, struct Node* n);
void env_free(Env* env);
