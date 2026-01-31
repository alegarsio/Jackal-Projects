#include "File/native_file.h"
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <math.h>
#include<sys/stat.h>

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

Value native_file_size(int arity, Value *args) {
    if (arity < 1 || args[0].type != VAL_STRING) {
        return (Value){VAL_NUMBER, {.number = -1}};
    }

    struct stat st;
    if (stat(args[0].as.string, &st) == 0) {
        return (Value){VAL_NUMBER, {.number = (double)st.st_size}};
    }

    return (Value){VAL_NUMBER, {.number = -1}};
}
Value native_file_create(int arity, Value *args) {
    if (arity < 1 || args[0].type != VAL_STRING) {
        return (Value){VAL_BOOL, {.boolean = 0}};
    }

    FILE *file = fopen(args[0].as.string, "w");
    
    if (file) {
        fclose(file);
        return (Value){VAL_BOOL, {.boolean = 1}};
    }

    return (Value){VAL_BOOL, {.boolean = 0}};
}
Value native_file_remove(int arity, Value *args) {
    if (arity < 1 || args[0].type != VAL_STRING) {
        return (Value){VAL_BOOL, {.boolean = 0}};
    }

    if (remove(args[0].as.string) == 0) {
        return (Value){VAL_BOOL, {.boolean = 1}};
    }
    return (Value){VAL_BOOL, {.boolean = 0}};
}
Value native_file_rename(int arity, Value *args) {
    if (arity < 2 || args[0].type != VAL_STRING || args[1].type != VAL_STRING) {
        return (Value){VAL_BOOL, {.boolean = 0}};
    }

    if (rename(args[0].as.string, args[1].as.string) == 0) {
        return (Value){VAL_BOOL, {.boolean = 1}};
    }
    return (Value){VAL_BOOL, {.boolean = 0}};
}
Value native_file_load(int arity, Value *args) {
    if (arity < 1 || args[0].type != VAL_STRING) {
        return (Value){VAL_BOOL, {.boolean = false}};
    }

    const char* path = args[0].as.string;
    FILE* f = fopen(path, "r");
    if (!f) return (Value){VAL_BOOL, {.boolean = false}};

    fseek(f, 0, SEEK_END);
    long len = ftell(f);
    rewind(f);

    char* source = malloc(len + 1);
    if (!source) {
        fclose(f);
        return (Value){VAL_BOOL, {.boolean = false}};
    }

    fread(source, 1, len, f);
    source[len] = '\0';
    fclose(f);

    extern Env* global_env; 
    execute_source(source, global_env);

    free(source);
    return (Value){VAL_BOOL, {.boolean = true}};
}

void register_file_natives(Env *env)
{
    FILE_REGISTER(env, "__ioFile_read", native_file_read);
    FILE_REGISTER(env, "__ioFile_exist", native_file_exists);
    FILE_REGISTER(env,"__ioFile_write",native_file_write);
    FILE_REGISTER(env,"__ioFile_append",native_file_append);
    FILE_REGISTER(env,"__ioFile_size",native_file_size);
    FILE_REGISTER(env,"__ioFile_create",native_file_create);
    FILE_REGISTER(env,"__ioFile_remove",native_file_remove);
    FILE_REGISTER(env,"__ioFile_rename",native_file_rename);
    FILE_REGISTER(env,"__ioFile_load",native_file_load);
}