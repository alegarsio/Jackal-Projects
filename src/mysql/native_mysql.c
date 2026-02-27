#include "mysql/native_mysql.h"
#include <stdlib.h>
#include <errno.h>
#include <sys/stat.h>
#include <stdio.h>
#include "env.h"

#define MSQL_REGISTER(env, name, func)                                           \
    do                                                                           \
    {                                                                            \
        set_var(env, name, (Value){VAL_NATIVE, {.native = func}}, true, "");     \
    } while (0)


#include <mysql/mysql.h>

Value native_mysql_connect(int arity, Value* args) {
    if (arity < 4) return (Value){VAL_NIL};

    MYSQL *conn = mysql_init(NULL);
    if (!mysql_real_connect(conn, args[0].as.string, args[1].as.string, 
                            args[2].as.string, args[3].as.string, 0, NULL, 0)) {
        return (Value){VAL_NIL};
    }
    return (Value){VAL_NUMBER, {.number = (uintptr_t)conn}};
}

Value native_mysql_query(int arity, Value* args) {
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
}

Value native_mysql_create_table(int arity, Value* args) {
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
            
            if (strcmp(user_type, "int") == 0) {
                strcat(sql, "INT");
            } else if (strcmp(user_type, "string") == 0) {
                strcat(sql, "VARCHAR(255)");
            } else if (strcmp(user_type, "text") == 0) {
                strcat(sql, "TEXT");
            } else if (strcmp(user_type, "primary") == 0) {
                strcat(sql, "INT AUTO_INCREMENT PRIMARY KEY");
            } else {
                strcat(sql, user_type);
            }

            strcat(sql, ", ");
        }
    }

    int len = strlen(sql);
    if (len > 0 && sql[len - 2] == ',') {
        sql[len - 2] = '\0';
    }
    strcat(sql, ");");

    if (mysql_query(conn, sql)) {
        fprintf(stderr, "MySQL Create Error: %s\n", mysql_error(conn));
        return (Value){VAL_BOOL, {.boolean = 0}};
    }

    return (Value){VAL_BOOL, {.boolean = 1}};
}

void register_mysql_natives(Env *env) {
    MSQL_REGISTER(env, "__mysql_connect__", native_mysql_connect);
    MSQL_REGISTER(env, "__mysql_query__", native_mysql_query);
}