
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
#include <curl/curl.h>
#include <float.h>

#if defined(__linux__) || (__APPLE__)
    #include <sys/socket.h>
    #include <sys/types.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <unistd.h>
# endif
typedef struct
{
    CURLcode result_code;
    long http_code;
    char *response_body;
} HttpClientResult;

typedef struct
{
    double distance;
    int label;
} Neighbor;
/**
 * @typedef @struct VALUEARRAY
 * Represents a dynamic array of Values in the Jackal programming language.
 */
ValueArray *array_new(void)
{
    ValueArray *arr = jackal_allocate_gc(sizeof(ValueArray));
    arr->capacity = 8;
    arr->count = 0;
    arr->values = malloc(sizeof(Value) * arr->capacity);
    
    for (int i = 0; i < arr->capacity; i++) {
        arr->values[i].gc_info = NULL;
    }

    return arr;
}

ValueArray* get_minor(ValueArray* matrix, int col_to_remove) {
    ValueArray* minor = array_new();
    int n = matrix->count;

    for (int i = 1; i < n; i++) {
        ValueArray* row = matrix->values[i].as.array;
        ValueArray* new_row = array_new();
        for (int j = 0; j < n; j++) {
            if (j == col_to_remove) continue;
            array_append(new_row, row->values[j]);
        }
        array_append(minor, (Value){VAL_ARRAY, {.array = new_row}});
    }
    return minor;
}
/**
 * Appends a Value to a ValueArray.
 * @param arr The ValueArray to append to.
 * @param val The Value to append.
 */
void array_append(ValueArray *arr, Value val)
{
    if (arr->count >= arr->capacity)
    {
        arr->capacity *= 2;
        arr->values = realloc(arr->values, sizeof(Value) * arr->capacity);
    }
    arr->values[arr->count++] = val;
}
/**
 * Frees the memory associated with a ValueArray.
 * @param arr The ValueArray to be freed.
 */
void array_free(ValueArray *arr)
{
    if (arr == NULL) return;

    for (int i = 0; i < arr->count; i++)
    {
        free_value(arr->values[i]);
    }

    if (arr->values != NULL)
    {
        free(arr->values);
        arr->values = NULL;
    }
    
    arr->count = 0;
    arr->capacity = 0;

}
/**
 * Prints a Value to stdout.
 * @param value The Value to be printed.
 */
void print_value(Value value)
{
    switch (value.type)
    {

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
        for (int i = 0; i < value.as.array->count; i++)
        {
            print_value(value.as.array->values[i]);
            if (i < value.as.array->count - 1)
            {
                printf(","); // Hanya koma, tanpa spasi ekstra agar bersih
            }
        }
        printf("]");
        break;
    case VAL_INTERFACE:
        printf("<interface %s>", value.as.interface_obj->name);
        break;
    case VAL_CLASS:
        if (value.as.class_obj)
        {
            printf("<class %s>", value.as.class_obj->name);
        }
        else
        {
            printf("<class nil>");
        }
        break;
    case VAL_INSTANCE:
        if (value.as.instance && value.as.instance->class_val)
        {
            printf("<instance %s>", value.as.instance->class_val->as.class_obj->name);
        }
        else
        {
            printf("<instance nil>");
        }
        break;
    case VAL_FILE:
        if (value.as.file)
        {
            printf("<file>");
        }
        else
        {
            printf("<closed file>");
        }
        break;
    case VAL_LINKEDLIST:
        printf("<linkedlist size %d>", value.as.list->count);
        break;
    case VAL_ENUM:
        printf("<enum %s>", value.as.enum_obj->name);
        break;
    }
}

/**
 * Frees the memory associated with a Value.
 * @param value The Value to be freed.
 */
void free_value(Value value)
{
    if (value.gc_info != NULL)
    {
        return;
    }

    switch (value.type)
    {
    case VAL_STRING:
        free(value.as.string);
        break;
    case VAL_FUNCTION:
        free(value.as.function);
        break;
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
    default:
        break;
    }
}

/**
 * Creates a copy of a Value.
 * @param value The Value to be copied.
 * @return A new Value that is a copy of the input.
 */

