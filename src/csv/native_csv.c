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

Value native_read_csv(int arity, Value *args) {
    if (arity < 1 || args[0].type != VAL_STRING) {
        print_error("csv.read() expects 1 string argument (file path)");
        return (Value){VAL_NIL, {0}};
    }

    const char *path = args[0].as.string;
    FILE *file = fopen(path, "r");

    if (!file) {
        print_error("Could not open file: %s", path);
        return (Value){VAL_NIL, {0}};
    }

    ValueArray *matrix = array_new();
    char line[1024];

    while (fgets(line, sizeof(line), file)) {
        line[strcspn(line, "\r\n")] = 0;

        ValueArray *row = array_new();
        char *token = strtok(line, ",");

        while (token != NULL) {
            Value val_str = (Value){VAL_STRING, {.string = strdup(token)}};
            array_append(row, val_str);
            token = strtok(NULL, ",");
        }

        array_append(matrix, (Value){VAL_ARRAY, {.array = row}});
    }

    fclose(file);
    return (Value){VAL_ARRAY, {.array = matrix}};
}
Value native_csv_select(int arity, Value *args) {
    if (arity < 2 || args[0].type != VAL_ARRAY || args[1].type != VAL_ARRAY) {
        print_error("csv_select expects (Array data, Array columns)");
        return (Value){VAL_NIL, {0}};
    }

    ValueArray *matrix = args[0].as.array;
    ValueArray *target_cols = args[1].as.array;

    if (matrix->count == 0) return (Value){VAL_ARRAY, {.array = array_new()}};

    ValueArray *header = matrix->values[0].as.array;
    ValueArray *result_matrix = array_new();
    
    int col_indices[32];
    int col_count = 0;

    for (int i = 0; i < target_cols->count && i < 32; i++) {
        int found_idx = -1;
        for (int j = 0; j < header->count; j++) {
            if (header->values[j].type == VAL_STRING &&
                strcmp(header->values[j].as.string, target_cols->values[i].as.string) == 0) {
                found_idx = j;
                break;
            }
        }
        if (found_idx != -1) {
            col_indices[col_count++] = found_idx;
        }
    }

    for (int i = 0; i < matrix->count; i++) {
        ValueArray *row = matrix->values[i].as.array;
        ValueArray *new_row = array_new();

        for (int j = 0; j < col_count; j++) {
            int target_idx = col_indices[j];
            if (target_idx < row->count) {
                array_append(new_row, copy_value(row->values[target_idx]));
            }
        }
        array_append(result_matrix, (Value){VAL_ARRAY, {.array = new_row}});
    }

    return (Value){VAL_ARRAY, {.array = result_matrix}};
}

alue native_csv_aggregate(int arity, Value *args) {
    if (arity < 3 || args[0].type != VAL_ARRAY || args[1].type != VAL_STRING || args[2].type != VAL_STRING) {
        print_error("csv_aggregate expects (Array data, String column, String op)");
        return (Value){VAL_NIL, {0}};
    }

    ValueArray *matrix = args[0].as.array;
    const char *col_name = args[1].as.string;
    const char *op = args[2].as.string;

    if (matrix->count < 2) return (Value){VAL_NUMBER, {.number = 0}};

    ValueArray *header = matrix->values[0].as.array;
    int col_idx = -1;
    for (int i = 0; i < header->count; i++) {
        if (header->values[i].type == VAL_STRING && strcmp(header->values[i].as.string, col_name) == 0) {
            col_idx = i;
            break;
        }
    }

    if (col_idx == -1) return (Value){VAL_NIL, {0}};

    double sum = 0;
    double min = DBL_MAX;
    double max = -DBL_MAX;
    int count = 0;

    for (int i = 1; i < matrix->count; i++) {
        ValueArray *row = matrix->values[i].as.array;
        if (col_idx < row->count && row->values[col_idx].type == VAL_STRING) {
            double val = atof(row->values[col_idx].as.string);
            sum += val;
            if (val < min) min = val;
            if (val > max) max = val;
            count++;
        }
    }

    if (strcmp(op, "sum") == 0) return (Value){VAL_NUMBER, {.number = sum}};
    if (strcmp(op, "avg") == 0) return (Value){VAL_NUMBER, {.number = count > 0 ? sum / count : 0}};
    if (strcmp(op, "min") == 0) return (Value){VAL_NUMBER, {.number = min == DBL_MAX ? 0 : min}};
    if (strcmp(op, "max") == 0) return (Value){VAL_NUMBER, {.number = max == -DBL_MAX ? 0 : max}};

    return (Value){VAL_NIL, {0}};
}

void register_csv_natives(Env *env){

    CSV_REGISTER(env,"__csv_read",native_read_csv);
    CSV_REGISTER(env,"__csv_select",native_csv_select);

}