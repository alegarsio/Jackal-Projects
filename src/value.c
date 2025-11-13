
/**
 * @include src 
 * represents part of the jackal programming language
 */
#include "value.h"
#include "parser.h"
#include "eval.h"
#include "env.h"

/**
 * @include collections dsa
 * represent dsa int collection stl
 */
#include "collections/linkedlist.h"

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

        case VAL_NATIVE:
            printf("<native fn>");
            break;
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
        case VAL_INTERFACE: printf("<interface %s>", value.as.interface_obj->name); break;
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
        case VAL_FILE:
        if (value.as.file) {
            printf("<file>");
        } else {
            printf("<closed file>");
        }
        break;
        case VAL_LINKEDLIST: 
            printf("<linkedlist size %d>", value.as.list->count);
            break;
        case VAL_ENUM: printf("<enum %s>", value.as.enum_obj->name); break;

        
        

        
    }
}

/**
 * Frees the memory associated with a Value.
 * @param value The Value to be freed.
 */
void free_value(Value value) {
    switch (value.type) {
        case VAL_STRING: free(value.as.string); break;
        case VAL_FUNCTION: free(value.as.function); break;
        case VAL_RETURN:
            free_value(*value.as.return_val);
            free(value.as.return_val);
            break;
            
        case VAL_ARRAY: 
            break;
        case VAL_CLASS: 
            break;
        case VAL_INSTANCE:
            break;
        case VAL_INTERFACE:
            break;
        case VAL_ENUM:
            break;
        
        default: break;
    }
}

/**
 * Creates a copy of a Value.
 * @param value The Value to be copied.
 * @return A new Value that is a copy of the input.
 */

Value copy_value(Value value) {
    switch (value.type) {
        case VAL_STRING:
            return (Value){VAL_STRING, {.string = strdup(value.as.string)}};
        case VAL_FUNCTION: {
            Func* new_func = malloc(sizeof(Func));
            memcpy(new_func, value.as.function, sizeof(Func));
            return (Value){VAL_FUNCTION, {.function = new_func}};
        }
        case VAL_ARRAY:
            return value;
        case VAL_CLASS:
        case VAL_INSTANCE:
        case VAL_INTERFACE:

        case VAL_FILE:
            return value;

        case VAL_NATIVE:
            return value; 
        case VAL_ENUM: 
            return value;   

        case VAL_LINKEDLIST: 
             return value;
    
             
        default: return value; 
    }
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
        case VAL_MAP: return value.as.map && value.as.map->count > 0;
        case VAL_FILE:  return value.as.file != NULL;
        case VAL_CLASS:    return true;
        case VAL_NATIVE:   return true;
        case VAL_INSTANCE: return true;
        case VAL_INTERFACE: return true;
        case VAL_BREAK: return false;
        case VAL_CONTINUE: return false;
        case VAL_ENUM: return true;
        case VAL_LINKEDLIST: return value.as.list->count > 0;
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
            case VAL_FILE: return (Value){VAL_NUMBER, {.number = (a.as.file == b.as.file)}};
            case VAL_NUMBER: result = (a.as.number == b.as.number); break;
            case VAL_STRING: result = (strcmp(a.as.string, b.as.string) == 0); break;
            case VAL_FUNCTION: result = (a.as.function == b.as.function); break;
            case VAL_ARRAY: result = (a.as.array == b.as.array); break;
            case VAL_CLASS: result = (a.as.class_obj == b.as.class_obj); break;
            case VAL_INSTANCE: result = (a.as.instance == b.as.instance); break;
            case VAL_INTERFACE: return (Value){VAL_NUMBER, {.number = (a.as.interface_obj == b.as.interface_obj)}};
            case VAL_ENUM: return (Value){VAL_NUMBER, {.number = (a.as.enum_obj == b.as.enum_obj)}};
            case VAL_BREAK: return (Value){VAL_NUMBER, {.number = 0.0}}; 
            case VAL_CONTINUE: return (Value){VAL_NUMBER, {.number = 0.0}}; 
            case VAL_LINKEDLIST: return (Value){VAL_NUMBER, {.number = (a.as.list == b.as.list)}};
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
    return (Value){VAL_NIL, .as = {0}};
}