Value copy_value(Value value)
{
    switch (value.type)
    {
    case VAL_STRING:
        return (Value){VAL_STRING, {.string = strdup(value.as.string)}};
    case VAL_FUNCTION:
    {
        Func *new_func = malloc(sizeof(Func));
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

    default:
        return value;
    }
}

/**
 * Determines if a Value is truthy.
 * @param value The Value to be evaluated.
 * @return true if the Value is truthy, false otherwise.
 */
bool is_value_truthy(Value value)
{
    switch (value.type)
    {
    case VAL_NIL:
        return false;
    case VAL_NUMBER:
        return value.as.number != 0;
    case VAL_STRING:
        return strlen(value.as.string) > 0;
    case VAL_FUNCTION:
        return true;
    case VAL_ARRAY:
        return value.as.array->count > 0;
    case VAL_MAP:
        return value.as.map && value.as.map->count > 0;
    case VAL_FILE:
        return value.as.file != NULL;
    case VAL_CLASS:
        return true;
    case VAL_NATIVE:
        return true;
    case VAL_INSTANCE:
        return true;
    case VAL_INTERFACE:
        return true;
    case VAL_BREAK:
        return false;
    case VAL_CONTINUE:
        return false;
    case VAL_ENUM:
        return true;
    case VAL_LINKEDLIST:
        return value.as.list->count > 0;
    case VAL_RETURN:
        return is_value_truthy(*value.as.return_val);
    }
    return false;
}

/**
 * Evaluates equality between two Values.
 * @param a The first Value.
 * @param b The second Value.
 * @return A Value of type VAL_NUMBER with 1.0 if equal, 0.0 otherwise.
 */
Value eval_equals(Value a, Value b)
{
    double result = 0.0;
    if (a.type != b.type)
    {
        result = 0.0;
    }
    else
    {
        switch (a.type)
        {
        case VAL_NIL:
            result = 1.0;
            break;
        case VAL_FILE:
            return (Value){VAL_NUMBER, {.number = (a.as.file == b.as.file)}};
        case VAL_NUMBER:
            result = (a.as.number == b.as.number);
            break;
        case VAL_STRING:
            result = (strcmp(a.as.string, b.as.string) == 0);
            break;
        case VAL_FUNCTION:
            result = (a.as.function == b.as.function);
            break;
        case VAL_ARRAY:
            result = (a.as.array == b.as.array);
            break;
        case VAL_CLASS:
            result = (a.as.class_obj == b.as.class_obj);
            break;
        case VAL_INSTANCE:
            result = (a.as.instance == b.as.instance);
            break;
        case VAL_INTERFACE:
            return (Value){VAL_NUMBER, {.number = (a.as.interface_obj == b.as.interface_obj)}};
        case VAL_ENUM:
            return (Value){VAL_NUMBER, {.number = (a.as.enum_obj == b.as.enum_obj)}};
        case VAL_BREAK:
            return (Value){VAL_NUMBER, {.number = 0.0}};
        case VAL_CONTINUE:
            return (Value){VAL_NUMBER, {.number = 0.0}};
        case VAL_LINKEDLIST:
            return (Value){VAL_NUMBER, {.number = (a.as.list == b.as.list)}};
        default:
            result = 0.0;
            break;
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
Value builtin_read(int arity, Value *args)
{
    (void)arity;
    (void)args;

    char buffer[1024];
    if (fgets(buffer, sizeof(buffer), stdin) != NULL)
    {

        size_t len = strlen(buffer);
        if (len > 0 && buffer[len - 1] == '\n')
        {
            buffer[len - 1] = '\0';
        }

        char *str_copy = malloc(strlen(buffer) + 1);
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

void array_delete(ValueArray *arr, int index)
{
    if (index < 0 || index >= arr->count)
        return;

    free_value(arr->values[index]);

    for (int i = index; i < arr->count - 1; i++)
    {
        arr->values[i] = arr->values[i + 1];
    }

    arr->count--;
}

/**
 * Pops the last Value from a ValueArray.
 * @param arr The ValueArray to pop from.
 * @return The popped Value, or VAL_NIL if the array is empty.
 */
Value array_pop(ValueArray *arr)
{
    if (arr->count == 0)
    {
        return (Value){VAL_NIL, {0}};
    }
    return arr->values[--arr->count];
}

/**
 * @brief Hashes a string using the FNV-1a algorithm.
 * @param key The string to be hashed.
 * @return The resulting hash value.
 */
static uint32_t hash_string(const char *key)
{
    uint32_t hash = 2166136261u;
    for (int i = 0; key[i] != '\0'; i++)
    {
        hash ^= (uint8_t)key[i];
        hash *= 16777619;
    }
    return hash;
}
/**
 * Creates a new HashMap.
 * @return Pointer to the newly created HashMap.
 */
HashMap *map_new(void)
{
    HashMap *map = malloc(sizeof(HashMap));
    map->count = 0;
    map->capacity = 0;
    map->entries = NULL;
    return map;
}

/**
 * Frees the memory associated with a HashMap.
 * @param map The HashMap to be freed.
 */
void map_free(HashMap *map)
{
    if (!map)
        return;
    for (int i = 0; i < map->capacity; i++)
    {
        if (map->entries[i].key != NULL)
        {
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
static Entry *find_entry(Entry *entries, int capacity, const char *key)
{
    uint32_t index = hash_string(key) % capacity;
    for (;;)
    {
        Entry *entry = &entries[index];
        if (entry->key == NULL)
        {
            if (entry->value.type == VAL_NIL)
                return entry; // Empty found
        }
        else if (strcmp(entry->key, key) == 0)
        {
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
static void map_adjust_capacity(HashMap *map, int capacity)
{
    Entry *entries = calloc(capacity, sizeof(Entry));
    map->count = 0;
    for (int i = 0; i < map->capacity; i++)
    {
        Entry *entry = &map->entries[i];
        if (entry->key == NULL)
            continue;
        Entry *dest = find_entry(entries, capacity, entry->key);
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
bool map_get(HashMap *map, const char *key, Value *out_val)
{
    if (map->count == 0)
        return false;
    Entry *entry = find_entry(map->entries, map->capacity, key);
    if (entry->key == NULL)
        return false;
    *out_val = entry->value;
    return true;
}

/**
 * Sets a key-value pair in the HashMap.
 * @param map The HashMap to set the value in.
 * @param key The key of the Value to set.
 * @param val The Value to set.
 */

void map_set(HashMap *map, const char *key, Value val)
{
    if (map->count + 1 > map->capacity * 0.75)
    {
        int capacity = map->capacity < 8 ? 8 : map->capacity * 2;
        map_adjust_capacity(map, capacity);
    }
    Entry *entry = find_entry(map->entries, map->capacity, key);
    bool is_new_key = (entry->key == NULL);
    if (is_new_key)
    {
        map->count++;
        entry->key = strdup(key);
    }
    entry->value = copy_value(val);
}

Value builtin_read_line(int arity, Value *args)
{
    if (arity != 0)
    {
        print_error("Error: '__io_read' expects zero arguments.");

        return (Value){VAL_NIL, .as = {0}};
    }

    char buffer[1024];

    if (fgets(buffer, sizeof(buffer), stdin) == NULL)
    {
        return (Value){VAL_NIL, .as = {0}};
    }

    size_t len = strlen(buffer);
    if (len > 0 && buffer[len - 1] == '\n')
    {
        buffer[len - 1] = '\0';
    }

    char *str_copy = malloc(strlen(buffer) + 1);

    if (str_copy == NULL)
    {
        return (Value){VAL_NIL, .as = {0}};
    }

    strcpy(str_copy, buffer);

    return (Value){VAL_STRING, {.string = str_copy}};
}

Value builtin_read_array(int arity, Value *args)
{
    if (arity != 1)
    {
        print_error("Error: '__io_read_array' expects one argument (delimiter).");
        return (Value){VAL_NIL, .as = {0}};
    }

    if (args[0].type != VAL_STRING)
    {
        print_error("Error: Delimiter must be a String.");
        return (Value){VAL_NIL, .as = {0}};
    }

    const char *delimiter = args[0].as.string;

    char buffer[1024];
    if (fgets(buffer, sizeof(buffer), stdin) == NULL)
    {
        return (Value){VAL_ARRAY, .as.array = array_new()};
    }

    size_t len = strlen(buffer);
    if (len > 0 && buffer[len - 1] == '\n')
    {
        buffer[len - 1] = '\0';
    }

    char *input_copy = strdup(buffer);
    if (input_copy == NULL)
    {
        return (Value){VAL_ARRAY, .as.array = array_new()};
    }

    ValueArray *result_array = array_new();

    char *token = strtok(input_copy, delimiter);
    while (token != NULL)
    {

        while (*token == ' ')
        {
            token++;
        }

        char *token_copy = malloc(strlen(token) + 1);
        if (token_copy != NULL)
        {
            strcpy(token_copy, token);
            Value token_val = (Value){VAL_STRING, {.string = token_copy}};

            array_append(result_array, token_val);
        }

        token = strtok(NULL, delimiter);
    }

    free(input_copy);
    return (Value){VAL_ARRAY, .as.array = result_array};
}

Value builtin_print_table(int argCount, Value *args)
{
    if (argCount != 1)
    {
        print_error("print_table() requires exactly one argument (Array).");
        return (Value){VAL_NIL, {0}};
    }

    Value arg = args[0];

    if (arg.type != VAL_ARRAY)
    {
        print_error("print_table() only supports Array type.");
        return (Value){VAL_NIL, {0}};
    }

    ValueArray *arr = arg.as.array;

    printf("\n+-------+-----------------------------+\n");
    printf("| Index | Value                       |\n");
    printf("+-------+-----------------------------+\n");

    for (int i = 0; i < arr->count; i++)
    {
        Value val = arr->values[i];

        printf("| %-5d | ", i);

        print_value(val);

        printf("\n");

        if (i < arr->count - 1)
        {
            printf("|-------|-----------------------------|\n");
        }
    }

    printf("+-------+-----------------------------+\n");
    fflush(stdout);

    return (Value){VAL_NIL, {0}};
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

Value builtin_print_json(int argCount, Value *args)
{
    if (argCount != 1)
    {
        print_error("print_json() requires exactly one argument.");
        return (Value){VAL_NIL, {0}};
    }

    cJSON *json_root = jackal_value_to_cjson(args[0]);

    if (json_root == NULL)
    {
        print_error("print_json() failed to serialize object.");
        return (Value){VAL_NIL, {0}};
    }

    char *json_string = cJSON_Print(json_root);

    if (json_string == NULL)
    {
        print_error("cJSON_Print failed.");
        cJSON_Delete(json_root);
        return (Value){VAL_NIL, {0}};
    }

    printf("\n%s\n", json_string);
    fflush(stdout);

    cJSON_Delete(json_root);
    free(json_string);

    return (Value){VAL_NIL, {0}};
}

/**
 * @brief Built-in method MapStream.forEach(map, callback)
 * Executes a callback function for each key-value pair in the map,
 * manually handling the function call stack.
 * The callback signature must be: function(key, value)
 * @param argCount Should be 2 (Map, Function).
 * @param args Array of arguments.
 * @return VAL_NIL.
 */
Value builtin_map_forEach(int argCount, Value *args)
{
    if (argCount != 2)
    {
        print_error("MapStream.forEach() requires 2 arguments: map and callback.");
        return (Value){VAL_NIL, {0}};
    }

    Value map_val = args[0];
    Value callback_val = args[1];

    if (map_val.type != VAL_MAP)
    {
        print_error("First argument to MapStream::Flush must be a Map.");
        return (Value){VAL_NIL, {0}};
    }

    if (callback_val.type != VAL_FUNCTION)
    {
        print_error("Second argument to MapStream.forEach() must be a Function.");
        return (Value){VAL_NIL, {0}};
    }

    Func *func = callback_val.as.function;

    if (func->arity != 2)
    {
        print_error("Callback in MapStream.forEach() must accept 2 arguments (key, value).");
        return (Value){VAL_NIL, {0}};
    }

    HashMap *map = map_val.as.map;

    for (int i = 0; i < map->capacity; i++)
    {
        Entry *entry = &map->entries[i];

        if (entry->key != NULL)
        {

            Env *call_env = env_new(func->env);

            char *key_copy = strdup(entry->key);
            Value key_val = (Value){VAL_STRING, {.string = key_copy}};

            Value value_copy = copy_value(entry->value);

            Node *param_k = func->params_head;
            Node *param_v = param_k->next;

            set_var(call_env, param_k->name, key_val, false, "");

            set_var(call_env, param_v->name, value_copy, false, "");

            free_value(key_val);
            free_value(value_copy);

            Value result = eval_node(call_env, func->body_head);

            env_free(call_env);

            if (result.type == VAL_RETURN || result.type == VAL_BREAK || result.type == VAL_CONTINUE)
            {
                free_value(result);
            }
            else
            {
                free_value(result);
            }
        }
    }

    return (Value){VAL_NIL, {0}};
}

Value builtin_map_keys(int argCount, Value *args)
{
    if (argCount != 1)
    {
        print_error("MapStream.keys() requires exactly one argument (Map).");
        return (Value){VAL_NIL, {0}};
    }

    Value map_val = args[0];

    if (map_val.type != VAL_MAP)
    {
        print_error("Argument to MapStream.keys() must be a Map.");
        return (Value){VAL_NIL, {0}};
    }

    HashMap *map = map_val.as.map;
    ValueArray *keys_array = array_new();

    for (int i = 0; i < map->capacity; i++)
    {
        Entry *entry = &map->entries[i];

        if (entry->key != NULL)
        {
            char *key_copy = strdup(entry->key);
            Value key_val = (Value){VAL_STRING, {.string = key_copy}};

            array_append(keys_array, key_val);
        }
    }

    return (Value){VAL_ARRAY, {.array = keys_array}};
}

Value builtin_map_values(int argCount, Value *args)
{
    if (argCount != 1)
    {
        print_error("MapStream.values() requires exactly one argument (Map).");
        return (Value){VAL_NIL, {0}};
    }

    Value map_val = args[0];

    if (map_val.type != VAL_MAP)
    {
        print_error("Argument to MapStream.values() must be a Map.");
        return (Value){VAL_NIL, {0}};
    }

    HashMap *map = map_val.as.map;
    ValueArray *values_array = array_new();

    for (int i = 0; i < map->capacity; i++)
    {
        Entry *entry = &map->entries[i];

        if (entry->key != NULL)
        {
            Value value_copy = copy_value(entry->value);

            array_append(values_array, value_copy);
        }
    }

    return (Value){VAL_ARRAY, {.array = values_array}};
}

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

Value builtin_array_limit(int argCount, Value *args)
{

    if (argCount != 2 || args[0].type != VAL_ARRAY || args[1].type != VAL_NUMBER)
    {
        print_error("limit() requires (Array, Number).");
        return (Value){VAL_NIL, {0}};
    }

    ValueArray *old_arr = args[0].as.array;
    int limit = (int)args[1].as.number;

    if (limit < 0)
        limit = 0;

    int count_to_copy = (limit < old_arr->count) ? limit : old_arr->count;

    ValueArray *new_arr = array_new();
    for (int i = 0; i < count_to_copy; i++)
    {

        array_append(new_arr, copy_value(old_arr->values[i]));
    }

    return (Value){VAL_ARRAY, {.array = new_arr}};
}

Value builtin_os_platform(int argCount, Value *args)
{
#ifdef _WIN32
    return (Value){VAL_STRING, {.string = "win32"}};
#elif __apple__
    return (Value){VAL_STRING, {.string = "darwin"}};
#else
    return (Value){VAL_STRING, {.string = "linux"}};
#endif
}

Value builtin_array_statistics(int argCount, Value *args)
{
    ValueArray *arr = args[-1].as.array;

    if (arr->count == 0)
        return (Value){VAL_NIL, {0}};

    double sum = 0;
    double min = arr->values[0].as.number;
    double max = arr->values[0].as.number;

    for (int i = 0; i < arr->count; i++)
    {
        if (arr->values[i].type != VAL_NUMBER)
            continue;

        double val = arr->values[i].as.number;
        sum += val;
        if (val < min)
            min = val;
        if (val > max)
            max = val;
    }

    HashMap *statsMap = map_new();

    map_set(statsMap, "sum", (Value){VAL_NUMBER, {.number = sum}});
    map_set(statsMap, "min", (Value){VAL_NUMBER, {.number = min}});
    map_set(statsMap, "max", (Value){VAL_NUMBER, {.number = max}});
    map_set(statsMap, "count", (Value){VAL_NUMBER, {.number = (double)arr->count}});
    map_set(statsMap, "mean", (Value){VAL_NUMBER, {.number = sum / arr->count}});

    return (Value){VAL_MAP, {.map = statsMap}};
}

Value builtin_map_get(int argCount, Value *args)
{
    if (argCount != 1 || args[0].type != VAL_STRING)
    {
        print_error("get() expects 1 string argument.");
        return (Value){VAL_NIL, {0}};
    }

    HashMap *map = args[-1].as.map;
    char *key = args[0].as.string;

    for (int i = 0; i < map->capacity; i++)
    {
        Entry *entry = &map->entries[i];
        if (entry->key != NULL && strcmp(entry->key, key) == 0)
        {
            return copy_value(entry->value);
        }
    }

    return (Value){VAL_NIL, {0}};
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

Value builtin_array_max(int argCount, Value *args)
{
    if (args == NULL)
        return (Value){VAL_NIL, {0}};

    Value receiver = args[-1];
    if (receiver.type != VAL_ARRAY)
        return (Value){VAL_NIL, {0}};

    ValueArray *arr = receiver.as.array;
    if (arr == NULL || arr->values == NULL || arr->count == 0)
        return (Value){VAL_NIL, {0}};

    double maxVal = -1.7976931348623158e+308;
    int found = 0;

    for (int i = 0; i < arr->count; i++)
    {
        if (arr->values[i].type == VAL_NUMBER)
        {
            double current = arr->values[i].as.number;
            if (!found || current > maxVal)
            {
                maxVal = current;
                found = 1;
            }
        }
    }

    if (!found)
        return (Value){VAL_NIL, {0}};

    ValueArray *resArr = array_new();
    array_append(resArr, (Value){VAL_NUMBER, {.number = maxVal}});

    return (Value){VAL_ARRAY, {.array = resArr}};
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

Value builtin_json_parse(int argCount, Value *args)
{
    if (argCount != 1 || args[0].type != VAL_STRING)
    {
        print_error("json_parse() requires 1 string argument.");
        return (Value){VAL_NIL};
    }

    cJSON *json = cJSON_Parse(args[0].as.string);
    if (json == NULL)
        return (Value){VAL_NIL};

    Value result = cjsonToJackal(json);
    cJSON_Delete(json);

    return result;
}

static cJSON *jackalToCjson(Value value)
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

Value builtin_type(int argCount, Value *args)
{
    if (argCount != 1)
    {
        return (Value){VAL_NIL, {0}};
    }

    Value val;
    val.type = VAL_STRING;

    switch (args[0].type)
    {
    case VAL_NUMBER:
        val.as.string = strdup("number");
        break;
    case VAL_STRING:
        val.as.string = strdup("string");
        break;
    case TOKEN_TRUE:
    case TOKEN_FALSE:
        val.as.string = strdup("bool");
        break;
    case VAL_MAP:
        val.as.string = strdup("map");
        break;
    case VAL_ARRAY:
        val.as.string = strdup("array");
        break;
    case VAL_NIL:
        val.as.string = strdup("nil");
        break;
    case VAL_CLASS:
        val.as.string = strdup("class");
        break;

    default:
        val.as.string = strdup("unknown");
        break;
    }

    return val;
}

Value builtin_plot(int argCount, Value *args)
{
    if (argCount < 1 || args[0].type != VAL_ARRAY)
    {
        return (Value){VAL_NIL, {0}};
    }

    ValueArray *mainArr = args[0].as.array;
    if (mainArr->count == 0)
        return (Value){VAL_NIL, {0}};

    printf("\n--- Jackal Multi-Plot ---\n");

    for (int i = 0; i < mainArr->count; i++)
    {
        Value item = mainArr->values[i];

        if (item.type == VAL_ARRAY)
        {
            ValueArray *subArr = item.as.array;
            printf("Group %d | ", i + 1);

            for (int j = 0; j < subArr->count; j++)
            {
                if (subArr->values[j].type == VAL_NUMBER)
                {
                    int val = (int)subArr->values[j].as.number;
                    if (val > 20)
                        val = 20;

                    for (int k = 0; k < val; k++)
                        printf("*");
                    printf(" ");
                }
            }
            printf("\n");
        }
        else if (item.type == VAL_NUMBER)
        {
            printf("Data %d  | ", i + 1);
            int val = (int)item.as.number;
            for (int k = 0; k < val; k++)
                printf("*");
            printf("\n");
        }
    }
    printf("         +----------------------------------\n\n");

    return (Value){VAL_NIL, {0}};
}

static char *base64_encode(const char *data, size_t input_length)
{
    const char table[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    size_t output_length = 4 * ((input_length + 2) / 3);
    char *encoded_data = malloc(output_length + 1);
    if (encoded_data == NULL)
        return NULL;

    for (size_t i = 0, j = 0; i < input_length;)
    {
        uint32_t octet_a = i < input_length ? (unsigned char)data[i++] : 0;
        uint32_t octet_b = i < input_length ? (unsigned char)data[i++] : 0;
        uint32_t octet_c = i < input_length ? (unsigned char)data[i++] : 0;
        uint32_t triple = (octet_a << 0x10) + (octet_b << 0x08) + octet_c;

        encoded_data[j++] = table[(triple >> 3 * 6) & 0x3F];
        encoded_data[j++] = table[(triple >> 2 * 6) & 0x3F];
        encoded_data[j++] = i > input_length + 1 ? '=' : table[(triple >> 1 * 6) & 0x3F];
        encoded_data[j++] = i > input_length ? '=' : table[(triple >> 0 * 6) & 0x3F];
    }
    encoded_data[output_length] = '\0';
    return encoded_data;
}
Value builtin_web_show(int argCount, Value *args)
{
    if (argCount < 1 || args[0].type != VAL_STRING)
        return (Value){VAL_NIL, {0}};

    char *html_raw = args[0].as.string;

    FILE *f = fopen("/tmp/jackal_web.html", "w");
    if (f)
    {
        fprintf(f, "%s", html_raw);
        fclose(f);
        system("open /tmp/jackal_web.html");
    }

    return (Value){VAL_NIL, {0}};
}

Value builtin_web_sync(int argCount, Value *args)
{
    if (argCount < 1 || args[0].type != VAL_STRING)
    {
        return (Value){VAL_NIL, {0}};
    }

    const char *path = "/tmp/jackal_live.html";
    char *html_content = args[0].as.string;

    FILE *file = fopen(path, "w");
    if (file != NULL)
    {
        fputs(html_content, file);
        fclose(file);
    }

    return (Value){VAL_NIL, {0}};
}

Value builtin_system(int argCount, Value *args)
{
    if (argCount < 1 || args[0].type != VAL_STRING)
    {
        return (Value){VAL_NIL, {0}};
    }

    const char *command = args[0].as.string;

    int result = system(command);

    return (Value){VAL_NUMBER, {.number = (double)result}};
}
Value builtin_http_serve(int argCount, Value *args)
{
    if (argCount < 2)
        return (Value){VAL_NIL, {0}};

    int port = (int)args[0].as.number;
    char *html_content = args[1].as.string;

    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0)
        return (Value){VAL_NIL, {0}};

    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in address;
    memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0)
    {
        close(server_fd);
        return (Value){VAL_NIL, {0}};
    }

    listen(server_fd, 5);

    int new_socket = accept(server_fd, NULL, NULL);
    if (new_socket >= 0)
    {
        char response[20000];
        sprintf(response,
                "HTTP/1.1 200 OK\r\n"
                "Content-Type: text/html\r\n"
                "Content-Length: %lu\r\n"
                "Connection: close\r\n\r\n%s",
                strlen(html_content), html_content);

        send(new_socket, response, strlen(response), 0);
        close(new_socket);
    }

    close(server_fd);
    return (Value){VAL_NIL, {0}};
}

void inorder_traverse_logic(ValueArray *source, ValueArray *dest, int index)
{
    if (index >= source->count)
        return;
    inorder_traverse_logic(source, dest, 2 * index + 1);
    array_append(dest, copy_value(source->values[index]));
    inorder_traverse_logic(source, dest, 2 * index + 2);
}

void preorder_traverse_logic(ValueArray *source, ValueArray *dest, int index)
{
    if (index >= source->count)
        return;
    array_append(dest, copy_value(source->values[index]));
    preorder_traverse_logic(source, dest, 2 * index + 1);
    preorder_traverse_logic(source, dest, 2 * index + 2);
}

void postorder_traverse_logic(ValueArray *source, ValueArray *dest, int index)
{
    if (index >= source->count)
        return;
    postorder_traverse_logic(source, dest, 2 * index + 1);
    postorder_traverse_logic(source, dest, 2 * index + 2);
    array_append(dest, copy_value(source->values[index]));
}

Value builtin_array_to_tree(int argCount, Value *args)
{
    if (argCount < 2 || args[0].type != VAL_ARRAY || args[1].type != VAL_STRING)
    {
        return (Value){VAL_NIL, {0}};
    }

    ValueArray *src = args[0].as.array;
    char *mode = args[1].as.string;
    ValueArray *result = array_new();

    if (strcmp(mode, "inorder") == 0)
    {
        inorder_traverse_logic(src, result, 0);
    }
    else if (strcmp(mode, "preorder") == 0)
    {
        preorder_traverse_logic(src, result, 0);
    }
    else if (strcmp(mode, "postorder") == 0)
    {
        postorder_traverse_logic(src, result, 0);
    }

    return (Value){VAL_ARRAY, {.array = result}};
}

Value native_plot(int arg_count, Value *args)
{
    if (arg_count == 0)
    {
        printf("Error: plot() requires at least 1 argument.\n");
        return (Value){VAL_NIL, .as = {0}};
    }

    if (arg_count == 1)
    {
        printf("LINE:");
        print_value(args[0]);
    }
    else
    {
        if (args[0].type == VAL_STRING)
        {
            printf("%s:", args[0].as.string);
        }
        else
        {
            printf("LINE:");
        }
        print_value(args[1]);
    }

    printf("\n");
    return (Value){VAL_NIL, .as = {0}};
}

Value native_transpose(int arg_count, Value *args)
{

    if (arg_count < 1 || args[0].type != VAL_ARRAY)
    {
        printf("Error: transpose() expects an array.\n");
        return (Value){VAL_NIL, .as = {0}};
    }

    ValueArray *matrix = args[0].as.array;
    int row_count = matrix->count;
    if (row_count == 0)
        return args[0];

    if (matrix->values[0].type != VAL_ARRAY)
    {
        printf("Error: transpose() expects a 2D array.\n");
        return (Value){VAL_NIL, .as = {0}};
    }

    int col_count = matrix->values[0].as.array->count;

    ValueArray *result = array_new();

    for (int j = 0; j < col_count; j++)
    {
        ValueArray *new_row = array_new();

        for (int i = 0; i < row_count; i++)
        {

            ValueArray *current_row = matrix->values[i].as.array;

            if (j < current_row->count)
            {
                array_append(new_row, current_row->values[j]);
            }
            else
            {
                array_append(new_row, (Value){VAL_NIL, .as = {0}});
            }
        }

        Value row_val;
        row_val.type = VAL_ARRAY;
        row_val.as.array = new_row;
        array_append(result, row_val);
    }

    Value final_val;
    final_val.type = VAL_ARRAY;
    final_val.as.array = result;
    return final_val;
}

Value native_linear_regression(int arg_count, Value *args)
{

    ValueArray *x_arr = args[0].as.array;
    ValueArray *y_arr = args[1].as.array;
    int n = x_arr->count;

    double sum_x = 0, sum_y = 0, sum_xy = 0, sum_xx = 0;
    for (int i = 0; i < n; i++)
    {
        double x = x_arr->values[i].as.number;
        double y = y_arr->values[i].as.number;
        sum_x += x;
        sum_y += y;
        sum_xx += x * x;
        sum_xy += x * y;
    }

    double slope = (n * sum_xy - sum_x * sum_y) / (n * sum_xx - sum_x * sum_x);
    double intercept = (sum_y - slope * sum_x) / n;

    ValueArray *res = array_new();
    array_append(res, (Value){VAL_NUMBER, {.number = slope}});
    array_append(res, (Value){VAL_NUMBER, {.number = intercept}});
    return (Value){VAL_ARRAY, {.array = res}};
}

Value native_standardize(int arg_count, Value *args)
{
    if (arg_count != 1 || args[0].type != VAL_ARRAY)
    {
        return (Value){VAL_NIL};
    }

    ValueArray *arr = args[0].as.array;
    int n = arr->count;
    if (n == 0)
        return args[0];

    double sum = 0;
    for (int i = 0; i < n; i++)
    {
        sum += arr->values[i].as.number;
    }
    double mean = sum / n;

    double sq_sum = 0;
    for (int i = 0; i < n; i++)
    {
        double diff = arr->values[i].as.number - mean;
        sq_sum += diff * diff;
    }
    double std_dev = sqrt(sq_sum / n);

    ValueArray *res = array_new();
    for (int i = 0; i < n; i++)
    {
        double z = (arr->values[i].as.number - mean) / std_dev;
        Value val = {VAL_NUMBER, {.number = z}};
        array_append(res, val);
    }

    return (Value){VAL_ARRAY, {.array = res}};
}

Value native_smooth(int arg_count, Value *args)
{
    if (arg_count != 2 || args[0].type != VAL_ARRAY || args[1].type != VAL_NUMBER)
    {
        return (Value){VAL_NIL};
    }

    ValueArray *input = args[0].as.array;
    int period = (int)args[1].as.number;
    int n = input->count;

    if (period <= 0 || period > n)
        return args[0];

    ValueArray *res = array_new();

    for (int i = 0; i <= n - period; i++)
    {
        double sum = 0;
        for (int j = 0; j < period; j++)
        {
            sum += input->values[i + j].as.number;
        }
        double avg = sum / period;
        array_append(res, (Value){VAL_NUMBER, {.number = avg}});
    }

    return (Value){VAL_ARRAY, {.array = res}};
}

Value native_correlate(int arg_count, Value *args)
{
    if (arg_count != 2 || args[0].type != VAL_ARRAY || args[1].type != VAL_ARRAY)
    {
        return (Value){VAL_NIL};
    }

    ValueArray *arrX = args[0].as.array;
    ValueArray *arrY = args[1].as.array;
    int n = arrX->count;

    if (n != arrY->count || n == 0)
        return (Value){VAL_NUMBER, {.number = 0}};

    double sumX = 0, sumY = 0, sumXY = 0;
    double sumX2 = 0, sumY2 = 0;

    for (int i = 0; i < n; i++)
    {
        double x = arrX->values[i].as.number;
        double y = arrY->values[i].as.number;

        sumX += x;
        sumY += y;
        sumXY += x * y;
        sumX2 += x * x;
        sumY2 += y * y;
    }

    double numerator = (n * sumXY) - (sumX * sumY);
    double denominator = sqrt((n * sumX2 - sumX * sumX) * (n * sumY2 - sumY * sumY));

    if (denominator == 0)
        return (Value){VAL_NUMBER, {.number = 0}};

    return (Value){VAL_NUMBER, {.number = numerator / denominator}};
}

int compareNeighbors(const void *a, const void *b)
{
    Neighbor *n1 = (Neighbor *)a;
    Neighbor *n2 = (Neighbor *)b;
    if (n1->distance < n2->distance)
        return -1;
    if (n1->distance > n2->distance)
        return 1;
    return 0;
}
Value native_knn_nd(int arg_count, Value* args) {

    if (arg_count != 4) return (Value){VAL_NIL};

    ValueArray* newPoint = args[0].as.array; 
    ValueArray* history = args[1].as.array;   
    ValueArray* labels = args[2].as.array;   
    int k = (int)args[3].as.number;

    int n_history = history->count;
    Neighbor* neighbors = malloc(sizeof(Neighbor) * n_history);

    for (int i = 0; i < n_history; i++) {
        ValueArray* refPoint = history->values[i].as.array;
        double sum_sq = 0;

        for (int j = 0; j < newPoint->count; j++) {
            double val1 = newPoint->values[j].as.number;
            double val2 = refPoint->values[j].as.number;
            
            double diff = val1 - val2;
            sum_sq += diff * diff;
        }

        neighbors[i].distance = sqrt(sum_sq);
        neighbors[i].label = (int)labels->values[i].as.number;
    }

    qsort(neighbors, n_history, sizeof(Neighbor), compareNeighbors);

    int votes0 = 0;
    int votes1 = 0;
    for (int i = 0; i < k && i < n_history; i++) {
        if (neighbors[i].label == 0) votes0++;
        else votes1++;
    }

    free(neighbors);

    return (Value){VAL_NUMBER, {.number = (votes1 > votes0 ? 1 : 0)}};
}
Value native_accuracy(int arg_count, Value *args)
{
    if (arg_count != 2 || args[0].type != VAL_ARRAY || args[1].type != VAL_ARRAY)
    {
        return (Value){VAL_NIL};
    }

    ValueArray *predictions = args[0].as.array;
    ValueArray *actual = args[1].as.array;

    if (predictions->count != actual->count || predictions->count == 0)
    {
        return (Value){VAL_NUMBER, {.number = 0.0}};
    }

    int correct = 0;
    for (int i = 0; i < predictions->count; i++)
    {
        if (predictions->values[i].as.number == actual->values[i].as.number)
        {
            correct++;
        }
    }

    double score = (double)correct / predictions->count;
    return (Value){VAL_NUMBER, {.number = score}};
}

Value native_normalize_nd(int arg_count, Value* args) {
    if (arg_count != 1 || args[0].type != VAL_ARRAY) return (Value){VAL_NIL};

    ValueArray* matrix = args[0].as.array;
    int rows = matrix->count;
    if (rows == 0) return (Value){VAL_NIL};
    
    int cols = matrix->values[0].as.array->count;

    double* min_vals = malloc(sizeof(double) * cols);
    double* max_vals = malloc(sizeof(double) * cols);

    for (int j = 0; j < cols; j++) {
        min_vals[j] = INFINITY;
        max_vals[j] = -INFINITY;
        for (int i = 0; i < rows; i++) {
            double val = matrix->values[i].as.array->values[j].as.number;
            if (val < min_vals[j]) min_vals[j] = val;
            if (val > max_vals[j]) max_vals[j] = val;
        }
    }
    ValueArray* newMatrix = array_new();
    for (int i = 0; i < rows; i++) {
        ValueArray* newRow = array_new();
        for (int j = 0; j < cols; j++) {
            double val = matrix->values[i].as.array->values[j].as.number;
            double norm_val = (max_vals[j] - min_vals[j] == 0) ? 0 : (val - min_vals[j]) / (max_vals[j] - min_vals[j]);
            array_append(newRow, (Value){VAL_NUMBER, {.number = norm_val}});
        }
        array_append(newMatrix, (Value){VAL_ARRAY, {.array = newRow}});
    }

    free(min_vals);
    free(max_vals);
    return (Value){VAL_ARRAY, {.array = newMatrix}};
}

Value native_knn_prob(int arg_count, Value *args)
{
    if (arg_count != 4)
        return (Value){VAL_NIL};

    ValueArray *newPoint = args[0].as.array;
    ValueArray *history = args[1].as.array;
    ValueArray *labels = args[2].as.array;
    int k = (int)args[3].as.number;

    int n_history = history->count;
    Neighbor *neighbors = malloc(sizeof(Neighbor) * n_history);

    for (int i = 0; i < n_history; i++)
    {
        ValueArray *refPoint = history->values[i].as.array;
        double sum_sq = 0;
        for (int j = 0; j < newPoint->count; j++)
        {
            double diff = newPoint->values[j].as.number - refPoint->values[j].as.number;
            sum_sq += diff * diff;
        }
        neighbors[i].distance = sqrt(sum_sq);
        neighbors[i].label = (int)labels->values[i].as.number;
    }

    qsort(neighbors, n_history, sizeof(Neighbor), compareNeighbors);

    int count1 = 0;
    for (int i = 0; i < k && i < n_history; i++)
    {
        if (neighbors[i].label == 1)
            count1++;
    }

    double probability = (double)count1 / k;

    free(neighbors);
    return (Value){VAL_NUMBER, {.number = probability}};
}

Value native_confusion_matrix(int arg_count, Value *args)
{
    if (arg_count != 2)
        return (Value){VAL_NIL};

    ValueArray *prediksi = args[0].as.array;
    ValueArray *kunci = args[1].as.array;

    int tp = 0;
    int tn = 0;
    int fp = 0;
    int fn = 0;

    for (int i = 0; i < prediksi->count; i++)
    {
        int p = (int)prediksi->values[i].as.number;
        int k = (int)kunci->values[i].as.number;

        if (p == 1 && k == 1)
            tp++;
        else if (p == 0 && k == 0)
            tn++;
        else if (p == 1 && k == 0)
            fp++;
        else if (p == 0 && k == 1)
            fn++;
    }

    ValueArray *result = array_new();
    array_append(result, (Value){VAL_NUMBER, {.number = (double)tp}});
    array_append(result, (Value){VAL_NUMBER, {.number = (double)tn}});
    array_append(result, (Value){VAL_NUMBER, {.number = (double)fp}});
    array_append(result, (Value){VAL_NUMBER, {.number = (double)fn}});

    return (Value){VAL_ARRAY, {.array = result}};
}
Value native_split(int arg_count, Value* args) {
    if (arg_count != 2) return (Value){VAL_NIL};

    ValueArray* data = args[0].as.array;
    double ratio = args[1].as.number;

    int totalCount = data->count;
    int trainLimit = (int)(totalCount * ratio);
    
    if (trainLimit == 0 && totalCount > 0 && ratio > 0) {
        trainLimit = 1;
    }

    ValueArray* trainArray = array_new();
    ValueArray* testArray = array_new();

    for (int i = 0; i < totalCount; i++) {
        if (i < trainLimit) {
            array_append(trainArray, data->values[i]);
        } else {
            array_append(testArray, data->values[i]);
        }
    }

    ValueArray* result = array_new();
    array_append(result, (Value){VAL_ARRAY, {.array = trainArray}});
    array_append(result, (Value){VAL_ARRAY, {.array = testArray}});

    return (Value){VAL_ARRAY, {.array = result}};
}


Value native_sync_shuffle(int arg_count, Value* args) {
   
    if (arg_count != 2 || args[0].type != VAL_ARRAY || args[1].type != VAL_ARRAY) {
        return (Value){VAL_NIL};
    }

    ValueArray* data = args[0].as.array;
    ValueArray* labels = args[1].as.array;

    if (data->count != labels->count) return (Value){VAL_NIL};

    int n = data->count;
    srand(time(NULL)); 

    for (int i = n - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        
        Value tempData = data->values[i];
        data->values[i] = data->values[j];
        data->values[j] = tempData;

        Value tempLabel = labels->values[i];
        labels->values[i] = labels->values[j];
        labels->values[j] = tempLabel;
    }

    ValueArray* result = array_new();
    array_append(result, (Value){VAL_ARRAY, {.array = data}});
    array_append(result, (Value){VAL_ARRAY, {.array = labels}});

    return (Value){VAL_ARRAY, {.array = result}};
}

Value native_zip(int arg_count, Value* args) {
    if (arg_count != 2) return (Value){VAL_NIL};
    if (args[0].type != VAL_ARRAY || args[1].type != VAL_ARRAY) {
        printf("Runtime Error: zip() expects two arrays.\n");
        return (Value){VAL_NIL};
    }

    ValueArray* arr1 = args[0].as.array;
    ValueArray* arr2 = args[1].as.array;

   
    int length = (arr1->count < arr2->count) ? arr1->count : arr2->count;

    ValueArray* result = array_new();
    for (int i = 0; i < length; i++) {
        ValueArray* pair = array_new();
        array_append(pair, arr1->values[i]);
        array_append(pair, arr2->values[i]);
        
        array_append(result, (Value){VAL_ARRAY, {.array = pair}});
    }

    return (Value){VAL_ARRAY, {.array = result}};
}
Value native_logistic_predict(int arg_count, Value* args) {
    if (arg_count != 2) return (Value){VAL_NIL};

    ValueArray* newPoint = args[0].as.array;
    ValueArray* model = args[1].as.array;

    ValueArray* weights = model->values[0].as.array;
    double bias = model->values[1].as.number;

    if (newPoint->count != weights->count) return (Value){VAL_NIL};

    double z = bias;
    for (int j = 0; j < newPoint->count; j++) {
        z += newPoint->values[j].as.number * weights->values[j].as.number;
    }

    double probability = 1.0 / (1.0 + exp(-z));
    double result = (probability >= 0.5) ? 1.0 : 0.0;

    return (Value){VAL_NUMBER, {.number = result}};
}

Value native_logistic_fit(int arg_count, Value* args) {
    if (arg_count != 4) return (Value){VAL_NIL};

    ValueArray* data = args[0].as.array;    
    ValueArray* labels = args[1].as.array;  
    double lr = args[2].as.number;          
    int iterations = (int)args[3].as.number;

    int n_samples = data->count;
    int n_features = data->values[0].as.array->count;

    double* weights = malloc(sizeof(double) * n_features);
    for(int f = 0; f < n_features; f++) weights[f] = 0.0;
    double bias = 0.0;

    for (int iter = 0; iter < iterations; iter++) {
        double* dW_sum = malloc(sizeof(double) * n_features);
        for(int f = 0; f < n_features; f++) dW_sum[f] = 0.0;
        double dB_sum = 0.0;

        for (int i = 0; i < n_samples; i++) {
            ValueArray* row = data->values[i].as.array;
            double y_true = labels->values[i].as.number;

            double z = bias;
            for (int j = 0; j < n_features; j++) {
                z += row->values[j].as.number * weights[j];
            }

            double y_pred;
            if (z >= 0) {
                y_pred = 1.0 / (1.0 + exp(-z));
            } else {
                double ez = exp(z);
                y_pred = ez / (1.0 + ez);
            }

            double error = y_pred - y_true;

            for (int j = 0; j < n_features; j++) {
                dW_sum[j] += error * row->values[j].as.number;
            }
            dB_sum += error;
        }

        for (int j = 0; j < n_features; j++) {
            weights[j] -= lr * (dW_sum[j] / n_samples);
        }
        bias -= lr * (dB_sum / n_samples);
        
        free(dW_sum);
    }

    ValueArray* final_weights = array_new();
    for (int j = 0; j < n_features; j++) {
        Value w_val = {VAL_NUMBER, {.number = weights[j]}};
        array_append(final_weights, w_val);
    }

    ValueArray* result = array_new();
    Value weights_wrap = {VAL_ARRAY, {.array = final_weights}};
    array_append(result, weights_wrap);
    
    Value bias_wrap = {VAL_NUMBER, {.number = bias}};
    array_append(result, bias_wrap);

    free(weights);
    return (Value){VAL_ARRAY, {.array = result}};
}

Value native_nb_fit(int arg_count, Value* args) {
    ValueArray* data = args[0].as.array;
    ValueArray* labels = args[1].as.array;

    int n_samples = data->count;
    int n_features = data->values[0].as.array->count;

    double* mean0 = malloc(sizeof(double) * n_features);
    double* var0 = malloc(sizeof(double) * n_features);
    double* mean1 = malloc(sizeof(double) * n_features);
    double* var1 = malloc(sizeof(double) * n_features);
    
    int count0 = 0, count1 = 0;

    for (int f = 0; f < n_features; f++) {
        mean0[f] = var0[f] = mean1[f] = var1[f] = 0.0;
    }

    for (int i = 0; i < n_samples; i++) {
        int label = (int)labels->values[i].as.number;
        ValueArray* row = data->values[i].as.array;
        if (label == 0) {
            count0++;
            for (int f = 0; f < n_features; f++) mean0[f] += row->values[f].as.number;
        } else {
            count1++;
            for (int f = 0; f < n_features; f++) mean1[f] += row->values[f].as.number;
        }
    }

    if (count0 > 0) for (int f = 0; f < n_features; f++) mean0[f] /= count0;
    if (count1 > 0) for (int f = 0; f < n_features; f++) mean1[f] /= count1;

    for (int i = 0; i < n_samples; i++) {
        int label = (int)labels->values[i].as.number;
        ValueArray* row = data->values[i].as.array;
        if (label == 0) {
            for (int f = 0; f < n_features; f++) {
                double diff = row->values[f].as.number - mean0[f];
                var0[f] += diff * diff;
            }
        } else {
            for (int f = 0; f < n_features; f++) {
                double diff = row->values[f].as.number - mean1[f];
                var1[f] += diff * diff;
            }
        }
    }

    ValueArray* v_mean0 = array_new();
    ValueArray* v_var0 = array_new();
    ValueArray* v_mean1 = array_new();
    ValueArray* v_var1 = array_new();

    for (int f = 0; f < n_features; f++) {
        array_append(v_mean0, (Value){VAL_NUMBER, {.number = mean0[f]}});
        array_append(v_var0, (Value){VAL_NUMBER, {.number = (var0[f] / (count0 > 0 ? count0 : 1)) + 1e-9}});
        array_append(v_mean1, (Value){VAL_NUMBER, {.number = mean1[f]}});
        array_append(v_var1, (Value){VAL_NUMBER, {.number = (var1[f] / (count1 > 0 ? count1 : 1)) + 1e-9}});
    }

    ValueArray* final_model = array_new();
    array_append(final_model, (Value){VAL_ARRAY, {.array = v_mean0}});
    array_append(final_model, (Value){VAL_ARRAY, {.array = v_var0}});
    array_append(final_model, (Value){VAL_ARRAY, {.array = v_mean1}});
    array_append(final_model, (Value){VAL_ARRAY, {.array = v_var1}});
    array_append(final_model, (Value){VAL_NUMBER, {.number = (double)count0}});
    array_append(final_model, (Value){VAL_NUMBER, {.number = (double)count1}});

    free(mean0); free(var0); free(mean1); free(var1);

    return (Value){VAL_ARRAY, {.array = final_model}};
}
Value native_nb_predict(int arg_count, Value* args) {
    if (arg_count != 2) return (Value){VAL_NIL};

    ValueArray* point = args[0].as.array;
    ValueArray* model = args[1].as.array;

    ValueArray* mean0 = model->values[0].as.array;
    ValueArray* var0 = model->values[1].as.array;
    ValueArray* mean1 = model->values[2].as.array;
    ValueArray* var1 = model->values[3].as.array;
    double count0 = model->values[4].as.number;
    double count1 = model->values[5].as.number;

    int n_features = point->count;
    double log_probs[2];
    
    log_probs[0] = log(count0 / (count0 + count1));
    log_probs[1] = log(count1 / (count0 + count1));

    for (int f = 0; f < n_features; f++) {
        double x = point->values[f].as.number;

        double m0 = mean0->values[f].as.number;
        double v0 = var0->values[f].as.number;
        log_probs[0] += -0.5 * log(2 * M_PI * v0) - pow(x - m0, 2) / (2 * v0);

        double m1 = mean1->values[f].as.number;
        double v1 = var1->values[f].as.number;
        log_probs[1] += -0.5 * log(2 * M_PI * v1) - pow(x - m1, 2) / (2 * v1);
    }

    double result = (log_probs[1] > log_probs[0]) ? 1.0 : 0.0;
    return (Value){VAL_NUMBER, {.number = result}};
}

Value native_kmeans_predict(int arg_count, Value* args) {
    ValueArray* point = args[0].as.array;
    ValueArray* centroids = args[1].as.array;

    double min_dist = 1e18;
    int best_cluster = 0;

    for (int i = 0; i < centroids->count; i++) {
        ValueArray* centroid = centroids->values[i].as.array;
        double dist = 0;
        for (int f = 0; f < point->count; f++) {
            double diff = point->values[f].as.number - centroid->values[f].as.number;
            dist += diff * diff;
        }
        if (dist < min_dist) {
            min_dist = dist;
            best_cluster = i;
        }
    }

    return (Value){VAL_NUMBER, {.number = (double)best_cluster}};
}

Value native_kmeans_fit(int arg_count, Value* args) {
    ValueArray* data = args[0].as.array;
    int k = (int)args[1].as.number;
    int iterations = (int)args[2].as.number;

    int n_samples = data->count;
    int n_features = data->values[0].as.array->count;

    double** centroids = malloc(sizeof(double*) * k);
    for (int i = 0; i < k; i++) {
        centroids[i] = malloc(sizeof(double) * n_features);
        ValueArray* random_row = data->values[i % n_samples].as.array;
        for (int f = 0; f < n_features; f++) {
            centroids[i][f] = random_row->values[f].as.number;
        }
    }

    for (int iter = 0; iter < iterations; iter++) {
        int* assignments = malloc(sizeof(int) * n_samples);
        
        for (int i = 0; i < n_samples; i++) {
            double min_dist = 1e18;
            int best_cluster = 0;
            ValueArray* row = data->values[i].as.array;

            for (int j = 0; j < k; j++) {
                double dist = 0;
                for (int f = 0; f < n_features; f++) {
                    double diff = row->values[f].as.number - centroids[j][f];
                    dist += diff * diff;
                }
                if (dist < min_dist) {
                    min_dist = dist;
                    best_cluster = j;
                }
            }
            assignments[i] = best_cluster;
        }

        for (int j = 0; j < k; j++) {
            double* new_mean = malloc(sizeof(double) * n_features);
            for(int f=0; f<n_features; f++) new_mean[f] = 0;
            int count = 0;

            for (int i = 0; i < n_samples; i++) {
                if (assignments[i] == j) {
                    count++;
                    for (int f = 0; f < n_features; f++) {
                        new_mean[f] += data->values[i].as.array->values[f].as.number;
                    }
                }
            }

            if (count > 0) {
                for (int f = 0; f < n_features; f++) centroids[j][f] = new_mean[f] / count;
            }
            free(new_mean);
        }
        free(assignments);
    }

    ValueArray* final_centroids = array_new();
    for (int i = 0; i < k; i++) {
        ValueArray* c_row = array_new();
        for (int f = 0; f < n_features; f++) {
            array_append(c_row, (Value){VAL_NUMBER, {.number = centroids[i][f]}});
        }
        array_append(final_centroids, (Value){VAL_ARRAY, {.array = c_row}});
        free(centroids[i]);
    }
    free(centroids);

    return (Value){VAL_ARRAY, {.array = final_centroids}};
}

Value native_kmeans_loss(int arg_count, Value* args) {
    ValueArray* data = args[0].as.array;
    ValueArray* centroids = args[1].as.array;
    double total_sse = 0;

    for (int i = 0; i < data->count; i++) {
        double min_dist = 1e18;
        ValueArray* row = data->values[i].as.array;
        for (int j = 0; j < centroids->count; j++) {
            double dist = 0;
            ValueArray* c = centroids->values[j].as.array;
            for (int f = 0; f < row->count; f++) {
                double diff = row->values[f].as.number - c->values[f].as.number;
                dist += diff * diff;
            }
            if (dist < min_dist) min_dist = dist;
        }
        total_sse += min_dist;
    }
    return (Value){VAL_NUMBER, {.number = total_sse}};
}

Value native_matrix_dot(int arg_count, Value* args) {
    if (arg_count != 2) return (Value){VAL_NIL};
    
    ValueArray* A = args[0].as.array;
    ValueArray* B = args[1].as.array;

    int rowsA = A->count;
    int colsA = A->values[0].as.array->count;
    int rowsB = B->count;
    int colsB = B->values[0].as.array->count;

    if (colsA != rowsB) {
        return (Value){VAL_NIL};
    }

    ValueArray* result = array_new();

    for (int i = 0; i < rowsA; i++) {
        ValueArray* new_row = array_new();
        ValueArray* rowA = A->values[i].as.array;

        for (int j = 0; j < colsB; j++) {
            double sum = 0;
            for (int k = 0; k < colsA; k++) {
                double valA = rowA->values[k].as.number;
                double valB = B->values[k].as.array->values[j].as.number;
                sum += valA * valB;
            }
            array_append(new_row, (Value){VAL_NUMBER, {.number = sum}});
        }
        array_append(result, (Value){VAL_ARRAY, {.array = new_row}});
    }

    return (Value){VAL_ARRAY, {.array = result}};
}

Value native_matrix_add(int arg_count, Value* args) {
    ValueArray* A = args[0].as.array;
    ValueArray* B = args[1].as.array;

    int rows = A->count;
    int cols = A->values[0].as.array->count;

    if (rows != B->count || cols != B->values[0].as.array->count) {
        return (Value){VAL_NIL};
    }

    ValueArray* result = array_new();

    for (int i = 0; i < rows; i++) {
        ValueArray* rowA = A->values[i].as.array;
        ValueArray* rowB = B->values[i].as.array;
        ValueArray* new_row = array_new();

        for (int j = 0; j < cols; j++) {
            double sum = rowA->values[j].as.number + rowB->values[j].as.number;
            array_append(new_row, (Value){VAL_NUMBER, {.number = sum}});
        }
        array_append(result, (Value){VAL_ARRAY, {.array = new_row}});
    }

    return (Value){VAL_ARRAY, {.array = result}};
}

Value native_matrix_sub(int arg_count, Value* args) {
    ValueArray* A = args[0].as.array;
    ValueArray* B = args[1].as.array;

    int rows = A->count;
    int cols = A->values[0].as.array->count;

    if (rows != B->count || cols != B->values[0].as.array->count) {
        return (Value){VAL_NIL};
    }

    ValueArray* result = array_new();

    for (int i = 0; i < rows; i++) {
        ValueArray* rowA = A->values[i].as.array;
        ValueArray* rowB = B->values[i].as.array;
        ValueArray* new_row = array_new();

        for (int j = 0; j < cols; j++) {
            double diff = rowA->values[j].as.number - rowB->values[j].as.number;
            array_append(new_row, (Value){VAL_NUMBER, {.number = diff}});
        }
        array_append(result, (Value){VAL_ARRAY, {.array = new_row}});
    }

    return (Value){VAL_ARRAY, {.array = result}};
}

double calculate_determinant(ValueArray* matrix) {
    int n = matrix->count;

    if (n == 1) return matrix->values[0].as.array->values[0].as.number;
    
    if (n == 2) {
        double a = matrix->values[0].as.array->values[0].as.number;
        double b = matrix->values[0].as.array->values[1].as.number;
        double c = matrix->values[1].as.array->values[0].as.number;
        double d = matrix->values[1].as.array->values[1].as.number;
        return (a * d) - (b * c);
    }

    double det = 0;
    int sign = 1;
    ValueArray* first_row = matrix->values[0].as.array;

    for (int j = 0; j < n; j++) {
        ValueArray* minor = get_minor(matrix, j);
        det += sign * first_row->values[j].as.number * calculate_determinant(minor);
        sign = -sign;
    }
    return det;
}

Value native_matrix_det(int arg_count, Value* args) {
    ValueArray* matrix = args[0].as.array;
    int rows = matrix->count;
    int cols = matrix->values[0].as.array->count;

    if (rows != cols) return (Value){VAL_NIL};

    double result = calculate_determinant(matrix);
    return (Value){VAL_NUMBER, {.number = result}};
}

Value native_matrix_scalar_mul(int arg_count, Value* args) {
    ValueArray* M = args[0].as.array;
    double scalar = args[1].as.number;
    int rows = M->count;
    int cols = M->values[0].as.array->count;

    ValueArray* result = array_new();
    for (int i = 0; i < rows; i++) {
        ValueArray* new_row = array_new();
        for (int j = 0; j < cols; j++) {
            double val = M->values[i].as.array->values[j].as.number;
            array_append(new_row, (Value){VAL_NUMBER, {.number = val * scalar}});
        }
        array_append(result, (Value){VAL_ARRAY, {.array = new_row}});
    }
    return (Value){VAL_ARRAY, {.array = result}};
}

Value native_read_csv(int arg_count, Value* args) {
    const char* filename = args[0].as.string;
    FILE* file = fopen(filename, "r");
    if (!file) return (Value){VAL_NIL};

    ValueArray* matrix = array_new();
    char line[4096];

    while (fgets(line, sizeof(line), file)) {
        line[strcspn(line, "\r\n")] = 0;
        if (strlen(line) == 0) continue;

        ValueArray* row = array_new();
        char* token = strtok(line, ",");
        
        while (token) {
            char* end;
            double val = strtod(token, &end);
            if (end != token) {
                array_append(row, (Value){VAL_NUMBER, {.number = val}});
            }
            token = strtok(NULL, ",");
        }
        
        if (row->count > 0) {
            array_append(matrix, (Value){VAL_ARRAY, {.array = row}});
        }
    }

    fclose(file);
    return (Value){VAL_ARRAY, {.array = matrix}};
}

Value native_load_csv_smart(int arg_count, Value* args) {
    const char* filename = args[0].as.string;
    FILE* file = fopen(filename, "r");
    if (!file) return (Value){VAL_NIL};

    ValueArray* all_rows = array_new();
    char line[10240]; 

    while (fgets(line, sizeof(line), file)) {
        ValueArray* row = array_new();
        char* token = strtok(line, ",");
        while (token) {
            token[strcspn(token, "\r\n")] = 0;
            
            char* endptr;
            double val = strtod(token, &endptr);
            
            
            if (endptr == token) {
                array_append(row, (Value){VAL_STRING, {.string = strdup(token)}});
            } else {
                array_append(row, (Value){VAL_NUMBER, {.number = val}});
            }
            token = strtok(NULL, ",");
        }
        array_append(all_rows, (Value){VAL_ARRAY, {.array = row}});
    }
    fclose(file);
    return (Value){VAL_ARRAY, {.array = all_rows}};
}

Value native_tensor_add(int arg_count, Value* args) {
    ValueArray* a = args[0].as.array;
    double scalar = args[1].as.number;
    ValueArray* result = array_new();

    for (int i = 0; i < a->count; i++) {
        double val = a->values[i].as.number;
        array_append(result, (Value){VAL_NUMBER, {.number = val + scalar}});
    }
    return (Value){VAL_ARRAY, {.array = result}};
}

Value native_tensor_sub(int arg_count, Value* args) {
    ValueArray* a = args[0].as.array;
    double scalar = args[1].as.number;
    ValueArray* result = array_new();

    for (int i = 0; i < a->count; i++) {
        double val = a->values[i].as.number;
        array_append(result, (Value){VAL_NUMBER, {.number = val - scalar}});
    }
    return (Value){VAL_ARRAY, {.array = result}};
}

Value native_tensor_mul(int arg_count, Value* args) {
    ValueArray* a = args[0].as.array;
    double scalar = args[1].as.number;
    ValueArray* result = array_new();

    for (int i = 0; i < a->count; i++) {
        double val = a->values[i].as.number;
        array_append(result, (Value){VAL_NUMBER, {.number = val * scalar}});
    }
    return (Value){VAL_ARRAY, {.array = result}};
}

Value native_tensor_check_shape(int arg_count, Value* args) {
    ValueArray* data = args[0].as.array;
    ValueArray* new_shape = args[1].as.array;

    int current_size = data->count;
    long target_size = 1;

    for (int i = 0; i < new_shape->count; i++) {
        target_size *= (int)new_shape->values[i].as.number;
    }

   
    if (current_size == (int)target_size) {
        return (Value){VAL_NUMBER, {.number = 1}}; 
    }
    return (Value){VAL_NUMBER, {.number = 0}};     
}

Value native_save_jml(int arg_count, Value* args) {
    const char* filename = args[0].as.string;
    ValueArray* data = args[1].as.array;
    ValueArray* shape = args[2].as.array;

    FILE* file = fopen(filename, "w");
    if (!file) return (Value){VAL_NUMBER, {.number = 0}};

    for (int i = 0; i < shape->count; i++) {
        fprintf(file, "%f", shape->values[i].as.number);
        if (i < shape->count - 1) fprintf(file, ",");
    }
    fprintf(file, "\n");

    for (int i = 0; i < data->count; i++) {
        fprintf(file, "%f", data->values[i].as.number);
        if (i < data->count - 1) fprintf(file, ",");
    }

    fclose(file);
    return (Value){VAL_NUMBER, {.number = 1}};
}

Value native_load_jml(int arg_count, Value* args) {
    const char* filename = args[0].as.string;
    FILE* file = fopen(filename, "r");
    if (!file) return (Value){VAL_NIL};

    ValueArray* result = array_new(); 
    char line[4096];

    while (fgets(line, sizeof(line), file)) {
        ValueArray* row = array_new();
        char* token = strtok(line, ",");
        while (token) {
            array_append(row, (Value){VAL_NUMBER, {.number = atof(token)}});
            token = strtok(NULL, ",");
        }
        array_append(result, (Value){VAL_ARRAY, {.array = row}});
    }

    fclose(file);
    return (Value){VAL_ARRAY, {.array = result}};
}

Value native_tensor_sum(int arg_count, Value* args) {
    ValueArray* a = args[0].as.array;
    double total = 0;

    for (int i = 0; i < a->count; i++) {
        total += a->values[i].as.number;
    }
    return (Value){VAL_NUMBER, {.number = total}};
}

Value native_tensor_mean(int arg_count, Value* args) {
    ValueArray* a = args[0].as.array;
    if (a->count == 0) return (Value){VAL_NUMBER, {.number = 0}};
    
    Value sum_val = native_tensor_sum(arg_count, args);
    return (Value){VAL_NUMBER, {.number = sum_val.as.number / a->count}};
}

Value native_tensor_dot(int arg_count, Value* args) {
    ValueArray* data_a = args[0].as.array;
    ValueArray* shape_a = args[1].as.array;
    ValueArray* data_b = args[2].as.array;
    ValueArray* shape_b = args[3].as.array;

    int r1 = (int)shape_a->values[0].as.number;
    int c1 = (int)shape_a->values[1].as.number;
    int r2 = (int)shape_b->values[0].as.number;
    int c2 = (int)shape_b->values[1].as.number;

    if (c1 != r2) return (Value){VAL_NIL}; 

    ValueArray* result = array_new();
    for (int i = 0; i < r1; i++) {
        for (int j = 0; j < c2; j++) {
            double sum = 0;
            for (int k = 0; k < c1; k++) {
                sum += data_a->values[i * c1 + k].as.number * data_b->values[k * c2 + j].as.number;
            }
            array_append(result, (Value){VAL_NUMBER, {.number = sum}});
        }
    }
    return (Value){VAL_ARRAY, {.array = result}};
}

Value native_vector_dot(int arg_count, Value* args) {
    ValueArray* a = args[0].as.array;
    ValueArray* b = args[1].as.array;
    double result = 0;

    for (int i = 0; i < a->count; i++) {
        result += a->values[i].as.number * b->values[i].as.number;
    }
    return (Value){VAL_NUMBER, {.number = result}};
}

Value native_vector_norm(int arg_count, Value* args) {
    ValueArray* a = args[0].as.array;
    double sum_sq = 0;

    for (int i = 0; i < a->count; i++) {
        double val = a->values[i].as.number;
        sum_sq += val * val;
    }
    return (Value){VAL_NUMBER, {.number = sqrt(sum_sq)}};
}

Value native_math_acos(int arg_count, Value* args) {
    if (args[0].type != VAL_NUMBER) {
        return (Value){VAL_NIL};
    }
    
    double val = args[0].as.number;
    
    if (val < -1.0) val = -1.0;
    if (val > 1.0) val = 1.0;
    
    return (Value){VAL_NUMBER, {.number = acos(val)}};
}