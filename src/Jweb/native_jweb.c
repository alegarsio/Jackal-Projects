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
#include <pthread.h>
#include "eval.h"
#include <openssl/ssl.h>
#include <sys/stat.h>

int global_server_fd = -1;
long last_mtime = 0;

typedef struct {
    int client_socket;
    Value data;
    Value req_copy;
} AsyncResponseData;

#define JWEB_REGISTER(env, name, func)                                           \
    do                                                                           \
    {                                                                            \
        if (func != NULL)                                                        \
        {                                                                        \
            set_var(env, name, (Value){VAL_NATIVE, {.native = func}}, true, ""); \
        }                                                                        \
    } while (0)

const char* get_mime_type(const char* filename) {
    const char* dot = strrchr(filename, '.');
    if (!dot) return "application/octet-stream";
    if (strcmp(dot, ".html") == 0) return "text/html";
    if (strcmp(dot, ".css") == 0) return "text/css";
    if (strcmp(dot, ".js") == 0) return "application/javascript";
    if (strcmp(dot, ".json") == 0) return "application/json";
    if (strcmp(dot, ".png") == 0) return "image/png";
    if (strcmp(dot, ".jpg") == 0 || strcmp(dot, ".jpeg") == 0) return "image/jpeg";
    if (strcmp(dot, ".gif") == 0) return "image/gif";
    if (strcmp(dot, ".svg") == 0) return "image/svg+xml";
    return "text/plain";
}

static void parse_query_params(HashMap* map, char* query_string) {
    if (!query_string || strlen(query_string) == 0) return;
    char* saveptr1;
    char* pair = strtok_r(query_string, "&", &saveptr1);
    while (pair != NULL) {
        char* saveptr2;
        char* key = strtok_r(pair, "=", &saveptr2);
        char* val = strtok_r(NULL, "=", &saveptr2);
        if (key != NULL) {
            char* final_val = (val != NULL) ? strdup(val) : strdup("");
            map_set(map, strdup(key), (Value){VAL_STRING, {.string = final_val}});
        }
        pair = strtok_r(NULL, "&", &saveptr1);
    }
}

void* async_send_thread(void* arg) {
    AsyncResponseData* async_data = (AsyncResponseData*)arg;
    Value data_to_encode = async_data->data;
    int client_socket = async_data->client_socket;

    Value json_str_val = builtin_json_encode(1, &data_to_encode);

    if (json_str_val.type == VAL_STRING) {
        char* json_body = json_str_val.as.string;
        size_t body_len = strlen(json_body);
        
        char header[512];
        int header_len = snprintf(header, sizeof(header), 
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: application/json\r\n"
            "Content-Length: %zu\r\n"
            "Connection: close\r\n\r\n", body_len);

        if (header_len > 0) {
            send(client_socket, header, header_len, 0);
            send(client_socket, json_body, body_len, 0);
        }
    }

    close(client_socket);
    free(async_data);
    return NULL;
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

    char buffer[8192] = {0};
    ssize_t bytes_read = read(client_socket, buffer, sizeof(buffer) - 1);

    if (bytes_read <= 0) {
        close(client_socket);
        return (Value){VAL_NIL};
    }

    char method[16] = {0};
    char full_path[1024] = {0};
    
    if (sscanf(buffer, "%15s %1023s", method, full_path) < 2) {
        close(client_socket);
        return (Value){VAL_NIL};
    }

    HashMap* headers_map = map_new();
    char* line = strchr(buffer, '\n');
    if (line) {
        line++; 
        while (line && *line != '\r' && *line != '\n') {
            char* end_line = strchr(line, '\n');
            if (!end_line) break;
            char* colon = strchr(line, ':');
            if (colon && colon < end_line) {
                int key_len = colon - line;
                char* key = malloc(key_len + 1);
                strncpy(key, line, key_len);
                key[key_len] = '\0';
                char* val_start = colon + 1;
                while (*val_start == ' ') val_start++; 
                int val_len = end_line - val_start;
                if (val_len > 0 && *(end_line-1) == '\r') val_len--; 
                char* val = malloc(val_len + 1);
                strncpy(val, val_start, val_len);
                val[val_len] = '\0';
                map_set(headers_map, key, (Value){VAL_STRING, {.string = val}});
            }
            line = end_line + 1;
        }
    }
    
    HashMap* query_map = map_new();
    char *query_part = strchr(full_path, '?');
    if (query_part) {
        *query_part = '\0';
        char* query_copy = strdup(query_part + 1);
        parse_query_params(query_map, query_copy);
        free(query_copy);
    }

    HashMap* req_map = map_new();
    map_set(req_map, "method", (Value){VAL_STRING, {.string = strdup(method)}});
    map_set(req_map, "path", (Value){VAL_STRING, {.string = strdup(full_path)}});
    map_set(req_map, "query", (Value){VAL_MAP, {.map = query_map}});
    map_set(req_map, "headers", (Value){VAL_MAP, {.map = headers_map}}); 
    map_set(req_map, "socket_fd", (Value){VAL_NUMBER, {.number = (double)client_socket}});
    map_set(req_map, "address", (Value){VAL_STRING, {.string = strdup(inet_ntoa(client_addr.sin_addr))}});
    map_set(req_map, "params", (Value){VAL_MAP, {.map = map_new()}});

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

    AsyncResponseData* async_data = malloc(sizeof(AsyncResponseData));
    async_data->client_socket = client_socket;
    async_data->data = args[1];

    pthread_t thread;
    pthread_create(&thread, NULL, async_send_thread, async_data);
    pthread_detach(thread);

    return (Value){VAL_BOOL, {.boolean = true}};
}

