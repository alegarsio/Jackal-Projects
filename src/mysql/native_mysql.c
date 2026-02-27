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

#ifdef USE_MYSQL
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

#else

Value native_mysql_connect(int arity, Value* args) {
    printf("Jackal Error: MySQL support is not enabled in this binary.\n");
    return (Value){VAL_NIL};
}

#endif

void register_mysql_natives(Env *env) {
    MSQL_REGISTER(env, "__mysql_connect__", native_mysql_connect);
}