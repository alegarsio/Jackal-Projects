#include "mysql/native_mysql.h"
#include <stdlib.h>
#include <errno.h>
#include <sys/stat.h>
#include <stdio.h>
#include <string.h>
#include "env.h"

#if __has_include(<mysql/mysql.h>)
    #include <mysql/mysql.h>
    #define HAS_MYSQL 1
#else
    #define HAS_MYSQL 0
#endif

#define MSQL_REGISTER(env, name, func)                                           \
    do                                                                           \
    {                                                                            \
        set_var(env, name, (Value){VAL_NATIVE, {.native = func}}, true, "");     \
    } while (0)


Value native_mysql_connect(int arity, Value* args) {
#if HAS_MYSQL
    if (arity < 4) return (Value){VAL_NIL};

    MYSQL *conn = mysql_init(NULL);
    if (!mysql_real_connect(conn, args[0].as.string, args[1].as.string, 
                            args[2].as.string, args[3].as.string, 0, NULL, 0)) {
        return (Value){VAL_NIL};
    }
    return (Value){VAL_NUMBER, {.number = (uintptr_t)conn}};
#else
    fprintf(stderr, "Error: MySQL support not compiled into Jackal.\n");
    return (Value){VAL_NIL};
#endif
}

Value native_mysql_query(int arity, Value* args) {
#if HAS_MYSQL
    if (arity < 2 || args[0].type != VAL_NUMBER) {
        return (Value){VAL_NIL};
    }

    MYSQL *conn = (MYSQL*)(uintptr_t)args[0].as.number;
    const char* sql = args[1].as.string;

    if (mysql_query(conn, sql)) {
        fprintf(stderr, "MySQL Query Error: %s\n", mysql_error(conn));
        return (Value){VAL_NIL};
    }

    MYSQL_RES *result = mysql_store_result(conn);
    
    if (result == NULL) {
        if (mysql_field_count(conn) == 0) {
            return (Value){VAL_BOOL, {.boolean = 1}};
        } else {
            return (Value){VAL_NIL};
        }
    }

    int num_fields = mysql_num_fields(result);
    MYSQL_FIELD *fields = mysql_fetch_fields(result);
    MYSQL_ROW row;

    ValueArray* va = array_new();

    while ((row = mysql_fetch_row(result))) {
        HashMap* map = map_new();
        for (int i = 0; i < num_fields; i++) {
            Value val;
            if (row[i]) {
                val = (Value){VAL_STRING, {.string = strdup(row[i])}};
            } else {
                val = (Value){VAL_NIL};
            }
            map_set(map, fields[i].name, val);
        }
        array_append(va, (Value){VAL_MAP, {.map = map}});
    }

    mysql_free_result(result);
    return (Value){VAL_ARRAY, {.array = va}};
#else
    return (Value){VAL_NIL};
#endif
}

Value native_mysql_create_table(int arity, Value* args) {
#if HAS_MYSQL
    if (arity < 3 || args[0].type != VAL_NUMBER || args[2].type != VAL_MAP) {
        return (Value){VAL_NIL};
    }

    MYSQL *conn = (MYSQL*)(uintptr_t)args[0].as.number;
    const char* table_name = args[1].as.string;
    HashMap* columns = args[2].as.map;

    char sql[2048];
    sprintf(sql, "CREATE TABLE IF NOT EXISTS %s (", table_name);

    for (int i = 0; i < columns->capacity; i++) {
        if (columns->entries[i].key != NULL) {
            strcat(sql, columns->entries[i].key);
            strcat(sql, " ");

            const char* user_type = columns->entries[i].value.as.string;
            
            if (strcmp(user_type, "int") == 0) strcat(sql, "INT");
            else if (strcmp(user_type, "string") == 0) strcat(sql, "VARCHAR(255)");
            else if (strcmp(user_type, "text") == 0) strcat(sql, "TEXT");
            else if (strcmp(user_type, "primary") == 0) strcat(sql, "INT AUTO_INCREMENT PRIMARY KEY");
            else strcat(sql, user_type);

            strcat(sql, ", ");
        }
    }

    int len = strlen(sql);
    if (len > 0 && sql[len - 2] == ',') sql[len - 2] = '\0';
    strcat(sql, ");");

    if (mysql_query(conn, sql)) {
        fprintf(stderr, "MySQL Create Error: %s\n", mysql_error(conn));
        return (Value){VAL_BOOL, {.boolean = 0}};
    }
    return (Value){VAL_BOOL, {.boolean = 1}};
#else
    return (Value){VAL_BOOL, {.boolean = 0}};
#endif
}

