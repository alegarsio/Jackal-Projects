#include "socket/socket_native.h"
#include "socket/net_utils.h"
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#define SOCKET_REGISTER(env, name, func) \
    do { \
        if (func != NULL) { \
            set_var(env, name, (Value){VAL_NATIVE, {.native = func}}, true, ""); \
        } else { \
            printf("Socket Error: Native function '%s' not implemented!\n", name); \
        } \
    } while (0)

Value native_socket_create(int arg_count, Value* args) {
    if (arg_count < 2) return (Value){VAL_NIL, {0}};
    
    socket_t s = net_socket_create((int)args[0].as.number, (int)args[1].as.number, 0);
    return (Value){VAL_NUMBER, {.number = (double)s}};
}

Value native_socket_bind(int arg_count, Value* args) {
    if (arg_count < 3) return (Value){VAL_NUMBER, {.number = -1}};
    
    socket_t s = (socket_t)args[0].as.number;
    const char* ip = args[1].as.string;
    int port = (int)args[2].as.number;
    
    int result = net_socket_bind(s, ip, port);
    
    if (result != 0) {
        printf("Socket Error: Bind failed with code: %d\n", net_get_last_error());
    }
    
    return (Value){VAL_NUMBER, {.number = (double)result}};
}

Value native_socket_listen(int arg_count, Value* args) {
    if (arg_count < 2) return (Value){VAL_NUMBER, {.number = -1}};
    
    socket_t s = (socket_t)args[0].as.number;
    int backlog = (int)args[1].as.number;
    
    int result = listen(s, backlog);
    return (Value){VAL_NUMBER, {.number = (double)result}};
}

Value native_socket_close(int arg_count, Value* args) {
    if (arg_count < 1) return (Value){VAL_NIL, {0}};
    socket_t s = (socket_t)args[0].as.number;
    CLOSE_SOCKET(s);
    return (Value){VAL_NIL, {0}};
}

void register_socket_natives(Env* env) {

    set_var(env, "AF_INET", (Value){VAL_NUMBER, {.number = 2}}, true, "");
    set_var(env, "SOCK_STREAM", (Value){VAL_NUMBER, {.number = 1}}, true, "");
    set_var(env, "SOCK_DGRAM", (Value){VAL_NUMBER, {.number = 2}}, true, "");
    set_var(env, "SOL_SOCKET", (Value){VAL_NUMBER, {.number = 0xFFFF}}, true, "");

    SOCKET_REGISTER(env, "socket_create", native_socket_create);
    SOCKET_REGISTER(env, "socket_bind", native_socket_bind);
    SOCKET_REGISTER(env, "socket_listen", native_socket_listen);
    SOCKET_REGISTER(env, "socket_close", native_socket_close);
}