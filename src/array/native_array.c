#include "array/native_array.h"

#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <float.h>
#include "env.h"
#include "eval.h"

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

Value builtin_array_anyMatch(int argCount, Value *args)
{
    if (argCount != 2 || args[0].type != VAL_ARRAY)
    {
        print_error("anyMatch() requires 2 arguments: (Array, Callback).");
        return (Value){VAL_NUMBER, {.number = 0.0}};
    }

    ValueArray *arr = args[0].as.array;
    Value callback = args[1];

    for (int i = 0; i < arr->count; i++)
    {
        Value current_val = arr->values[i];

        Value res = call_jackal_function(NULL, callback, 1, &current_val);

        if (is_value_truthy(res))
        {
            free_value(res);
            return (Value){VAL_NUMBER, {.number = 1.0}};
        }
        free_value(res);
    }
    return (Value){VAL_NUMBER, {.number = 0.0}};
}

Value builtin_array_map(int argCount, Value *args)
{
    if (argCount != 2 || args[0].type != VAL_ARRAY || args[1].type != VAL_FUNCTION)
    {
        print_error("map() expects (Array, Callback).");
        return (Value){VAL_NIL, {0}};
    }

    ValueArray *old_arr = args[0].as.array;
    Value callback = args[1];
    ValueArray *new_arr = array_new();

    for (int i = 0; i < old_arr->count; i++)
    {
        Value arg = old_arr->values[i];
        Value new_val = call_jackal_function(NULL, callback, 1, &arg);
        array_append(new_arr, new_val);
    }

    return (Value){VAL_ARRAY, {.array = new_arr}};
}
Value builtin_array_filter(int argCount, Value *args)
{
    if (argCount != 2 || args[0].type != VAL_ARRAY || args[1].type != VAL_FUNCTION)
    {
        print_error("filter() expects (Array, Callback).");
        return (Value){VAL_NIL, {0}};
    }

    ValueArray *old_arr = args[0].as.array;
    Value callback = args[1];
    ValueArray *new_arr = array_new();

    for (int i = 0; i < old_arr->count; i++)
    {
        Value arg = old_arr->values[i];
        Value result = call_jackal_function(NULL, callback, 1, &arg);
        if (is_value_truthy(result))
        {
            array_append(new_arr, copy_value(arg));
        }
        free_value(result);
    }

    return (Value){VAL_ARRAY, {.array = new_arr}};
}

Value builtin_array_reduce(int argCount, Value *args)
{
    if (argCount < 2 || args[0].type != VAL_ARRAY || args[1].type != VAL_FUNCTION)
    {
        print_error("reduce() expects at least (Array, Callback).");
        return (Value){VAL_NIL, {0}};
    }

    ValueArray *arr = args[0].as.array;
    Value callback = args[1];
    Value accumulator;
    int start_index = 0;

    if (argCount == 3)
    {
        accumulator = copy_value(args[2]);
        start_index = 0;
    }
    else
    {
        if (arr->count == 0)
            return (Value){VAL_NIL, {0}};
        accumulator = copy_value(arr->values[0]);
        start_index = 1;
    }

    for (int i = start_index; i < arr->count; i++)
    {
        Value cb_args[2] = {accumulator, arr->values[i]};
        Value next_acc = call_jackal_function(NULL, callback, 2, cb_args);
        free_value(accumulator);
        accumulator = next_acc;
    }

    return accumulator;
}

Value builtin_array_sort(int argCount, Value *args)
{
    if (argCount != 2 || args[0].type != VAL_ARRAY || args[1].type != VAL_FUNCTION)
    {
        print_error("sorted() expects (Array, Callback).");
        return (Value){VAL_NIL, {0}};
    }

    ValueArray *old_arr = args[0].as.array;
    ValueArray *new_arr = array_new();
    for (int i = 0; i < old_arr->count; i++)
    {
        array_append(new_arr, copy_value(old_arr->values[i]));
    }

    for (int i = 0; i < new_arr->count - 1; i++)
    {
        for (int j = 0; j < new_arr->count - i - 1; j++)
        {
            Value cb_args[2] = {new_arr->values[j], new_arr->values[j + 1]};

            Value result = call_jackal_function(NULL, args[1], 2, cb_args);

            if (result.type == VAL_NUMBER && result.as.number > 0)
            {
                Value temp = new_arr->values[j];
                new_arr->values[j] = new_arr->values[j + 1];
                new_arr->values[j + 1] = temp;
            }
            free_value(result);
        }
    }

    return (Value){VAL_ARRAY, {.array = new_arr}};
}

Value builtin_array_mean(int argCount, Value *args)
{
    if (args == NULL)
        return (Value){VAL_NIL, {0}};

    Value receiver = args[-1];
    if (receiver.type != VAL_ARRAY)
        return (Value){VAL_NIL, {0}};

    ValueArray *arr = receiver.as.array;
    if (arr == NULL || arr->values == NULL)
        return (Value){VAL_NIL, {0}};

    double sum = 0;
    int count = 0;
    for (int i = 0; i < arr->count; i++)
    {
        if (arr->values[i].type == VAL_NUMBER)
        {
            sum += arr->values[i].as.number;
            count++;
        }
    }

    double result = (count > 0) ? (sum / count) : 0;

    ValueArray *resArr = array_new();
    array_append(resArr, (Value){VAL_NUMBER, {.number = result}});

    return (Value){VAL_ARRAY, {.array = resArr}};
}



void register_array_natives(Env *env){
    ARRAY_REGISTER(env,"__array_distinct",builtin_array_distinct);
    ARRAY_REGISTER(env,"__array_anyMatch",builtin_array_anyMatch);
    ARRAY_REGISTER(env,"__array_map",builtin_array_map);
    ARRAY_REGISTER(env,"__array_filter",builtin_array_filter);
    ARRAY_REGISTER(env,"__array_reduce",builtin_array_reduce);
    ARRAY_REGISTER(env,"__array_sort",builtin_array_sort);
}
