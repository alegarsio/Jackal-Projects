#include"mysql/native_mysql.h"


#include <stdlib.h>
#include <errno.h>
#include <sys/stat.h>
#include "env.h"

#define MSQL_REGISTER(env, name, func)                                           \
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

#ifdef USE_MYSQL
#include <mysql/mysql.h>

Value native_mysql_connect(int arity, Value* args) {
    MYSQL *conn = mysql_init(NULL);
    if (!mysql_real_connect(conn, args[0].as.string, args[1].as.string, 
                            args[2].as.string, args[3].as.string, 0, NULL, 0)) {
        return (Value){VAL_NIL};
    }
    return (Value){VAL_NUMBER, {.number = (uintptr_t)conn}};
}
#endif
void register_mysql_natives(Env *env){

}