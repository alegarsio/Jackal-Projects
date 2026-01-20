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

void register_string_natives(Env* env) {
    STRING_REGISTER(env, "__str_toUpper", native_string_uppercase);
    STRING_REGISTER(env, "__str_toLower", native_string_lowercase);
}