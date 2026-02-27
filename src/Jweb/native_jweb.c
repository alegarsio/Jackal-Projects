#include "Jweb/native_jweb.h"
#include <stdio.h>      
#include <stdlib.h>     
#include <string.h>     
#include <unistd.h>     
#include <sys/socket.h>  
#include <netinet/in.h>  
#include <arpa/inet.h>   
#include <fcntl.h>       

int global_server_fd = -1;

#define JWEB_REGISTER(env, name, func)                                           \
    do                                                                           \
    {                                                                            \
        if (func != NULL)                                                        \
        {                                                                        \
            set_var(env, name, (Value){VAL_NATIVE, {.native = func}}, true, ""); \
        }                                                                        \
    } while (0)

Value native_web_listen(int arity, Value* args) {
    if (arity < 1 || args[0].type != VAL_NUMBER) return (Value){VAL_NIL};
    
    int port = (int)args[0].as.number;
    struct sockaddr_in address;
    int opt = 1;

    global_server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (global_server_fd == -1) return (Value){VAL_BOOL, {.boolean = false}};

    setsockopt(global_server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    if (bind(global_server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        close(global_server_fd);
        return (Value){VAL_BOOL, {.boolean = false}};
    }
    
    listen(global_server_fd, 10);
    return (Value){VAL_BOOL, {.boolean = true}};
}

Value native_web_poll(int arity, Value* args) {
    if (global_server_fd == -1) return (Value){VAL_NIL};

    struct sockaddr_in client_addr;
    socklen_t addrlen = sizeof(client_addr);
    int client_socket = accept(global_server_fd, (struct sockaddr *)&client_addr, &addrlen);

    if (client_socket < 0) return (Value){VAL_NIL};

    char* ip_address = inet_ntoa(client_addr.sin_addr);

    char buffer[4096] = {0};
    read(client_socket, buffer, sizeof(buffer) - 1);

    char method[16], full_path[1024];
    if (sscanf(buffer, "%15s %1023s", method, full_path) < 2) {
        close(client_socket);
        return (Value){VAL_NIL};
    }

    char *query = strchr(full_path, '?');
    if (query) { *query = '\0'; }

    HashMap* req_map = map_new();
    map_set(req_map, "method", (Value){VAL_STRING, {.string = strdup(method)}});
    map_set(req_map, "path", (Value){VAL_STRING, {.string = strdup(full_path)}});
    map_set(req_map, "socket_fd", (Value){VAL_NUMBER, {.number = (double)client_socket}});
    map_set(req_map, "address", (Value){VAL_STRING, {.string = strdup(ip_address)}});
    
    return (Value){VAL_MAP, {.map = req_map}};
}
Value native_web_send_response(int arity, Value* args) {
    if (arity < 2 || args[0].type != VAL_MAP) return (Value){VAL_NIL};

    Value socket_val;
    if (!map_get(args[0].as.map, "socket_fd", &socket_val)) return (Value){VAL_NIL};
    int client_socket = (int)socket_val.as.number;
    
    Value json_str_val = builtin_json_encode(1, &args[1]);
    
    if (json_str_val.type != VAL_STRING) {
        char* err = "{\"error\": \"JSON Encode Failed\"}";
        send(client_socket, err, strlen(err), 0);
        close(client_socket);
        return (Value){VAL_BOOL, {.boolean = false}};
    }
    
    char* json_body = json_str_val.as.string;
    char response[8192];
    int response_len = sprintf(response, 
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: application/json\r\n"
        "Content-Length: %zu\r\n"
        "Connection: close\r\n\r\n"
        "%s", strlen(json_body), json_body);

    send(client_socket, response, response_len, 0);
    close(client_socket);
    
    return (Value){VAL_BOOL, {.boolean = true}};
}
Value native_match_route(int arity, Value *args) {
  
    if (arity < 2) return (Value){VAL_BOOL, {.boolean = 0}};

    const char* path = (arity == 3) ? args[1].as.string : args[0].as.string;
    const char* pattern = (arity == 3) ? args[2].as.string : args[1].as.string;

    if (strcmp(path, pattern) == 0) return (Value){VAL_BOOL, {.boolean = 1}};

    const char* p = path;
    const char* pt = pattern;
    HashMap* params = NULL;

    if (arity == 3) params = map_new(); 

    while (*p != '\0' && *pt != '\0') {
        if (*pt == '{') {
            pt++; 
            const char* start_k = pt;
            while (*pt != '}' && *pt != '\0') pt++;
            char key[64] = {0};
            strncpy(key, start_k, pt - start_k);
            if (*pt == '}') pt++;

            const char* start_v = p;
            while (*p != '/' && *p != '\0') p++;
            char val[256] = {0};
            strncpy(val, start_v, p - start_v);

            if (params) {
                map_set(params, strdup(key), (Value){VAL_STRING, {.string = strdup(val)}});
            }
        } else {
            if (*p != *pt) return (Value){VAL_BOOL, {.boolean = 0}};
            p++;
            pt++;
        }
    }

    bool matched = (*p == '\0' && *pt == '\0');

    if (matched && arity == 3) {
        map_set(args[0].as.map, "params", (Value){VAL_MAP, {.map = params}});
    }

    return (Value){VAL_BOOL, {.boolean = matched}};
}



char* read_file_to_string(const char* filename) {
    FILE* f = fopen(filename, "rb");
    if (f == NULL) return NULL;

    fseek(f, 0, SEEK_END);
    long fsize = ftell(f);
    fseek(f, 0, SEEK_SET);

    char* string = malloc(fsize + 1);
    fread(string, fsize, 1, f);
    fclose(f);

    string[fsize] = 0;
    return string;
}

Value native_render_file(int arity, Value *args) {
    if (arity != 1 || args[0].type != VAL_STRING) {
        print_error("render_file expects (string path)");
        return (Value){VAL_NIL, {0}};
    }

    const char *path = args[0].as.string;
    FILE *file = fopen(path, "rb");
    if (!file) {
        print_error("Could not open file: %s", path);
        return (Value){VAL_NIL, {0}};
    }

    fseek(file, 0, SEEK_END);
    long fsize = ftell(file);
    fseek(file, 0, SEEK_SET);

    char *content = malloc(fsize + 1);
    fread(content, fsize, 1, file);
    fclose(file);
    content[fsize] = 0;

    Value result = (Value){VAL_STRING, {.string = content}};
    return result;
}
Value native_send_html(int arity, Value *args) {
    if (arity != 2 || args[1].type != VAL_STRING) {
        print_error("send_html expects (int socket, string content)");
        return (Value){VAL_NIL, {0}};
    }

    int client_socket = (int)args[0].as.number;
    const char *html_content = args[1].as.string;

    char header[512];
    int header_len = sprintf(header,
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: text/html\r\n"
        "Content-Length: %zu\r\n"
        "Connection: close\r\n"
        "\r\n",
        strlen(html_content));

    send(client_socket, header, header_len, 0);
    send(client_socket, html_content, strlen(html_content), 0);

    return (Value){VAL_BOOL, {.boolean = true}};
}
void register_jweb_natives(Env *env){
    JWEB_REGISTER(env, "__listen__", native_web_listen);
    JWEB_REGISTER(env, "__accept__", native_web_poll);
    JWEB_REGISTER(env, "__send_json__", native_web_send_response);
    JWEB_REGISTER(env, "__match_route__", native_match_route);
    JWEB_REGISTER(env,"__render_file__",native_render_file);
    JWEB_REGISTER(env,"__send_html__",native_send_html);
}