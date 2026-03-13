#ifndef JSON_REGISTRY_H
#define JSON_REGISTRY_H

#include "common.h"
#include "env.h"
#include "value.h"

void register_json_natives(Env* env);

Value native_json_parse(int arity, Value *args);

#endif