/**
 * Deletes a Value at a specific index from a ValueArray.
 * @param arr The ValueArray to delete from.
 * @param index The index of the Value to delete.
 */

void array_delete(ValueArray* arr, int index) {
    if (index < 0 || index >= arr->count) return;

    free_value(arr->values[index]);

    for (int i = index; i < arr->count - 1; i++) {
        arr->values[i] = arr->values[i + 1];
    }

    arr->count--;
}


/**
 * Pops the last Value from a ValueArray.
 * @param arr The ValueArray to pop from.
 * @return The popped Value, or VAL_NIL if the array is empty.
 */
Value array_pop(ValueArray* arr) {
    if (arr->count == 0) {
        return (Value){VAL_NIL, {0}};
    }
   return arr->values[--arr->count];
}


/**
 * @brief Hashes a string using the FNV-1a algorithm.
 * @param key The string to be hashed.
 * @return The resulting hash value.
 */
static uint32_t hash_string(const char* key) {
    uint32_t hash = 2166136261u;
    for (int i = 0; key[i] != '\0'; i++) {
        hash ^= (uint8_t)key[i];
        hash *= 16777619;
    }
    return hash;
}
/**
 * Creates a new HashMap.
 * @return Pointer to the newly created HashMap.
 */
HashMap* map_new(void) {
    HashMap* map = malloc(sizeof(HashMap));
    map->count = 0;
    map->capacity = 0;
    map->entries = NULL;
    return map;
}

/**
 * Frees the memory associated with a HashMap.
 * @param map The HashMap to be freed.
 */
void map_free(HashMap* map) {
    if (!map) return;
    for (int i = 0; i < map->capacity; i++) {
        if (map->entries[i].key != NULL) {
            free(map->entries[i].key);
            
        }
    }
    free(map->entries);
    free(map);
}

/**
 * @brief Finds an entry in the HashMap by key.
 * @param entries The array of entries in the HashMap.
 * @param capacity The capacity of the HashMap.
 * @param key The key to search for.
 */
static Entry* find_entry(Entry* entries, int capacity, const char* key) {
    uint32_t index = hash_string(key) % capacity;
    for (;;) {
        Entry* entry = &entries[index];
        if (entry->key == NULL) {
            if (entry->value.type == VAL_NIL) return entry; // Empty found
        } else if (strcmp(entry->key, key) == 0) {
            return entry; // Key found
        }
        index = (index + 1) % capacity;
    }
}

/**
 * @brief Adjusts the capacity of the HashMap.
 * @param map The HashMap to adjust.
 * @param capacity The new capacity.
 */
static void map_adjust_capacity(HashMap* map, int capacity) {
    Entry* entries = calloc(capacity, sizeof(Entry));
    map->count = 0;
    for (int i = 0; i < map->capacity; i++) {
        Entry* entry = &map->entries[i];
        if (entry->key == NULL) continue;
        Entry* dest = find_entry(entries, capacity, entry->key);
        dest->key = entry->key;
        dest->value = entry->value;
        map->count++;
    }
    free(map->entries);
    map->entries = entries;
    map->capacity = capacity;
}

/**
 * Sets a key-value pair in the HashMap.
 * @param map The HashMap to set the key-value pair in.
 * @param key The key as a string.
 * @param value The Value to associate with the key.
 */
bool map_get(HashMap* map, const char* key, Value* out_val) {
    if (map->count == 0) return false;
    Entry* entry = find_entry(map->entries, map->capacity, key);
    if (entry->key == NULL) return false;
    *out_val = entry->value;
    return true;
}


/**
 * Sets a key-value pair in the HashMap.
 * @param map The HashMap to set the value in.
 * @param key The key of the Value to set.
 * @param val The Value to set.
 */

void map_set(HashMap* map, const char* key, Value val) {
    if (map->count + 1 > map->capacity * 0.75) {
        int capacity = map->capacity < 8 ? 8 : map->capacity * 2;
        map_adjust_capacity(map, capacity);
    }
    Entry* entry = find_entry(map->entries, map->capacity, key);
    bool is_new_key = (entry->key == NULL);
    if (is_new_key) {
        map->count++;
        entry->key = strdup(key);
    }
    entry->value = copy_value(val); 
}