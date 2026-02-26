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

void register_map_natives(Env *env){
        MAP_REGISTER(env, "__map_set_manual__", native_map_set_manual);
}