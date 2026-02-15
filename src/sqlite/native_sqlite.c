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

Value native_db_open(int arity, Value *args)
{
    sqlite3 *db;
    if (sqlite3_open(args[0].as.string, &db) != SQLITE_OK)
    {
        return (Value){VAL_NIL, {0}};
    }
    return (Value){VAL_FILE, {.file = (FILE *)db}};
}

void register_sqlite_native(Env *env)
{
    SQLITE_REGISTER(env,"__db_open",native_db_open);
}