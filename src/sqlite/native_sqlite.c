#include "sqlite/native_sqlite.h"
#include <sqlite3.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include "env.h"

#define SQLITE_REGISTER(env, name, func)                                         \
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

Value native_db_open(int arity, Value *args) {
    static int call_count = 0;
    call_count++;
    printf("--- DEBUG: native_db_execute dipanggil ke-%d ---\n", call_count);

    if (arity < 1 || args[0].type != VAL_STRING) {
        print_error("db_open expects a string path.");
        return (Value){VAL_NIL, {0}};
    }

    const char* db_name = args[0].as.string;
    sqlite3 *db;
    int rc = sqlite3_open(db_name, &db);

    if (rc != SQLITE_OK) {
        printf("SQLite Error: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return (Value){VAL_NIL, {0}};
    }

    printf("Successfully connected to database: %s\n", db_name);
    
    return (Value){VAL_FILE, {.file = (FILE*)db}};
}

Value native_db_execute(int arity, Value *args) {
    if (arity < 2) {
        print_error("db_execute expects 2 arguments: (handle, sql_string)");
        return (Value){VAL_BOOL, {.boolean = false}};
    }

    if (args[0].type == VAL_NIL || args[0].as.file == NULL) {
        return (Value){VAL_BOOL, {.boolean = false}};
    }

    if (args[1].type != VAL_STRING) {
        return (Value){VAL_BOOL, {.boolean = false}};
    }

    sqlite3 *db = (sqlite3*)args[0].as.file;
    const char* sql = args[1].as.string;

    if (sql == NULL) {
        return (Value){VAL_BOOL, {.boolean = false}};
    }

    char *err_msg = NULL;
    int rc = sqlite3_exec(db, sql, NULL, NULL, &err_msg);

    if (rc != SQLITE_OK) {
        if (err_msg != NULL) {
            printf("SQL Error: %s\n", err_msg);
            sqlite3_free(err_msg);
        }
        return (Value){VAL_BOOL, {.boolean = false}};
    }

    return (Value){VAL_BOOL, {.boolean = true}};
}
Value native_db_connect(int arity, Value *args) {
    if (arity < 1 || args[0].type != VAL_STRING) {
        print_error("db_connect expects a string path.");
        return (Value){VAL_NIL, {0}};
    }

    const char* db_name = args[0].as.string;
    sqlite3 *db;
    
    int rc = sqlite3_open_v2(db_name, &db, SQLITE_OPEN_READWRITE, NULL);

    if (rc != SQLITE_OK) {
        sqlite3_close(db);
        return (Value){VAL_NIL, {0}};
    }

    return (Value){VAL_FILE, {.file = (FILE*)db}};
}
Value native_db_query(int arity, Value *args) {
    if (arity < 2) {
        print_error("db_query expects 2 arguments: (handle, sql_string)");
        return (Value){VAL_NIL, {0}};
    }

    sqlite3 *db = (sqlite3*)args[0].as.file;
    if (db == NULL) {
        printf("--- NATIVE DEBUG: dbHandle is NULL ---\n");
        return (Value){VAL_NIL, {0}};
    }

    const char* sql = args[1].as.string;
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        printf("--- NATIVE DEBUG: Prepare Error: %s ---\n", sqlite3_errmsg(db));
        return (Value){VAL_NIL, {0}};
    }

    ValueArray *results = array_new(); 
    int col_count = sqlite3_column_count(stmt);
    int row_count = 0;

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        row_count++;
        
        HashMap *row = map_new(); 

        for (int i = 0; i < col_count; i++) {
            const char *col_name = sqlite3_column_name(stmt, i);
            Value val;

            int type = sqlite3_column_type(stmt, i);
            if (type == SQLITE_INTEGER) {
                val = (Value){VAL_NUMBER, {.number = (double)sqlite3_column_int(stmt, i)}};
            } else if (type == SQLITE_FLOAT) {
                val = (Value){VAL_NUMBER, {.number = sqlite3_column_double(stmt, i)}};
            } else if (type == SQLITE_TEXT) {
                const char *text = (const char*)sqlite3_column_text(stmt, i);
                val = (Value){VAL_STRING, {.string = strdup(text)}};
            } else {
                val = (Value){VAL_NIL, {0}};
            }

            map_set(row, col_name, val);
        }
        
        Value row_val;
        row_val.type = VAL_MAP;
        row_val.as.map = row;
        
        array_append(results, row_val);
    }


    sqlite3_finalize(stmt);

    Value final_result;
    final_result.type = VAL_ARRAY;
    final_result.as.array = results;
    return final_result;
}
void register_sqlite_native(Env *env)
{
    SQLITE_REGISTER(env,"__db_query",native_db_query);
    SQLITE_REGISTER(env,"__db_open",native_db_open);
    SQLITE_REGISTER(env,"__db_execute",native_db_execute);
    SQLITE_REGISTER(env,"__db_connect",native_db_connect);
}