#include "File/native_file.h"
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <math.h>

#define FILE_REGISTER(env, name, func)                                           \
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

Value native_file_read(int arity, Value *args)
{
    if (arity < 1 || args[0].type != VAL_STRING)
        return (Value){VAL_NIL, {0}};

    FILE *file = fopen(args[0].as.string, "r");
    if (!file)
        return (Value){VAL_NIL, {0}};

    fseek(file, 0, SEEK_END);
    long fsize = ftell(file);
    fseek(file, 0, SEEK_SET);

    char *string = malloc(fsize + 1);
    fread(string, fsize, 1, file);
    fclose(file);

    string[fsize] = 0;
    return (Value){VAL_STRING, {.string = string}};
}

Value native_file_exists(int arity, Value *args)
{
    if (arity < 1 || args[0].type != VAL_STRING)
        return (Value){VAL_BOOL, {.boolean = false}};

    FILE *file = fopen(args[0].as.string, "r");

    if (file)
    {
        fclose(file);
        return (Value){VAL_BOOL, {.boolean = true}};
    }

    return (Value){VAL_BOOL, {.boolean = false}};
}

Value native_file_write(int arity, Value *args) {
    if (arity < 2 || args[0].type != VAL_STRING || args[1].type != VAL_STRING) {
        return (Value){VAL_NUMBER, {.number = 0}};
    }

    FILE *file = fopen(args[0].as.string, "w");
    if (!file) return (Value){VAL_NUMBER, {.number = 0}};

    fprintf(file, "%s", args[1].as.string);
    fclose(file);

    return (Value){VAL_NUMBER, {.number = 1}};
}

Value native_file_append(int arity, Value *args) {
    if (arity < 2 || args[0].type != VAL_STRING || args[1].type != VAL_STRING) {
        return (Value){VAL_BOOL, {.boolean = false}};
    }

    FILE *file = fopen(args[0].as.string, "a");
    if (!file) {
        return (Value){VAL_BOOL, {.boolean = false}};
    }

    fprintf(file, "%s", args[1].as.string);
    fclose(file);

    return (Value){VAL_BOOL, {.boolean = true}};
}
void register_file_natives(Env *env)
{
    FILE_REGISTER(env, "__ioFile_read", native_file_read);
    FILE_REGISTER(env, "__ioFile_exist", native_file_exists);
    FILE_REGISTER(env,"__ioFile_write",native_file_write);
}