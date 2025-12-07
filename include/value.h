#pragma once
#include "parser.h"
#include "common.h"
#include <stdbool.h>
#include<cjson/cJSON.h>

/**
 * @typedef @struct ENTRY
 * Represents a key-value pair in the HashMap.
 */
typedef struct {
    char* key;
    Value value;
} Entry;

/**
 * @typedef @struct HASHMAP
 * Represents a hash map (dictionary) in the Jackal programming language.
 */
typedef struct HashMap {
    int count;
    int capacity;
    Entry* entries;
} HashMap;


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

/**
 * Deletes a Value at a specific index from a ValueArray.
 * @param arr The ValueArray to delete from.
 * @param index The index of the Value to delete.
 */
void array_delete(ValueArray* arr, int index);

/**
 * Pops the last Value from a ValueArray.
 * @param arr The ValueArray to pop from.
 * @return The popped Value.
 */
Value array_pop(ValueArray* arr);

/**
 * @typedef @struct HASHMAP
 * Represents a hash map (dictionary) in the Jackal programming language.
 */
HashMap* map_new(void);

/**
 * Frees the memory associated with a HashMap.
 * @param map The HashMap to be freed.
 */
void map_free(HashMap* map);

/**
 * Retrieves a Value from the HashMap by key.
 * @param map The HashMap to retrieve from.
 * @param key The key of the Value to retrieve.
 * @param out_val Pointer to store the retrieved Value.
 * @return true if the key exists and out_val is set, false otherwise.
 */
bool map_get(HashMap* map, const char* key, Value* out_val);

/**
 * Sets a key-value pair in the HashMap.
 * @param map The HashMap to set the value in.
 * @param key The key of the Value to set.
 * @param val The Value to set.
 */
void map_set(HashMap* map, const char* key, Value val);

/**
 * Jackal_value_to_cjson
 * @brief parse jackal std value to json format
 * @param jackal_val
 */
cJSON *jackal_value_to_cjson(Value jackal_val);