Value native_middleware_auth(int arity, Value* args) {
    if (arity < 1 || args[0].type != VAL_MAP) return (Value){VAL_NIL};

    HashMap* req = args[0].as.map;
    Value headers_val;
    
    if (map_get(req, "headers", &headers_val) && headers_val.type == VAL_MAP) {
        Value auth_token;
        if (map_get(headers_val.as.map, "Authorization", &auth_token)) {
            if (auth_token.type == VAL_STRING && strcmp(auth_token.as.string, "Secret-Jackal-Key") == 0) {
                return (Value){VAL_NIL};
            }
        }
    }

    HashMap* error_res = map_new();
    map_set(error_res, "error", (Value){VAL_STRING, {.string = strdup("Unauthorized")}});
    map_set(error_res, "status", (Value){VAL_NUMBER, {.number = 401}});
    
    return (Value){VAL_MAP, {.map = error_res}};
}

Value native_web_redirect(int arity, Value* args) {
    if (arity < 2 || args[1].type != VAL_STRING) return (Value){VAL_NIL};
    int client_socket;
    if (args[0].type == VAL_MAP) {
        Value socket_val;
        if (!map_get(args[0].as.map, "socket_fd", &socket_val)) return (Value){VAL_NIL};
        client_socket = (int)socket_val.as.number;
    } else {
        client_socket = (int)args[0].as.number;
    }
    char* target_url = args[1].as.string;
    char response[1024];
    int len = sprintf(response, "HTTP/1.1 302 Found\r\nLocation: %s\r\nContent-Length: 0\r\nConnection: close\r\n\r\n", target_url);
    send(client_socket, response, len, 0);
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
            pt++; const char* start_k = pt;
            while (*pt != '}' && *pt != '\0') pt++;
            char key[64] = {0}; strncpy(key, start_k, pt - start_k);
            if (*pt == '}') pt++;
            const char* start_v = p;
            while (*p != '/' && *p != '\0') p++;
            char val[256] = {0}; strncpy(val, start_v, p - start_v);
            if (params) map_set(params, strdup(key), (Value){VAL_STRING, {.string = strdup(val)}});
        } else {
            if (*p != *pt) return (Value){VAL_BOOL, {.boolean = 0}};
            p++; pt++;
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
                char placeholder[128]; sprintf(placeholder, "{{%s}}", entry->key);
                char *val_str = value_to_string(entry->value);
                char *ins, *tmp, *result;
                int len_rep = strlen(placeholder), len_with = strlen(val_str), count = 0;
                ins = content;
                while ((tmp = strstr(ins, placeholder))) { count++; ins = tmp + len_rep; }
                result = malloc(strlen(content) + (len_with - len_rep) * count + 1);
                tmp = result; char* src = content;
                while (count--) {
                    ins = strstr(src, placeholder);
                    int len_front = ins - src; memcpy(tmp, src, len_front);
                    tmp += len_front; memcpy(tmp, val_str, len_with);
                    tmp += len_with; src = ins + len_rep;
                }
                strcpy(tmp, src); free(content); content = result; free(val_str);
            }
        }
    }
    return (Value){VAL_STRING, {.string = content}};
}

