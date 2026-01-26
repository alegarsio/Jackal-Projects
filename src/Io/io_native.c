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
void register_io_natives(Env *env)
{
}