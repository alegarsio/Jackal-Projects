#include"math/native_math.h"
#include<stdlib.h>
#include<string.h>
#include<errno.h>

#define MATH_REGISTER(env, name, func) \
    do { \
        if (func != NULL) { \
            set_var(env, name, (Value){VAL_NATIVE, {.native = func}}, true, ""); \
        } else { \
            printf("Error: Native function '%s' not implemented!\n", name); \
        } \
    } while (0)

Value native_math_abs(int arg_count, Value* args) {
    if (arg_count < 1 || args[0].type != VAL_NUMBER) return (Value){VAL_NUMBER, {.number = 0}};
    return (Value){VAL_NUMBER, {.number = fabs(args[0].as.number)}};
}

Value native_math_sqrt(int arg_count, Value* args) {
    if (arg_count < 1 || args[0].type != VAL_NUMBER) return (Value){VAL_NUMBER, {.number = 0}};
    return (Value){VAL_NUMBER, {.number = sqrt(args[0].as.number)}};
}

void register_math_natives(Env* env){
    MATH_REGISTER(env,"__math_abs",native_math_abs);
    MATH_REGISTER(env,"__math_sqrt",native_math_sqrt);
}