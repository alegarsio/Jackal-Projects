#include "csv/native_csv.h"

#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include<sys/stat.h>

#define CSV_REGISTER(env, name, func)                                           \
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

    Value native_read_csv(int arg_count, Value *args)
{
    const char *filename = args[0].as.string;
    FILE *file = fopen(filename, "r");
    if (!file)
        return (Value){VAL_NIL};

    ValueArray *matrix = array_new();
    char line[4096];

    while (fgets(line, sizeof(line), file))
    {
        line[strcspn(line, "\r\n")] = 0;
        if (strlen(line) == 0)
            continue;

        ValueArray *row = array_new();
        char *token = strtok(line, ",");

        while (token)
        {
            char *end;
            double val = strtod(token, &end);
            if (end != token)
            {
                array_append(row, (Value){VAL_NUMBER, {.number = val}});
            }
            token = strtok(NULL, ",");
        }

        if (row->count > 0)
        {
            array_append(matrix, (Value){VAL_ARRAY, {.array = row}});
        }
    }

    fclose(file);
    return (Value){VAL_ARRAY, {.array = matrix}};
}

void register_csv_natives(Env *env){

}