Value native_mysql_insert(int arity, Value* args) {
#if HAS_MYSQL
    if (arity < 3 || args[0].type != VAL_NUMBER || args[2].type != VAL_MAP) {
        return (Value){VAL_NIL};
    }

    MYSQL *conn = (MYSQL*)(uintptr_t)args[0].as.number;
    const char* table_name = args[1].as.string;
    HashMap* data = args[2].as.map;

    char sql[4096];
    char columns[2048] = "";
    char values[2048] = "";

    sprintf(sql, "INSERT INTO %s (", table_name);

    for (int i = 0; i < data->capacity; i++) {
        if (data->entries[i].key != NULL) {
            strcat(columns, data->entries[i].key);
            strcat(columns, ", ");

            Value val = data->entries[i].value;
            if (val.type == VAL_STRING) {
                strcat(values, "'");
                strcat(values, val.as.string);
                strcat(values, "'");
            } else if (val.type == VAL_NUMBER) {
                char num_str[32];
                sprintf(num_str, "%g", val.as.number);
                strcat(values, num_str);
            } else if (val.type == VAL_BOOL) {
                strcat(values, val.as.boolean ? "1" : "0");
            } else {
                strcat(values, "NULL");
            }
            strcat(values, ", ");
        }
    }

    columns[strlen(columns) - 2] = '\0';
    values[strlen(values) - 2] = '\0';

    strcat(sql, columns);
    strcat(sql, ") VALUES (");
    strcat(sql, values);
    strcat(sql, ");");

    if (mysql_query(conn, sql)) {
        fprintf(stderr, "MySQL Insert Error: %s\n", mysql_error(conn));
        return (Value){VAL_BOOL, {.boolean = 0}};
    }
    return (Value){VAL_BOOL, {.boolean = 1}};
#else
    return (Value){VAL_BOOL, {.boolean = 0}};
#endif
}

Value native_mysql_find_all(int arity, Value* args) {
#if HAS_MYSQL
    if (arity < 2 || args[0].type != VAL_NUMBER || args[1].type != VAL_STRING) {
        return (Value){VAL_NIL};
    }

    MYSQL *conn = (MYSQL*)(uintptr_t)args[0].as.number;
    const char* table_name = args[1].as.string;

    char sql[256];
    snprintf(sql, sizeof(sql), "SELECT * FROM %s", table_name);

    if (mysql_query(conn, sql)) {
        fprintf(stderr, "MySQL FindAll Error: %s\n", mysql_error(conn));
        return (Value){VAL_NIL};
    }

    MYSQL_RES *result = mysql_store_result(conn);
    if (result == NULL) return (Value){VAL_NIL};

    int num_fields = mysql_num_fields(result);
    MYSQL_FIELD *fields = mysql_fetch_fields(result);
    MYSQL_ROW row;

    ValueArray* va = array_new();

    while ((row = mysql_fetch_row(result))) {
        HashMap* map = map_new();
        for (int i = 0; i < num_fields; i++) {
            Value val;
            if (row[i]) {
                val = (Value){VAL_STRING, {.string = strdup(row[i])}};
            } else {
                val = (Value){VAL_NIL};
            }
            map_set(map, fields[i].name, val);
        }
        array_append(va, (Value){VAL_MAP, {.map = map}});
    }

    mysql_free_result(result);
    return (Value){VAL_ARRAY, {.array = va}};
#else
    return (Value){VAL_NIL};
#endif
}

