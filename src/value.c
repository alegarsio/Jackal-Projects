#include "value.h"
#include "parser.h"
#include "eval.h"
#include "env.h"
#include <string.h>

ValueArray* array_new(void) {
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
    if (value.type == VAL_INSTANCE) {
        free(value.as.instance->class_val);
        env_free(value.as.instance->fields);
        free(value.as.instance);
    }
}

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
    if (value.type == VAL_CLASS) {
        return value;
    }
    if (value.type == VAL_INSTANCE) {
        Instance* old_inst = value.as.instance;
        Instance* new_inst = malloc(sizeof(Instance));
        
        new_inst->class_val = malloc(sizeof(Value));
        *new_inst->class_val = *old_inst->class_val;
        
        new_inst->fields = env_new(NULL);
        if (old_inst->fields) {
            for (Var* v = old_inst->fields->vars; v; v = v->next) {
                set_var(new_inst->fields, v->name, v->value);
            }
        }
        return (Value){VAL_INSTANCE, {.instance = new_inst}};
    }
    return value; 
}

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