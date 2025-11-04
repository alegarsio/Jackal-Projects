#pragma once
#include "parser.h" 

Value eval_node(Env* env, struct Node* n);
void env_free(Env* env);
Env* env_new(Env* outer);