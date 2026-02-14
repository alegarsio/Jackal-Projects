#include "csv/native_csv.h"

#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <float.h>
#include "env.h"

static int sort_col_idx = 0;
static bool sort_asc = true;

typedef struct {
    char* key;
    double value;
    double total_value;
    int count;
} GroupEntry;

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

char* trim_whitespace(char* str) {
    char* end;
    while(isspace((unsigned char)*str)) str++;
    if(*str == 0) return str;
    end = str + strlen(str) - 1;
    while(end > str && isspace((unsigned char)*end)) end--;
    end[1] = '\0';
    return str;
}

int get_column_index(ValueArray *header, const char *col_name) {
    for (int i = 0; i < header->count; i++) {
        if (header->values[i].type == VAL_STRING && 
            strcmp(header->values[i].as.string, col_name) == 0) {
            return i;
        }
    }
    return -1; 
}

static int compare_rows(const void *a, const void *b) {
    Value val_a = ((Value*)a)->as.array->values[sort_col_idx];
    Value val_b = ((Value*)b)->as.array->values[sort_col_idx];

    double num_a = (val_a.type == VAL_NUMBER) ? val_a.as.number : (val_a.type == VAL_STRING ? atof(val_a.as.string) : 0);
    double num_b = (val_b.type == VAL_NUMBER) ? val_b.as.number : (val_b.type == VAL_STRING ? atof(val_b.as.string) : 0);

    if (num_a < num_b) return sort_asc ? -1 : 1;
    if (num_a > num_b) return sort_asc ? 1 : -1;
    return 0;
}

void print_separator(int *widths, int col_count) {
    printf("+");
    for (int i = 0; i < col_count; i++) {
        for (int j = 0; j < widths[i] + 2; j++) printf("-");
        printf("+");
    }
    printf("\n");
}

