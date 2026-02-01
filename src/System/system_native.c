#include"System/system_native.h"
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#define UNKNOWN 0
#define WINDOWS 1
#define MACOS   2
#define LINUX   3
#define UNIX    4

#ifdef _WIN32
    #include <windows.h>
#else
    #include <unistd.h>
    #include <sys/time.h>
#endif

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

Value native_system_platform_id(int arg_count, Value* args) {
    #ifdef _WIN32
        return (Value){VAL_NUMBER, {.number = WINDOWS}};
    #elif __APPLE__
        return (Value){VAL_NUMBER, {.number = MACOS}};
    #elif __linux__
        return (Value){VAL_NUMBER, {.number = LINUX}};
    #elif __unix__
        return (Value){VAL_NUMBER, {.number = UNIX}};
    #else
        return (Value){VAL_NUMBER, {.number = UNKNOWN}};
    #endif
}
Value native_system_exec(int arg_count, Value* args) {
    if (arg_count < 1 || args[0].type != VAL_STRING) {
        return (Value){VAL_NUMBER, {.number = -1}};
    }
    
    int result = system(args[0].as.string);
    return (Value){VAL_NUMBER, {.number = (double)result}};
}

Value native_sys_sleep(int arg_count, Value* args) {
    if (arg_count < 1 || args[0].type != VAL_NUMBER) return (Value){VAL_NIL, {0}};
    int ms = (int)args[0].as.number;
#ifdef _WIN32
    Sleep(ms);
#else
    usleep(ms * 1000);
#endif
    return (Value){VAL_NIL, {0}};
}

Value native_sys_now(int arg_count, Value* args) {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    double ms = (double)(tv.tv_sec) * 1000 + (double)(tv.tv_usec) / 1000;
    return (Value){VAL_NUMBER, {.number = ms}};
}

Value native_system_getenv(int arg_count, Value* args) {
    if (arg_count < 1 || args[0].type != VAL_STRING) {
        return (Value){VAL_NIL, {0}, NULL};
    }
    
    char* val = getenv(args[0].as.string);
    if (val == NULL) return (Value){VAL_NIL, {0}, NULL};
    
    return (Value){VAL_STRING, {.string = strdup(val)}, NULL};
}

void register_sys_natives(Env* env){

    set_var(env, "WINDOWS", (Value){VAL_NUMBER, {.number = WINDOWS}}, true, "");
    set_var(env, "MACOS",   (Value){VAL_NUMBER, {.number = MACOS}}, true, "");
    set_var(env, "LINUX",   (Value){VAL_NUMBER, {.number = LINUX}}, true, "");
    set_var(env, "UNIX",    (Value){VAL_NUMBER, {.number = UNIX}}, true, "");

    SYS_REGISTER(env,"__sys_run",native_system_exec);
    SYS_REGISTER(env,"__sys_sleep",native_sys_sleep);
    SYS_REGISTER(env,"__sys_platform",native_system_platform);
    SYS_REGISTER(env,"__sys_now",native_sys_now);
    SYS_REGISTER(env, "__sys_getenv", native_system_getenv);
}