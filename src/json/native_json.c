#include "json/native_json.h"
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <math.h>
#include <sys/stat.h>
#include <curl/curl.h>
#include <cjson/cJSON.h>

#define JSON_REGISTER(env, name, func)                                           \
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

Value cjson_to_jackal(cJSON *item)
{
    if (cJSON_IsNumber(item))
    {
        return (Value){VAL_NUMBER, {.number = item->valuedouble}};
    }
    else if (cJSON_IsString(item))
    {
        return (Value){VAL_STRING, {.string = strdup(item->valuestring)}};
    }
    else if (cJSON_IsBool(item))
    {
        return (Value){VAL_BOOL, {.boolean = cJSON_IsTrue(item)}};
    }
    else if (cJSON_IsNull(item))
    {
        return (Value){VAL_NIL, {0}};
    }
    else if (cJSON_IsObject(item))
    {
        HashMap *map = map_new();
        cJSON *child;
        cJSON_ArrayForEach(child, item)
        {
            map_set(map, child->string, cjson_to_jackal(child));
        }
        return (Value){VAL_MAP, {.map = map}};
    }
    else if (cJSON_IsArray(item))
    {
        ValueArray *arr = array_new();
        cJSON *element;
        cJSON_ArrayForEach(element, item)
        {
            array_append(arr, cjson_to_jackal(element));
        }
        return (Value){VAL_ARRAY, {.array = arr}};
    }
    return (Value){VAL_NIL, {0}};
}

Value native_json_parse(int arity, Value *args)
{
    if (arity < 1 || args[0].type != VAL_STRING)
    {
        return (Value){VAL_NIL, {0}};
    }

    cJSON *json = cJSON_Parse(args[0].as.string);
    if (json == NULL)
        return (Value){VAL_NIL, {0}};

    Value result = cjson_to_jackal(json);
    cJSON_Delete(json);
    return result;
}
Value native_json_pretty(int arity, Value *args) {
    if (arity < 1) return (Value){VAL_NIL, {0}, NULL};

    cJSON *json = jackal_to_cjson(args[0]); 
    char *json_str = cJSON_Print(json);     
    
    Value result = (Value){VAL_STRING, {.string = strdup(json_str)}, NULL};
    
    free(json_str);
    cJSON_Delete(json);
    return result;
}

void register_json_natives(Env *env)
{
    JSON_REGISTER(env,"__json_parse",native_json_parse);
    JSON_REGISTER(env,"__json_preety",native_json_pretty);
}