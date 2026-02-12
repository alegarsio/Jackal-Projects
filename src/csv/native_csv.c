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

int get_column_index(ValueArray *header, const char *col_name) {
    for (int i = 0; i < header->count; i++) {
        if (header->values[i].type == VAL_STRING && 
            strcmp(header->values[i].as.string, col_name) == 0) {
            return i;
        }
    }
    return -1; 
}

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

Value native_csv_select(int arity, Value *args) {
    if (arity < 2 || args[0].type != VAL_ARRAY || args[1].type != VAL_ARRAY) {
        return (Value){VAL_NIL, {0}};
    }

    ValueArray *matrix = args[0].as.array;
    ValueArray *target_cols = args[1].as.array;
    ValueArray *header = matrix->values[0].as.array;

    ValueArray *result = array_new(); 

    for (int i = 0; i < matrix->count; i++) {
        ValueArray *row = matrix->values[i].as.array;
        ValueArray *new_row = array_new();

        for (int j = 0; j < target_cols->count; j++) {
            int col_idx = get_column_index(header, target_cols->values[j].as.string);
            if (col_idx != -1) {
                array_append(new_row, row->values[col_idx]);
            }
        }
        array_append(result, (Value){VAL_ARRAY, {.array = new_row}});
    }

    return (Value){VAL_ARRAY, {.array = result}};
}

void register_csv_natives(Env *env){

    CSV_REGISTER(env,"__csv_read",native_read_csv);

}