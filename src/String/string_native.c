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

Value native_string_endswith(int arg_count, Value* args) {
    if (arg_count < 2 || args[0].type != VAL_STRING || args[1].type != VAL_STRING) {
        return (Value){VAL_NUMBER, {.number = 0}};
    }

    const char* str = args[0].as.string;
    const char* suffix = args[1].as.string;
    size_t str_len = strlen(str);
    size_t suffix_len = strlen(suffix);

    if (suffix_len > str_len) return (Value){VAL_NUMBER, {.number = 0}};

    if (strcmp(str + str_len - suffix_len, suffix) == 0) {
        return (Value){VAL_NUMBER, {.number = 1}};
    }
    return (Value){VAL_NUMBER, {.number = 0}};
}

Value native_string_contains(int arg_count, Value* args) {
    if (arg_count < 2 || args[0].type != VAL_STRING || args[1].type != VAL_STRING) {
        return (Value){VAL_BOOL, {.boolean = false}};
    }

    const char* str = args[0].as.string;
    const char* needle = args[1].as.string;

    if (strstr(str, needle) != NULL) {
        return (Value){VAL_BOOL, {.boolean = true}};
    }
    return (Value){VAL_BOOL, {.boolean = false}};
}

Value native_string_replace(int arg_count, Value* args) {
    if (arg_count < 3 || args[0].type != VAL_STRING || 
        args[1].type != VAL_STRING || args[2].type != VAL_STRING) {
        return (Value){VAL_NIL, {0}};
    }

    const char* str = args[0].as.string;
    const char* old_sub = args[1].as.string;
    const char* new_sub = args[2].as.string;

    if (strlen(old_sub) == 0) return (Value){VAL_STRING, {.string = strdup(str)}};

    char *result;
    int i, count = 0;
    size_t new_len = strlen(new_sub);
    size_t old_len = strlen(old_sub);

    for (i = 0; str[i] != '\0'; i++) {
        if (strstr(&str[i], old_sub) == &str[i]) {
            count++;
            i += old_len - 1;
        }
    }

    result = (char *)malloc(i + count * (new_len - old_len) + 1);
    if (result == NULL) return (Value){VAL_NIL, {0}};

    i = 0;
    while (*str) {
        if (strstr(str, old_sub) == str) {
            strcpy(&result[i], new_sub);
            i += new_len;
            str += old_len;
        } else {
            result[i++] = *str++;
        }
    }
    result[i] = '\0';

    return (Value){VAL_STRING, {.string = result}};
}
Value native_string_trim(int arg_count, Value* args) {
    if (arg_count < 1 || args[0].type != VAL_STRING) return (Value){VAL_NIL, {0}, NULL};
    
    char* str = args[0].as.string;
    char* end;

    while(isspace((unsigned char)*str)) str++;

    if(*str == 0) return (Value){VAL_STRING, {.string = strdup("")}, NULL};

    end = str + strlen(str) - 1;
    while(end > str && isspace((unsigned char)*end)) end--;

    int len = (int)(end - str + 1);
    char* trimmed = malloc(len + 1);
    strncpy(trimmed, str, len);
    trimmed[len] = '\0';

    return (Value){VAL_STRING, {.string = trimmed}, NULL};
}
void register_string_natives(Env* env) {
    STRING_REGISTER(env, "__str_toUpper", native_string_uppercase);
    STRING_REGISTER(env, "__str_toLower", native_string_lowercase);
    STRING_REGISTER(env, "__str_startsWith", native_string_startswith);
    STRING_REGISTER(env, "__str_endsWith", native_string_endswith);
    STRING_REGISTER(env, "__str_contains", native_string_contains);
    STRING_REGISTER(env, "__str_replace", native_string_replace);
    
}