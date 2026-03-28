#include "json/native_json.h"
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <math.h>
#include <sys/stat.h>
#include <curl/curl.h>

#if __has_include(<cjson/cJSON.h>)
    #include <cjson/cJSON.h>
    #define HAS_CJSON 1
#else
    #define HAS_CJSON 0
#endif


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
cJSON* jackal_to_cjson(Value val) {
    switch (val.type) {
        case VAL_NUMBER:
            return cJSON_CreateNumber(val.as.number);
        case VAL_STRING:
            return cJSON_CreateString(val.as.string);
        case VAL_BOOL:
            return val.as.boolean ? cJSON_CreateTrue() : cJSON_CreateFalse();
        case VAL_NIL:
            return cJSON_CreateNull();
        case VAL_ARRAY: {
            cJSON *arr = cJSON_CreateArray();
            ValueArray *list = val.as.array;
            for (int i = 0; i < list->count; i++) {
                cJSON_AddItemToArray(arr, jackal_to_cjson(list->values[i]));
            }
            return arr;
        }
        case VAL_MAP: {
            cJSON *obj = cJSON_CreateObject();
            HashMap *map = val.as.map;
            for (int i = 0; i < map->capacity; i++) {
                Entry *entry = &map->entries[i];
                if (entry->key != NULL) {
                    cJSON_AddItemToObject(obj, entry->key, jackal_to_cjson(entry->value));
                }
            }
            return obj;
        }
        default:
            return cJSON_CreateNull();
    }
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

Value native_json_encode(int arity, Value *args) {
    if (arity < 1) return (Value){VAL_NIL, {0}};

    cJSON *json = jackal_to_cjson(args[0]); 
    char *json_str = cJSON_PrintUnformatted(json); // Tanpa spasi dan newline
    
    Value result = (Value){VAL_STRING, {.string = strdup(json_str)}};
    
    free(json_str);
    cJSON_Delete(json);
    return result;
}

cJSON *jackal_value_to_cjson(Value jackal_val)
{
    if (jackal_val.type == VAL_NIL)
    {
        return cJSON_CreateNull();
    }
    if (jackal_val.type == VAL_NUMBER)
    {
        return cJSON_CreateNumber(jackal_val.as.number);
    }
    if (jackal_val.type == VAL_STRING)
    {
        return cJSON_CreateString(jackal_val.as.string);
    }

    if (jackal_val.type == VAL_MAP)
    {
        cJSON *root = cJSON_CreateObject();
        HashMap *map = jackal_val.as.map;

        for (int i = 0; i < map->capacity; i++)
        {
            Entry *entry = &map->entries[i];
            if (entry->key != NULL)
            {
                cJSON *val = jackal_value_to_cjson(entry->value);
                cJSON_AddItemToObject(root, entry->key, val);
            }
        }
        return root;
    }

    if (jackal_val.type == VAL_ARRAY)
    {
        cJSON *root = cJSON_CreateArray();
        ValueArray *arr = jackal_val.as.array;

        for (int i = 0; i < arr->count; i++)
        {
            cJSON *val = jackal_value_to_cjson(arr->values[i]);
            cJSON_AddItemToArray(root, val);
        }
        return root;
    }

    if (jackal_val.type == VAL_INSTANCE)
    {
        cJSON *root = cJSON_CreateObject();
        Instance *inst = jackal_val.as.instance;
        Var *v = inst->fields->vars;

        while (v)
        {
            cJSON *val = jackal_value_to_cjson(v->value);
            cJSON_AddItemToObject(root, v->name, val);
            v = v->next;
        }
        return root;
    }

    if (jackal_val.type == VAL_FUNCTION || jackal_val.type == VAL_NATIVE)
    {
        return cJSON_CreateString("<Function>");
    }
    if (jackal_val.type == VAL_CLASS)
    {
        char buffer[128];
        snprintf(buffer, sizeof(buffer), "<Class %s>", jackal_val.as.class_obj->name);
        return cJSON_CreateString(buffer);
    }

    return cJSON_CreateString("<Unsupported Type>");
}


static Value cjsonToJackal(cJSON *item)
{
    Value val;
    val.as.number = 0;

    if (cJSON_IsNumber(item))
    {
        val.type = VAL_NUMBER;
        val.as.number = item->valuedouble;
        return val;
    }

    if (cJSON_IsString(item))
    {
        val.type = VAL_STRING;
        val.as.string = strdup(item->valuestring);
        return val;
    }

    if (cJSON_IsBool(item))
    {
        val.type = cJSON_IsTrue(item) ? TOKEN_TRUE : TOKEN_FALSE;
        return val;
    }

    if (cJSON_IsNull(item))
    {
        val.type = VAL_NIL;
        return val;
    }

    if (cJSON_IsArray(item))
    {
        ValueArray *arr = array_new();
        cJSON *element = NULL;
        for (element = item->child; element != NULL; element = element->next)
        {
            array_append(arr, cjsonToJackal(element));
        }
        val.type = VAL_ARRAY;
        val.as.array = arr;
        return val;
    }

    if (cJSON_IsObject(item))
    {
        HashMap *map = map_new();
        cJSON *child = NULL;
        for (child = item->child; child != NULL; child = child->next)
        {
            map_set(map, child->string, cjsonToJackal(child));
        }
        val.type = VAL_MAP;
        val.as.map = map;
        return val;
    }

    val.type = VAL_NIL;
    return val;
}

cJSON *jackalToCjson(Value value)
{
    if (value.type == VAL_NUMBER)
    {
        return cJSON_CreateNumber(value.as.number);
    }

    if (value.type == VAL_STRING)
    {
        return cJSON_CreateString(value.as.string);
    }

    if (value.type == TOKEN_TRUE)
    {
        return cJSON_CreateBool(true);
    }

    if (value.type == TOKEN_FALSE)
    {
        return cJSON_CreateBool(false);
    }

    if (value.type == VAL_NIL)
    {
        return cJSON_CreateNull();
    }

    if (value.type == VAL_ARRAY)
    {
        cJSON *jsonArr = cJSON_CreateArray();
        ValueArray *arr = value.as.array;
        for (int i = 0; i < arr->count; i++)
        {
            cJSON_AddItemToArray(jsonArr, jackalToCjson(arr->values[i]));
        }
        return jsonArr;
    }

    if (value.type == VAL_MAP)
    {
        cJSON *jsonObj = cJSON_CreateObject();
        HashMap *map = value.as.map;
        for (int i = 0; i < map->capacity; i++)
        {
            if (map->entries[i].key != NULL)
            {
                cJSON_AddItemToObject(jsonObj,
                                      map->entries[i].key,
                                      jackalToCjson(map->entries[i].value));
            }
        }
        return jsonObj;
    }

    return cJSON_CreateNull();
}

char* value_to_json_string(Value val) {
    cJSON* json = jackal_value_to_cjson(val);
    char* str = cJSON_PrintUnformatted(json);
    cJSON_Delete(json);
    return str; 
}

Value builtin_json_stringify(int argCount, Value *args)
{
    if (argCount < 1)
    {
        return (Value){VAL_NIL, {0}};
    }

    cJSON *json = jackalToCjson(args[0]);
    char *string = NULL;

    if (argCount > 1 && args[1].type == TOKEN_TRUE)
    {
        string = cJSON_Print(json);
    }
    else
    {
        string = cJSON_PrintUnformatted(json);
    }

    Value result;
    result.type = VAL_STRING;
    result.as.string = strdup(string);

    cJSON_free(string);
    cJSON_Delete(json);

    return result;
}


void register_json_natives(Env *env)
{
    JSON_REGISTER(env,"__json_parse",native_json_parse);
    JSON_REGISTER(env,"__json_preety",native_json_pretty);
    JSON_REGISTER(env,"__json_encode",native_json_encode);
}