Value native_mysql_select_builder(int arity, Value* args) {
#if HAS_MYSQL
    if (arity == 0 || args[0].type == VAL_NIL) return (Value){VAL_STRING, {.string = strdup("*")}};

    char buffer[2048] = "";
    Value input = args[0];

    if (input.type == VAL_MAP) {
        HashMap* map = input.as.map;
        for (int j = 0; j < map->capacity; j++) {
            if (map->entries[j].key != NULL) {
                strcat(buffer, map->entries[j].key);
                strcat(buffer, " AS ");
                if (map->entries[j].value.type == VAL_STRING) {
                    strcat(buffer, map->entries[j].value.as.string);
                }
                strcat(buffer, ", ");
            }
        }
    } else if (input.type == VAL_ARRAY) {
        ValueArray* arr = input.as.array;
        for (int i = 0; i < arr->count; i++) {
            if (arr->values[i].type == VAL_STRING) {
                strcat(buffer, arr->values[i].as.string);
                strcat(buffer, ", ");
            }
        }
    } else if (input.type == VAL_STRING) {
        strcat(buffer, input.as.string);
    }

    int len = strlen(buffer);
    if (len > 1 && buffer[len - 2] == ',') {
        buffer[len - 2] = '\0';
    }

    if (strlen(buffer) == 0) return (Value){VAL_STRING, {.string = strdup("*")}};
    return (Value){VAL_STRING, {.string = strdup(buffer)}};
#else
    return (Value){VAL_STRING, {.string = strdup("*")}};
#endif
}

Value native_mysql_update(int arity, Value* args) {
#if HAS_MYSQL
    if (arity < 4 || args[0].type != VAL_NUMBER || args[2].type != VAL_MAP) {
        return (Value){VAL_BOOL, {.boolean = 0}};
    }

    MYSQL *conn = (MYSQL*)(uintptr_t)args[0].as.number;
    const char* table_name = args[1].as.string;
    HashMap* data = args[2].as.map;
    const char* where_clause = args[3].as.string;

    char sql[4096] = "";
    char set_part[2048] = "";

    for (int i = 0; i < data->capacity; i++) {
        if (data->entries[i].key != NULL) {
            strcat(set_part, data->entries[i].key);
            strcat(set_part, " = ");

            Value val = data->entries[i].value;
            if (val.type == VAL_STRING) {
                strcat(set_part, "'");
                strcat(set_part, val.as.string);
                strcat(set_part, "'");
            } else if (val.type == VAL_NUMBER) {
                char num_str[32];
                sprintf(num_str, "%g", val.as.number);
                strcat(set_part, num_str);
            } else if (val.type == VAL_BOOL) {
                strcat(set_part, val.as.boolean ? "1" : "0");
            } else {
                strcat(set_part, "NULL");
            }
            strcat(set_part, ", ");
        }
    }

    int set_len = strlen(set_part);
    if (set_len > 1 && set_part[set_len - 2] == ',') {
        set_part[set_len - 2] = '\0';
    }

    snprintf(sql, sizeof(sql), "UPDATE %s SET %s %s", table_name, set_part, where_clause);

    if (mysql_query(conn, sql)) {
        fprintf(stderr, "MySQL Update Error: %s\n", mysql_error(conn));
        return (Value){VAL_BOOL, {.boolean = 0}};
    }

    return (Value){VAL_BOOL, {.boolean = 1}};
#else
    return (Value){VAL_BOOL, {.boolean = 0}};
#endif
}

void register_mysql_natives(Env *env) {
    MSQL_REGISTER(env, "__mysql_connect__", native_mysql_connect);
    MSQL_REGISTER(env, "__mysql_query__", native_mysql_query);
    MSQL_REGISTER(env, "__mysql_create__", native_mysql_create_table);
    MSQL_REGISTER(env, "__mysql_insert__", native_mysql_insert);
    MSQL_REGISTER(env, "__mysql_findall__", native_mysql_find_all);
    MSQL_REGISTER(env, "__mysql_select_", native_mysql_select_builder);
    MSQL_REGISTER(env,"__mysql_update__",native_mysql_update);
}