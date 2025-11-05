/**
 * @file value.c
 * @brief Implements the Value structure and related functions for the Jackal programming language.
 */

#include "value.h"
#include "parser.h"
#include "eval.h"
#include "env.h"


#include <string.h>
#include <stdio.h>


/**
 * @typedef @struct VALUEARRAY
 * Represents a dynamic array of Values in the Jackal programming language.
 */
ValueArray* array_new(void) {
    ValueArray* arr = malloc(sizeof(ValueArray));
    arr->capacity = 8;
    arr->count = 0;
    arr->values = malloc(sizeof(Value) * arr->capacity);
    return arr;
}
/**
 * Appends a Value to a ValueArray.
 * @param arr The ValueArray to append to.
 * @param val The Value to append.
 */
void array_append(ValueArray* arr, Value val) {
    if (arr->count >= arr->capacity) {
        arr->capacity *= 2;
        arr->values = realloc(arr->values, sizeof(Value) * arr->capacity);
    }
    arr->values[arr->count++] = val;
}
/**
 * Frees the memory associated with a ValueArray.
 * @param arr The ValueArray to be freed.
 */
void array_free(ValueArray* arr) {
    for (int i = 0; i < arr->count; i++) {
        free_value(arr->values[i]);
    }
    free(arr->values);
    free(arr);
}
/**
 * Prints a Value to stdout.
 * @param value The Value to be printed.
 */
void print_value(Value value) {
    switch (value.type) {
        case VAL_NIL:
            printf("nil");
            break;
        case VAL_NUMBER:
            printf("%g", value.as.number);
            break;
        case VAL_STRING:
            printf("%s", value.as.string);
            break;
        case VAL_FUNCTION:
            printf("<function>");
            break;
        case VAL_RETURN:
            print_value(*value.as.return_val);
            break;
        case VAL_ARRAY:
            printf("[");
            for (int i = 0; i < value.as.array->count; i++) {
                print_value(value.as.array->values[i]);
                if (i < value.as.array->count - 1) {
                    printf(", ");
                }
            }
            printf("]");
            break;
        case VAL_CLASS:
            if (value.as.class_obj) {
                printf("<class %s>", value.as.class_obj->name);
            } else {
                printf("<class nil>");
            }
            break;
        case VAL_INSTANCE:
            if (value.as.instance && value.as.instance->class_val) {
                printf("<instance %s>", value.as.instance->class_val->as.class_obj->name);
            } else {
                printf("<instance nil>");
            }
            break;
    }
}

/**
 * Frees the memory associated with a Value.
 * @param value The Value to be freed.
 */
void free_value(Value value) {
    if (value.type == VAL_STRING) {
        free(value.as.string);
    }
    if (value.type == VAL_FUNCTION) {
        Func* func = value.as.function;
        
        free(func);
    }
    if (value.type == VAL_RETURN) {
        free_value(*value.as.return_val);
        free(value.as.return_val);
    }
    if (value.type == VAL_ARRAY) {
        array_free(value.as.array);
    }
    
    if (value.type == VAL_CLASS) {

    }
    if (value.type == VAL_INSTANCE) {
        
    }
}

/**
 * Creates a copy of a Value.
 * @param value The Value to be copied.
 * @return A new Value that is a copy of the input.
 */

Value copy_value(Value value) {
    if (value.type == VAL_STRING) {
        char* new_string = malloc(strlen(value.as.string) + 1);
        if (new_string) { 
            strcpy(new_string, value.as.string);
        }
        return (Value){VAL_STRING, {.string = new_string}};
    }
    if (value.type == VAL_FUNCTION) {
        Func* old_func = value.as.function;
        Func* new_func = malloc(sizeof(Func));
        memcpy(new_func, old_func, sizeof(Func));
        return (Value){VAL_FUNCTION, {.function = new_func}};
    }
    if (value.type == VAL_ARRAY) {
        ValueArray* old_arr = value.as.array;
        ValueArray* new_arr = array_new();
        for (int i = 0; i < old_arr->count; i++) {
            array_append(new_arr, copy_value(old_arr->values[i]));
        }
        return (Value){VAL_ARRAY, {.array = new_arr}};
    }
    if (value.type == VAL_CLASS || value.type == VAL_INSTANCE) {
        return value; // Kembalikan pointer yang sama (referensi)
    }
    return value; 
}

/**
 * Determines if a Value is truthy.
 * @param value The Value to be evaluated.
 * @return true if the Value is truthy, false otherwise.
 */
bool is_value_truthy(Value value) {
    switch (value.type) {
        case VAL_NIL:    return false;
        case VAL_NUMBER: return value.as.number != 0;
        case VAL_STRING: return strlen(value.as.string) > 0;
        case VAL_FUNCTION: return true;
        case VAL_ARRAY: return value.as.array->count > 0;
        case VAL_CLASS:    return true;
        case VAL_INSTANCE: return true;
        case VAL_RETURN: return is_value_truthy(*value.as.return_val);
    }
    return false;
}

/**
 * Evaluates equality between two Values.
 * @param a The first Value.
 * @param b The second Value.
 * @return A Value of type VAL_NUMBER with 1.0 if equal, 0.0 otherwise.
 */
Value eval_equals(Value a, Value b) {
    double result = 0.0;
    if (a.type != b.type) {
        result = 0.0;
    } else {
        switch(a.type) {
            case VAL_NIL: result = 1.0; break;
            case VAL_NUMBER: result = (a.as.number == b.as.number); break;
            case VAL_STRING: result = (strcmp(a.as.string, b.as.string) == 0); break;
            case VAL_FUNCTION: result = (a.as.function == b.as.function); break;
            case VAL_ARRAY: result = (a.as.array == b.as.array); break;
            case VAL_CLASS: result = (a.as.class_obj == b.as.class_obj); break;
            case VAL_INSTANCE: result = (a.as.instance == b.as.instance); break;
            default: result = 0.0; break;
        }
    }
    return (Value){VAL_NUMBER, {.number = result}};
}
/**
 * Reads a line from standard input and returns it as a Value of type VAL_STRING.
 * @param arity The number of arguments (should be 0).
 * @param args The array of argument Values (not used).
 * @return A Value containing the input string, or VAL_NIL on failure/EOF.
 */
Value builtin_read(int arity, Value* args) {
    (void)arity; 
    (void)args;  

    char buffer[1024];
    if (fgets(buffer, sizeof(buffer), stdin) != NULL) {
        
        size_t len = strlen(buffer);
        if (len > 0 && buffer[len - 1] == '\n') {
            buffer[len - 1] = '\0';
        }
        
        char* str_copy = malloc(strlen(buffer) + 1);
        strcpy(str_copy, buffer);
        return (Value){VAL_STRING, {.string = str_copy}};
    }
    return (Value){VAL_NIL, .as = {0}}; // Kembalikan nil jika gagal/EOF
}