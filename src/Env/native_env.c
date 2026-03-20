#include "Env/native_env.h"

#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <float.h>
#include "env.h"

#define ENV_REGISTER(env, name, func)                                           \
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


Value native_env_load(int arity, Value *args) {
    if (arity < 1 || args[0].type != VAL_STRING)
        return (Value){VAL_NIL};

    FILE *file = fopen(args[0].as.string, "r");
    if (!file) return (Value){VAL_NIL};

    HashMap* env_map = map_new();
    char line[1024];

    while (fgets(line, sizeof(line), file)) {
        line[strcspn(line, "\r\n")] = 0;

        char* trimmed = line;
        while(*trimmed == ' ') trimmed++;
        if (*trimmed == '\0' || *trimmed == '#') continue;

        char* delimiter = strchr(trimmed, '=');
        if (delimiter) {
            *delimiter = '\0';
            char* key = trimmed;
            char* value = delimiter + 1;

            char* end = key + strlen(key) - 1;
            while(end > key && *end == ' ') { *end = '\0'; end--; }
            while(*value == ' ') value++;

            map_set(env_map, strdup(key), (Value){VAL_STRING, {.string = strdup(value)}});
        }
    }

    fclose(file);
    return (Value){VAL_MAP, {.map = env_map}};
}



void register_env_natives(Env *env){
    ENV_REGISTER(env,"__env_load__",native_env_load);
}