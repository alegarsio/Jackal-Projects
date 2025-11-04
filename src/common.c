#include "common.h"
#include "parser.h"
#include "eval.h"

void print_error(const char *message) {
    fprintf(stderr, "Error: %s\n", message);
}

ValueArray* array_new() {
    ValueArray* arr = malloc(sizeof(ValueArray));
    arr->capacity = 8;
    arr->count = 0;
    arr->values = malloc(sizeof(Value) * arr->capacity);
    return arr;
}

void array_append(ValueArray* arr, Value val) {
    if (arr->count >= arr->capacity) {
        arr->capacity *= 2;
        arr->values = realloc(arr->values, sizeof(Value) * arr->capacity);
    }
    arr->values[arr->count++] = val;
}

void array_free(ValueArray* arr) {
    for (int i = 0; i < arr->count; i++) {
        free_value(arr->values[i]);
    }
    free(arr->values);
    free(arr);
}

void print_value(Value value) {
    switch (value.type) {
        case VAL_NIL:
            // Jangan print apa-apa untuk nilai kosong
            break;

        case VAL_NUMBER:
            printf("%g", value.as.number);
            break;

        case VAL_STRING:
            if (value.as.string)
                printf("%s", value.as.string);
            break;

        case VAL_FUNCTION:
            printf("<function>");
            break;

        case VAL_RETURN:
            if (value.as.return_val)
                print_value(*value.as.return_val);
            break;

        case VAL_ARRAY:
            if (value.as.array) {
                printf("[");
                for (int i = 0; i < value.as.array->count; i++) {
                    print_value(value.as.array->values[i]);
                    if (i < value.as.array->count - 1)
                        printf(", ");
                }
                printf("]");
            } else {
                printf("[]");
            }
            break;

        case VAL_CLASS:
            if (value.as.class_obj && value.as.class_obj->name)
                printf("<class %s>", value.as.class_obj->name);
            else
                printf("<class>");
            break;

        case VAL_INSTANCE:
            if (value.as.instance &&
                value.as.instance->class_val &&
                value.as.instance->class_val->as.class_obj &&
                value.as.instance->class_val->as.class_obj->name)
                printf("<instance %s>",
                       value.as.instance->class_val->as.class_obj->name);
            else
                printf("<instance>");
            break;
    }
}

void free_value(Value value) {
    if (value.type == VAL_STRING) {
        free(value.as.string);
    }
    if (value.type == VAL_FUNCTION) {
        Func* func = value.as.function;
        free_node(func->params_head);
        free_node(func->body_head);
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
        env_free(value.as.class_obj->methods);
        free(value.as.class_obj);
    }
    
    // --- PERBAIKAN DI SINI ---
    if (value.type == VAL_INSTANCE) {
        // JANGAN free class_val->as.class_obj, karena itu di-share
        free(value.as.instance->class_val); // Free 'Value*' yang menyimpan pointer class
        env_free(value.as.instance->fields); // Instance memiliki (owns) field-nya
        free(value.as.instance); // Free instance itu sendiri
    }
    // --- BATAS PERBAIKAN ---
}