#include "array/native_array.h"

#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <float.h>
#include "env.h"

#define ARRAY_REGISTER(env, name, func)                                           \
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

Value builtin_array_distinct(int argCount, Value *args)
{
    if (argCount != 1 || args[0].type != VAL_ARRAY)
    {
        print_error("distinct() requires an Array.");
        return (Value){VAL_NIL, {0}};
    }

    ValueArray *old_arr = args[0].as.array;
    ValueArray *new_arr = array_new();

    for (int i = 0; i < old_arr->count; i++)
    {
        bool is_dup = false;
        for (int j = 0; j < new_arr->count; j++)
        {
            Value eq = eval_equals(old_arr->values[i], new_arr->values[j]);
            if (eq.as.number == 1.0)
            {
                is_dup = true;
                break;
            }
        }
        if (!is_dup)
        {
            array_append(new_arr, copy_value(old_arr->values[i]));
        }
    }
    return (Value){VAL_ARRAY, {.array = new_arr}};
}
void register_array_natives(Env *env){
    ARRAY_REGISTER(env,"__array_distinc",builtin_array_distinct);
}