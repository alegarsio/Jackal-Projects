#include "String/string_native.h"
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#define STRING_REGISTER(env, name, func) \
    do { \
        if (func != NULL) { \
            set_var(env, name, (Value){VAL_NATIVE, {.native = func}}, true, ""); \
        } else { \
            printf("Socket Error: Native function '%s' not implemented!\n", name); \
        } \
    } while (0)


Value native_string_uppercase(int arg_count, Value* args) {
    if (arg_count < 1 || args[0].type != VAL_STRING) {
        return (Value){VAL_NIL, {0}};
    }

    const char* source = args[0].as.string;
    int length = strlen(source);
    
    char* result = malloc(length + 1);
    
    for (int i = 0; i < length; i++) {
        result[i] = toupper((unsigned char)source[i]);
    }
    result[length] = '\0';

    Value res_val = (Value){VAL_STRING, {.string = result}};
    return res_val;
}

Value native_string_lowercase(int arg_count, Value* args) {
    if (arg_count < 1 || args[0].type != VAL_STRING) {
        return (Value){VAL_NIL, {0}};
    }

    const char* source = args[0].as.string;
    int length = strlen(source);
    
    char* result = malloc(length + 1);
    if (result == NULL) return (Value){VAL_NIL, {0}};
    
    for (int i = 0; i < length; i++) {
        result[i] = (char)tolower((unsigned char)source[i]);
    }
    result[length] = '\0';

    Value res_val = (Value){VAL_STRING, {.string = result}};
    return res_val;
}

Value native_string_startswith(int arg_count, Value* args) {
    if (arg_count < 2 || args[0].type != VAL_STRING || args[1].type != VAL_STRING) {
        return (Value){VAL_NUMBER, {.number = 0}};
    }

    const char* str = args[0].as.string;
    const char* prefix = args[1].as.string;
    size_t str_len = strlen(str);
    size_t prefix_len = strlen(prefix);

    if (prefix_len > str_len) return (Value){VAL_NUMBER, {.number = 0}};

    if (strncmp(str, prefix, prefix_len) == 0) {
        return (Value){VAL_NUMBER, {.number = 1}};
    }
    return (Value){VAL_NUMBER, {.number = 0}};
}



void register_string_natives(Env* env) {
    STRING_REGISTER(env, "__str_toUpper", native_string_uppercase);
    STRING_REGISTER(env, "__str_toLower", native_string_lowercase);
    STRING_REGISTER(env, "__str_startsWith", native_string_startswith);
    
}