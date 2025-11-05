#pragma once
#include "parser.h"
#include "common.h"
#include <stdbool.h>

/**
 * @typedef @struct VALUEARRAY
 * Represents a dynamic array of Values in the Jackal programming language.
 */
ValueArray* array_new(void);

/**
 * Appends a Value to a ValueArray.
 * @param arr The ValueArray to append to.
 * @param val The Value to append.
 */
void array_append(ValueArray* arr, Value val);

/**
 * Frees the memory associated with a ValueArray.
 * @param arr The ValueArray to be freed.
 */
void array_free(ValueArray* arr);

/**
 * Prints a Value to stdout.
 * @param value The Value to be printed.
 */
void print_value(Value value);

/**
 * Frees the memory associated with a Value.
 * @param value The Value to be freed.
 */
void free_value(Value value);

/**
 * Creates a copy of a Value.
 * @param value The Value to be copied.
 * @return A new Value that is a copy of the input.
 */
Value copy_value(Value value);

/**
 * Determines if a Value is truthy.
 * @param value The Value to be evaluated.
 * @return true if the Value is truthy, false otherwise.
 */
bool is_value_truthy(Value value);

/**
 * Evaluates equality between two Values.
 * @param a The first Value.
 * @param b The second Value.
 * @return A Value of type VAL_NUMBER with 1.0 if equal, 0.0 otherwise.
 */
Value eval_equals(Value a, Value b);