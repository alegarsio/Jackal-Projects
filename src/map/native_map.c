#include "map/native_map.h"

#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <float.h>
#include "env.h"

#define MAP_REGISTER(env, name, func)                                           \
    do                                                                           \
    {                                                                            \
        if (func != NULL)                                                        \
        {                                                                        \
            set_var(env, name, (Value){VAL_NATIVE, {.native = func}}, true, ""); \
        }                                                                        \
        else                                                                     \
        {                                                                        \
            printf("Error: Native function '%s' not implemented!\n", name);      \
        }                                                                        \
    } while (0)

 Value native_map_init_manual(int arity, Value *args) {
    Value map = (Value){VAL_MAP, {.map = map_new()}};
    return map;
}

Value native_map_set_manual(int arity, Value* args) {
 
    if (arity < 3 || args[0].type != VAL_MAP || args[1].type != VAL_STRING) {
        return (Value){VAL_NIL}; 
    }

    HashMap* map = args[0].as.map;
    const char* key = args[1].as.string;
    Value val = args[2];

    map_set(map, key, val);

    return val; 
}


Value native_map_keys(int arity, Value* args) {
    if (arity < 1 || args[0].type != VAL_MAP) {
        ValueArray* empty_va = array_new();
        return (Value){VAL_ARRAY, {.array = empty_va}};
    }

    HashMap* map = args[0].as.map;
    ValueArray* keys_va = array_new();

    if (map == NULL || map->entries == NULL || map->capacity <= 0) {
        return (Value){VAL_ARRAY, {.array = keys_va}};
    }

    for (int i = 0; i < map->capacity; i++) {
        Entry* entry = &map->entries[i];
        
        if (entry->key != NULL) {
            Value key_val;
            key_val.type = VAL_STRING;
            key_val.as.string = strdup(entry->key);
            
            array_append(keys_va, key_val);
        }
    }

    return (Value){VAL_ARRAY, {.array = keys_va}};
}

Value native_map_remove_manual(int arity, Value* args) {
    // Validasi: Pastikan ada 2 argumen (Map dan Key)
    if (arity < 2 || args[0].type != VAL_MAP || args[1].type != VAL_STRING) {
        return (Value){VAL_BOOL, {.boolean = false}}; 
    }

    HashMap* map = args[0].as.map;
    const char* key = args[1].as.string;

    bool success = map_delete(map, key);

    return (Value){VAL_BOOL, {.boolean = success}}; 
}

void register_map_natives(Env *env){
        MAP_REGISTER(env, "__map_set_manual__", native_map_set_manual);
        MAP_REGISTER(env, "__map_init_manual__", native_map_init_manual);
        MAP_REGISTER(env, "__map_keys__", native_map_keys);
}