Value native_read_csv(int arity, Value *args) {
    if (arity < 1 || args[0].type != VAL_STRING) {
        print_error("csv_read expects at least 1 argument (path)");
        return (Value){VAL_NIL, {0}};
    }

    const char *path = args[0].as.string;
    const char *delim = (arity > 1 && args[1].type == VAL_STRING) ? args[1].as.string : ",";

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
        char *token = strtok(line, delim);

        while (token != NULL) {
            Value val_str = (Value){VAL_STRING, {.string = strdup(token)}};
            array_append(row, val_str);
            token = strtok(NULL, delim);
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

Value native_csv_aggregate(int arity, Value *args) {
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


Value native_csv_sort(int arity, Value *args) {
    if (arity < 3 || args[0].type != VAL_ARRAY || args[1].type != VAL_STRING || args[2].type != VAL_STRING) {
        return (Value){VAL_NIL, {0}};
    }

    ValueArray *matrix = args[0].as.array;
    const char *col_name = args[1].as.string;
    sort_asc = strcmp(args[2].as.string, "asc") == 0;

    if (matrix->count < 2) return args[0];

    ValueArray *header = matrix->values[0].as.array;
    sort_col_idx = -1;
    for (int i = 0; i < header->count; i++) {
        if (header->values[i].type == VAL_STRING && strcmp(header->values[i].as.string, col_name) == 0) {
            sort_col_idx = i;
            break;
        }
    }

    if (sort_col_idx == -1) return args[0];

    qsort(&matrix->values[1], matrix->count - 1, sizeof(Value), compare_rows);

    return args[0];
}
Value native_csv_write(int arity, Value *args) {
    if (arity < 2 || args[0].type != VAL_STRING || args[1].type != VAL_ARRAY) {
        print_error("csv_write expects (string path, Array data)");
        return (Value){VAL_NIL, {0}};
    }

    const char *path = args[0].as.string;
    ValueArray *matrix = args[1].as.array;
    FILE *file = fopen(path, "w");

    if (!file) {
        print_error("Could not create file: %s", path);
        return (Value){VAL_NIL, {0}};
    }

    for (int i = 0; i < matrix->count; i++) {
        ValueArray *row = matrix->values[i].as.array;
        for (int j = 0; j < row->count; j++) {
            const char *str = value_to_string(row->values[j]);
            fprintf(file, "%s", str);
            
            if (j < row->count - 1) {
                fprintf(file, ",");
            }
            free((void*)str);
        }
        fprintf(file, "\n");
    }

    fclose(file);
    return (Value){VAL_BOOL, {.boolean = true}};
}


Value native_csv_group_by(int arity, Value *args) {
    if (arity < 4) {
        print_error("groupBy expects (Array data, String group_col, String target_col, String op)");
        return (Value){VAL_NIL, {0}};
    }

    ValueArray *matrix = args[0].as.array;
    char *group_col_name = args[1].as.string;
    char *target_col_name = args[2].as.string;
    char *op = args[3].as.string;

    ValueArray *header = matrix->values[0].as.array;
    int g_idx = -1, t_idx = -1;

    for (int i = 0; i < header->count; i++) {
        char* h_name = strdup(header->values[i].as.string);
        char* clean_h = trim_whitespace(h_name);
        if (strcmp(clean_h, group_col_name) == 0) g_idx = i;
        if (strcmp(clean_h, target_col_name) == 0) t_idx = i;
        free(h_name);
    }

    if (g_idx == -1 || t_idx == -1) {
        print_error("GroupBy Error: Column not found.");
        return (Value){VAL_NIL, {0}};
    }

    GroupEntry groups[512]; 
    int group_count = 0;

    for (int i = 1; i < matrix->count; i++) {
        ValueArray *row = matrix->values[i].as.array;
        char* raw_key = row->values[g_idx].as.string;
        
        char* key_copy = strdup(raw_key);
        char* key = trim_whitespace(key_copy);
        
        double val = (row->values[t_idx].type == VAL_NUMBER) ? 
                      row->values[t_idx].as.number : atof(row->values[t_idx].as.string);

        int found_idx = -1;
        for (int k = 0; k < group_count; k++) {
            if (strcmp(groups[k].key, key) == 0) {
                found_idx = k;
                break;
            }
        }

        if (found_idx != -1) {
            groups[found_idx].total_value += val;
            groups[found_idx].count++;
            free(key_copy); 
        } else {
            groups[group_count].key = strdup(key);
            groups[group_count].total_value = val;
            groups[group_count].count = 1;
            group_count++;
            free(key_copy);
        }
    }

    ValueArray *res_matrix = array_new();
    
    ValueArray *h_row = array_new();
    array_append(h_row, (Value){VAL_STRING, {.string = strdup(group_col_name)}});
    array_append(h_row, (Value){VAL_STRING, {.string = strdup("result")}});
    array_append(res_matrix, (Value){VAL_ARRAY, {.array = h_row}});

    for (int i = 0; i < group_count; i++) {
        ValueArray *d_row = array_new();
        array_append(d_row, (Value){VAL_STRING, {.string = groups[i].key}});
        
        double final_val = groups[i].total_value;
        if (strcmp(op, "count") == 0) final_val = (double)groups[i].count;
        else if (strcmp(op, "avg") == 0) final_val /= groups[i].count;

        array_append(d_row, (Value){VAL_NUMBER, {.number = final_val}});
        array_append(res_matrix, (Value){VAL_ARRAY, {.array = d_row}});
    }

    return (Value){VAL_ARRAY, {.array = res_matrix}};
}

Value native_csv_join(int arity, Value *args) {
    if (arity < 4) {
        print_error("join expects (Array left, Array right, String left_key, String right_key)");
        return (Value){VAL_NIL, {0}};
    }

    ValueArray *left_matrix = args[0].as.array;
    ValueArray *right_matrix = args[1].as.array;
    char *left_key = trim_ws(args[2].as.string);
    char *right_key = trim_ws(args[3].as.string);

    ValueArray *left_header = left_matrix->values[0].as.array;
    ValueArray *right_header = right_matrix->values[0].as.array;

    int l_idx = -1, r_idx = -1;
    for (int i = 0; i < left_header->count; i++) {
        if (strcmp(trim_ws(left_header->values[i].as.string), left_key) == 0) l_idx = i;
    }
    for (int i = 0; i < right_header->count; i++) {
        if (strcmp(trim_ws(right_header->values[i].as.string), right_key) == 0) r_idx = i;
    }

    if (l_idx == -1 || r_idx == -1) return (Value){VAL_NIL, {0}};

    ValueArray *result_matrix = array_new();

    ValueArray *new_header = array_new();
    for (int i = 0; i < left_header->count; i++) array_append(new_header, left_header->values[i]);
    for (int i = 0; i < right_header->count; i++) {
        if (i == r_idx) continue; 
        array_append(new_header, right_header->values[i]);
    }
    array_append(result_matrix, (Value){VAL_ARRAY, {.array = new_header}});

    for (int i = 1; i < left_matrix->count; i++) {
        ValueArray *l_row = left_matrix->values[i].as.array;
        char *l_val = l_row->values[l_idx].as.string;

        for (int j = 1; j < right_matrix->count; j++) {
            ValueArray *r_row = right_matrix->values[j].as.array;
            char *r_val = r_row->values[r_idx].as.string;

            if (strcmp(trim_ws(l_val), trim_ws(r_val)) == 0) {
                ValueArray *joined_row = array_new();
                for (int k = 0; k < l_row->count; k++) array_append(joined_row, l_row->values[k]);
                for (int k = 0; k < r_row->count; k++) {
                    if (k == r_idx) continue;
                    array_append(joined_row, r_row->values[k]);
                }
                array_append(result_matrix, (Value){VAL_ARRAY, {.array = joined_row}});
            }
        }
    }

    return (Value){VAL_ARRAY, {.array = result_matrix}};
}

void register_csv_natives(Env *env){

    CSV_REGISTER(env,"__csv_read",native_read_csv);
    CSV_REGISTER(env,"__csv_select",native_csv_select);
    CSV_REGISTER(env,"__csv_agregate",native_csv_aggregate);
    CSV_REGISTER(env,"__csv_sort",native_csv_sort);
    CSV_REGISTER(env,"__csv_write",native_csv_write);
    CSV_REGISTER(env,"__csv_group_by",native_csv_group_by);
    CSV_REGISTER(env,"__csv_join",native_csv_join);

}