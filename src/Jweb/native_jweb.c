#include "Jweb/native_jweb.h"
#include "json/native_json.h"
#include <stdio.h>      
#include <stdlib.h>     
#include <string.h>     
#include <unistd.h>     
#include <sys/socket.h>  
#include <netinet/in.h>  
#include <arpa/inet.h>   
#include <fcntl.h>       
#include "eval.h"
#include "value.h"
int global_server_fd = -1;

#define JWEB_REGISTER(env, name, func)                                           \
    do                                                                           \
    {                                                                            \
        if (func != NULL)                                                        \
        {                                                                        \
            set_var(env, name, (Value){VAL_NATIVE, {.native = func}}, true, ""); \
        }                                                                        \
    } while (0)

// Helper untuk parsing query string: ?id=123&name=jackal
static void parse_query_params(HashMap* map, char* query_string) {
    if (!query_string) return;
    char* saveptr;
    char* token = strtok_r(query_string, "&", &saveptr);
    while (token != NULL) {
        char* eq = strchr(token, '=');
        if (eq) {
            *eq = '\0';
            char* key = token;
            char* val = eq + 1;
            map_set(map, strdup(key), (Value){VAL_STRING, {.string = strdup(val)}});
        }
        token = strtok_r(NULL, "&", &saveptr);
    }
}

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
    char buffer[8192] = {0};
    read(client_socket, buffer, sizeof(buffer) - 1);

    char method[16], full_path[1024];
    sscanf(buffer, "%15s %1023s", method, full_path);

    // --- PERUBAHAN DISINI: Parsing Query String ---
    HashMap* query_map = map_new();
    char *query_part = strchr(full_path, '?');
    if (query_part) {
        *query_part = '\0'; // Memisahkan path murni dengan query string
        char* query_copy = strdup(query_part + 1);
        parse_query_params(query_map, query_copy);
        free(query_copy);
    }
    // ----------------------------------------------

    HashMap* req_map = map_new();
    map_set(req_map, "method", (Value){VAL_STRING, {.string = strdup(method)}});
    map_set(req_map, "path", (Value){VAL_STRING, {.string = strdup(full_path)}});
    map_set(req_map, "query", (Value){VAL_MAP, {.map = query_map}}); // Masukkan query map
    map_set(req_map, "socket_fd", (Value){VAL_NUMBER, {.number = (double)client_socket}});
    map_set(req_map, "address", (Value){VAL_STRING, {.string = strdup(ip_address)}});
    map_set(req_map, "params", (Value){VAL_MAP, {.map = map_new()}}); // Untuk path params {id}

    char *body_start = strstr(buffer, "\r\n\r\n");
    if (body_start) {
        body_start += 4;
        Value json_arg = (Value){VAL_STRING, {.string = body_start}};
        Value body_map = native_json_parse(1, &json_arg); 
        map_set(req_map, "body", body_map);
    } else {
        map_set(req_map, "body", (Value){VAL_NIL});
    }
    
    return (Value){VAL_MAP, {.map = req_map}};
}

Value native_web_send_response(int arity, Value* args) {
    if (arity < 2) return (Value){VAL_NIL};

    Value socket_val;
    if (args[0].type == VAL_MAP) {
        if (!map_get(args[0].as.map, "socket_fd", &socket_val)) return (Value){VAL_NIL};
    } else {
        socket_val = args[0];
    }
    int client_socket = (int)socket_val.as.number;

    Value data_to_encode = args[1];
    Value result_from_func = {VAL_NIL};

    if (data_to_encode.type == VAL_FUNCTION || data_to_encode.type == VAL_NATIVE) {
        Value callback_args[1] = { args[0] }; 
        result_from_func = call_jackal_function(NULL, data_to_encode, 1, callback_args);
        data_to_encode = result_from_func;
    }

    Value json_str_val = builtin_json_encode(1, &data_to_encode);

    if (json_str_val.type == VAL_STRING) {
        char* json_body = json_str_val.as.string;
        char response[8192];
        int response_len = sprintf(response, 
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: application/json\r\n"
            "Content-Length: %zu\r\n"
            "Connection: close\r\n\r\n"
            "%s", strlen(json_body), json_body);

        send(client_socket, response, response_len, 0);
    }

    if (result_from_func.type != VAL_NIL) {
        free_value(result_from_func);
    }
    
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

    if (arity == 3) {
        Value existing_params;
        if (map_get(args[0].as.map, "params", &existing_params) && existing_params.type == VAL_MAP) {
            params = existing_params.as.map;
        } else {
            params = map_new();
            map_set(args[0].as.map, "params", (Value){VAL_MAP, {.map = params}});
        }
    }

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

    return (Value){VAL_BOOL, {.boolean = (*p == '\0' && *pt == '\0')}};
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
    if (arity < 1 || args[0].type != VAL_STRING) return (Value){VAL_NIL};
    char *content = read_file_to_string(args[0].as.string);
    if (!content) return (Value){VAL_NIL};

    if (arity == 2 && args[1].type == VAL_MAP) {
        HashMap *data = args[1].as.map;
        for (int i = 0; i < data->capacity; i++) {
            Entry *entry = &data->entries[i];
            if (entry->key != NULL) {
                char placeholder[128];
                sprintf(placeholder, "{{%s}}", entry->key);
                char *val_str = value_to_string(entry->value);
                char *ins, *tmp, *result;
                int len_rep = strlen(placeholder), len_with = strlen(val_str), count = 0;

                ins = content;
                while ((tmp = strstr(ins, placeholder))) { count++; ins = tmp + len_rep; }
                
                result = malloc(strlen(content) + (len_with - len_rep) * count + 1);
                tmp = result;
                char* src = content;
                while (count--) {
                    ins = strstr(src, placeholder);
                    int len_front = ins - src;
                    memcpy(tmp, src, len_front);
                    tmp += len_front;
                    memcpy(tmp, val_str, len_with);
                    tmp += len_with;
                    src = ins + len_rep;
                }
                strcpy(tmp, src);
                free(content);
                content = result;
                free(val_str);
            }
        }
    }
    return (Value){VAL_STRING, {.string = content}};
}

Value native_send_html(int arity, Value *args) {
    if (arity != 2 || args[1].type != VAL_STRING) return (Value){VAL_NIL};
    int client_socket = (int)args[0].as.number;
    const char *html_content = args[1].as.string;
    char header[512];
    int header_len = sprintf(header, "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: %zu\r\nConnection: close\r\n\r\n", strlen(html_content));
    send(client_socket, header, header_len, 0);
    send(client_socket, html_content, strlen(html_content), 0);
    return (Value){VAL_BOOL, {.boolean = true}};
}

void register_jweb_natives(Env *env){
    JWEB_REGISTER(env, "__listen__", native_web_listen);
    JWEB_REGISTER(env, "__accept__", native_web_poll);
    JWEB_REGISTER(env, "__send_json__", native_web_send_response);
    JWEB_REGISTER(env, "__match_route__", native_match_route);
    JWEB_REGISTER(env, "__render_file__", native_render_file);
    JWEB_REGISTER(env, "__send_html__", native_send_html);
}