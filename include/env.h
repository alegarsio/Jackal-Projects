#pragma once
#include "common.h"
#include "parser.h" 

Env* env_new(Env* outer);
Var* find_var(Env* env, const char* name);
void set_var(Env* env, const char* name, Value value);