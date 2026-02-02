#include "buffer/native_buffer.h"
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <math.h>
#include<sys/stat.h>
#include<curl/curl.h>


#define BUFFER_REGISTER(env, name, func)                                           \
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

void register_buffer_natives(Env *env){

}