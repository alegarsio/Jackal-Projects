#include "Io/io_native.h"
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#define IO_REGISTER(env, name, func)                                             \
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

char *get_input_raw(const char *prompt)
{
    if (prompt)
    {
        printf("%s", prompt);
        fflush(stdout);
    }
    static char buffer[1024];
    if (fgets(buffer, sizeof(buffer), stdin) == NULL)
        return NULL;
    buffer[strcspn(buffer, "\n")] = 0;
    return buffer;
}

Value native_input_string(int arity, Value *args) {
    char* input = get_input_raw(arity > 0 ? args[0].as.string : NULL);
    if (!input) return (Value){VAL_NIL, {0}};
    return (Value){VAL_STRING, {.string = strdup(input)}};
}

Value native_input_number(int arity, Value *args) {
    char* input = get_input_raw(arity > 0 ? args[0].as.string : NULL);
    if (!input) return (Value){VAL_NUMBER, {.number = 0}};
    return (Value){VAL_NUMBER, {.number = atof(input)}};
}


Value native_reads(int arity, Value *args) {

    if (arity > 0 && args[0].type == VAL_STRING) {
        printf("%s", args[0].as.string);
        fflush(stdout);
    }

    char buffer[1024];
    if (fgets(buffer, sizeof(buffer), stdin) == NULL) {
        return (Value){VAL_NIL, .as = {0}};
    }

    size_t len = strlen(buffer);
    if (len > 0 && buffer[len - 1] == '\n') {
        buffer[len - 1] = '\0';
    }

    char *endptr;
    double num = strtod(buffer, &endptr);

    if (strlen(buffer) == 0) {
        char *empty = malloc(1);
        empty[0] = '\0';
        return (Value){VAL_STRING, {.string = empty}};
    }

    if (*endptr == '\0') {
        return (Value){VAL_NUMBER, {.number = num}};
    }

    char *str_copy = malloc(strlen(buffer) + 1);
    if (str_copy == NULL) return (Value){VAL_NIL, .as = {0}};
    strcpy(str_copy, buffer);

    return (Value){VAL_STRING, {.string = str_copy}};
}

void register_io_natives(Env *env)
{
    IO_REGISTER(env,"__io_string",native_input_string);
    IO_REGISTER(env,"__io_number",native_input_number);
    IO_REGISTER(env,"__io_autostream",native_reads);
}