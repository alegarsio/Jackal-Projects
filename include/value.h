#pragma once
#include "parser.h"
#include "common.h"
#include <stdbool.h>

ValueArray* array_new(void);
void array_append(ValueArray* arr, Value val);
void array_free(ValueArray* arr);

void print_value(Value value);
void free_value(Value value);
Value copy_value(Value value);
bool is_value_truthy(Value value);
Value eval_equals(Value a, Value b);