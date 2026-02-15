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

    sqlite3 *db = (sqlite3*)args[0].as.file;
    const char* sql = args[1].as.string;
    char *err_msg = 0;

    int rc = sqlite3_exec(db, sql, 0, 0, &err_msg);

    if (rc != SQLITE_OK) {
        printf("SQL Error: %s\n", err_msg);
        sqlite3_free(err_msg);
        return (Value){VAL_BOOL, {.boolean = false}};
    }

    return (Value){VAL_BOOL, {.boolean = true}};
}

void register_sqlite_native(Env *env)
{
    SQLITE_REGISTER(env,"__db_open",native_db_open);
}