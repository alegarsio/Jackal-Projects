#include"System/system_native.h"
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#define SYS_REGISTER(env, name, func) \
    do { \
        if (func != NULL) { \
            set_var(env, name, (Value){VAL_NATIVE, {.native = func}}, true, ""); \
        } else { \
            printf("Error: Native function '%s' not implemented!\n", name); \
        } \
    } while (0)

Value native_system_platform(int arg_count, Value* args) {
    #ifdef _WIN32
        return (Value){VAL_STRING, {.string = strdup("windows")}};
    #elif __APPLE__
        #include <TargetConditionals.h>
        #if TARGET_OS_IPHONE
            return (Value){VAL_STRING, {.string = strdup("ios")}};
        #else
            return (Value){VAL_STRING, {.string = strdup("macos")}};
        #endif
    #elif __linux__
        return (Value){VAL_STRING, {.string = strdup("linux")}};
    #elif __unix__
        return (Value){VAL_STRING, {.string = strdup("unix")}};
    #else
        return (Value){VAL_STRING, {.string = strdup("unknown")}};
    #endif
}

Value native_system_exec(int arg_count, Value* args) {
    if (arg_count < 1 || args[0].type != VAL_STRING) {
        return (Value){VAL_NUMBER, {.number = -1}};
    }
    
    int result = system(args[0].as.string);
    return (Value){VAL_NUMBER, {.number = (double)result}};
}


void register_sys_natives(Env* env){
    SYS_REGISTER(env,"__sys_run",native_system_exec);
    SYS_REGISTER(env,"__sys_platform",native_system_platform);
}