Value native_web_send_docs(int arity, Value* args) {
    if (arity < 1) return (Value){VAL_NIL};
    
    int client_socket;
    if (args[0].type == VAL_MAP) {
        Value socket_val;
        if (!map_get(args[0].as.map, "socket_fd", &socket_val)) return (Value){VAL_NIL};
        client_socket = (int)socket_val.as.number;
    } else {
        client_socket = (int)args[0].as.number;
    }

    const char* html = 
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: text/html\r\n"
        "Connection: close\r\n\r\n"
        "<!DOCTYPE html><html lang='en'><head><meta charset='utf-8' />"
        "<title>API Docs</title>"
        "<link rel='stylesheet' href='https://unpkg.com/swagger-ui-dist@5/swagger-ui.css' />"
        "</head><body><div id='swagger-ui'></div>"
        "<script src='https://unpkg.com/swagger-ui-dist@5/swagger-ui-bundle.js'></script>"
        "<script>window.onload=()=>{window.ui=SwaggerUIBundle({url:'/swagger.json',dom_id:'#swagger-ui'});};</script>"
        "</body></html>";

    send(client_socket, html, strlen(html), 0);
    close(client_socket);
    return (Value){VAL_BOOL, {.boolean = true}};
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
Value native_gateway_forward(int arity, Value* args) {
    if (arity < 3 || args[0].type != VAL_MAP || args[1].type != VAL_STRING || args[2].type != VAL_NUMBER) {
        return (Value){VAL_BOOL, {.boolean = false}};
    }

    Value socket_val;
    if (!map_get(args[0].as.map, "socket_fd", &socket_val)) return (Value){VAL_BOOL, {.boolean = false}};
    int client_socket = (int)socket_val.as.number;

    char* target_host = args[1].as.string;
    int target_port = (int)args[2].as.number;

    int service_socket = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in service_addr;
    service_addr.sin_family = AF_INET;
    service_addr.sin_port = htons(target_port);
    service_addr.sin_addr.s_addr = inet_addr(target_host);

    if (connect(service_socket, (struct sockaddr *)&service_addr, sizeof(service_addr)) < 0) {
        close(service_socket);
        return (Value){VAL_BOOL, {.boolean = false}};
    }

    Value v_method, v_path;
    map_get(args[0].as.map, "method", &v_method);
    map_get(args[0].as.map, "path", &v_path);

    char req_line[2048];
    int req_len = sprintf(req_line, "%s %s HTTP/1.1\r\nHost: %s\r\nConnection: close\r\n\r\n", 
        v_method.as.string, 
        v_path.as.string, 
        target_host);
    
    send(service_socket, req_line, req_len, 0);

    char buffer[8192];
    ssize_t n;
    while ((n = recv(service_socket, buffer, sizeof(buffer), 0)) > 0) {
        send(client_socket, buffer, n, 0);
    }

    close(service_socket);
    close(client_socket);

    return (Value){VAL_BOOL, {.boolean = true}};
}

Value native_check_file_change(int arity, Value* args) {
    if (arity < 1 || args[0].type != VAL_STRING) {
        return (Value){VAL_BOOL, {.boolean = false}};
    }

    char* file_path = args[0].as.string;
    struct stat attr;

    if (stat(file_path, &attr) == 0) {
        if (last_mtime == 0) {
            last_mtime = attr.st_mtime;
            return (Value){VAL_BOOL, {.boolean = false}};
        }

        if (attr.st_mtime > last_mtime) {
            last_mtime = attr.st_mtime;
            return (Value){VAL_BOOL, {.boolean = true}};
        }
    }

    return (Value){VAL_BOOL, {.boolean = false}};
}

void register_jweb_natives(Env *env){
    JWEB_REGISTER(env, "__listen__", native_web_listen);
    JWEB_REGISTER(env, "__accept__", native_web_poll);
    JWEB_REGISTER(env, "__send_json__", native_web_send_response);
    JWEB_REGISTER(env, "__match_route__", native_match_route);
    JWEB_REGISTER(env, "__render_file__", native_render_file);
    JWEB_REGISTER(env, "__send_html__", native_send_html);
    JWEB_REGISTER(env, "__redirect__", native_web_redirect);
    JWEB_REGISTER(env, "__native_auth__", native_middleware_auth);
    JWEB_REGISTER(env,"__send_docs__",native_web_send_docs);
    JWEB_REGISTER(env,"__native_forward__",native_gateway_forward);
    JWEB_REGISTER(env,"__check_file_change__",native_check_file_change);
}