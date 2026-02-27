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

    ValueArray* va = value_array_new();

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
        
        value_array_push(va, (Value){VAL_MAP, {.map = map}});
    }

    mysql_free_result(result);

    return (Value){VAL_ARRAY, {.array = va}};
}

void register_mysql_natives(Env *env) {
    MSQL_REGISTER(env, "__mysql_connect__", native_mysql_connect);
}