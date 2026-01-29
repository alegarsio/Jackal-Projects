#include"File/native_file.h"
#include<stdlib.h>
#include<string.h>
#include<errno.h>
#include<math.h>

#define FILE_REGISTER(env, name, func) \
    do { \
        if (func != NULL) { \
            set_var(env, name, (Value){VAL_NATIVE, {.native = func}}, true, ""); \
        } else { \
            printf("Error: Native function '%s' not implemented!\n", name); \
        } \
    } while (0)

    Value native_file_read(int arity, Value *args) {
        if (arity < 1 || args[0].type != VAL_STRING) return (Value){VAL_NIL, {0}};

        FILE *file = fopen(args[0].as.string, "r");
        if (!file) return (Value){VAL_NIL, {0}};

        fseek(file, 0, SEEK_END);
        long fsize = ftell(file);
        fseek(file, 0, SEEK_SET);

        char *string = malloc(fsize + 1);
        fread(string, fsize, 1, file);
        fclose(file);

        string[fsize] = 0;
        return (Value){VAL_STRING, {.string = string}};
    }

void register_file_natives(Env* env){

}