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

char* net_resolve_host(const char* host) {
    struct hostent *he;
    struct in_addr **addr_list;

    if ((he = gethostbyname(host)) == NULL) {
        return NULL;
    }

    addr_list = (struct in_addr **)he->h_addr_list;
    if (addr_list[0] != NULL) {
        return inet_ntoa(*addr_list[0]);
    }

    return NULL;
}

Value native_net_htons(int arg_count, Value* args) {
    if (arg_count < 1 || args[0].type != VAL_NUMBER) return (Value){VAL_NUMBER, {.number = 0}};
    
    uint16_t port = (uint16_t)args[0].as.number;
    return (Value){VAL_NUMBER, {.number = (double)htons(port)}};
}

Value native_net_aton(int arg_count, Value* args) {
    if (arg_count < 1 || args[0].type != VAL_STRING) return (Value){VAL_NIL, {0}};
    
    const char* ip_str = args[0].as.string;
    struct in_addr addr;
    
    if (inet_aton(ip_str, &addr) == 0) return (Value){VAL_NIL, {0}};
    
    return (Value){VAL_NUMBER, {.number = (double)addr.s_addr}};
}
Value native_net_ntoa(int arg_count, Value* args) {
    if (arg_count < 1 || args[0].type != VAL_NUMBER) return (Value){VAL_NIL, {0}};
    
    struct in_addr addr;
    addr.s_addr = (uint32_t)args[0].as.number;
    
    char* ip_str = inet_ntoa(addr);
    return (Value){VAL_STRING, {.string = strdup(ip_str)}};
}

Value native_socket_get_local_ip(int arg_count, Value* args) {
    char hostname[256];
    if (gethostname(hostname, sizeof(hostname)) == -1) {
        return (Value){VAL_NIL, {0}};
    }

    struct hostent *he = gethostbyname(hostname);
    if (he == NULL) {
        return (Value){VAL_NIL, {0}};
    }

    struct in_addr **addr_list = (struct in_addr **)he->h_addr_list;
    if (addr_list[0] != NULL) {
        char* ip = inet_ntoa(*addr_list[0]);
        return (Value){VAL_STRING, {.string = strdup(ip)}};
    }

    return (Value){VAL_NIL, {0}};
}

Value native_socket_resolve(int arg_count, Value* args) {
    if (arg_count < 1 || args[0].type != VAL_STRING) {
        return (Value){VAL_NIL, {0}};
    }

    const char* host = args[0].as.string;
    char* ip = net_resolve_host(host);

    if (ip == NULL) {
        return (Value){VAL_NIL, {0}};
    }

    char* res_str = strdup(ip);
    return (Value){VAL_STRING, {.string = res_str}};
}

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


Value native_socket_send(int arg_count, Value* args) {
    if (arg_count < 2 || args[0].type != VAL_NUMBER || args[1].type != VAL_STRING) 
        return (Value){VAL_NUMBER, {.number = -1}};

    socket_t s = (socket_t)args[0].as.number;
    const char* data = args[1].as.string;
    
    int sent = send(s, data, (int)strlen(data), 0);
    return (Value){VAL_NUMBER, {.number = (double)sent}};
}

Value native_socket_recv(int arg_count, Value* args) {
    if (arg_count < 2 || args[0].type != VAL_NUMBER || args[1].type != VAL_NUMBER)
        return (Value){VAL_NIL, {0}};

    socket_t s = (socket_t)args[0].as.number;
    int buffer_size = (int)args[1].as.number;
    
    char* buffer = malloc(buffer_size + 1);
    int bytes_received = recv(s, buffer, buffer_size, 0);

    if (bytes_received <= 0) {
        free(buffer);
        return (Value){VAL_NIL, {0}};
    }

    buffer[bytes_received] = '\0';
    Value res = (Value){VAL_STRING, {.string = buffer}};
    return res;
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

Value native_socket_connect(int arg_count, Value* args) {
    if (arg_count < 3) return (Value){VAL_NUMBER, {.number = -1}};
    socket_t s = (socket_t)args[0].as.number;
    const char* host = args[1].as.string;
    int port = (int)args[2].as.number;
    int res = net_socket_connect(s, host, port);
    return (Value){VAL_NUMBER, {.number = (double)res}};
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
    SOCKET_REGISTER(env, "socket_send", native_socket_send);
    SOCKET_REGISTER(env, "socket_recv", native_socket_recv);
    SOCKET_REGISTER(env, "socket_resolve", native_socket_resolve);
    SOCKET_REGISTER(env, "socket_connect", native_socket_connect);
    SOCKET_REGISTER(env, "__net_htons", native_net_htons);
    SOCKET_REGISTER(env, "__net_aton", native_net_aton);
    SOCKET_REGISTER(env, "__net_ntoa", native_net_ntoa);
    SOCKET_REGISTER(env, "socket_get_local_ip", native_socket_get_local_ip);
}