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
#include "value.h"
#include "common.h"
#include <openssl/ssl.h>
#include <sys/stat.h>

int global_server_fd = -1;
long last_mtime = 0;

typedef struct {
    int client_socket;
    Value data;
    Value req_copy;
} AsyncResponseData;

extern Env* global_env;

#define JWEB_REGISTER(env, name, func)                                           \
    do                                                                           \
    {                                                                            \
        if (func != NULL)                                                        \
        {                                                                        \
            set_var(env, name, (Value){VAL_NATIVE, {.native = func}}, true, ""); \
        }                                                                        \
    } while (0)

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
    if (strcmp(dot, ".csv") == 0) return "text/csv";
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
Value deserialize_to_jackal(char* buffer, int length) {
    if (length <= 0) return (Value){VAL_NIL};

    if (buffer[0] == '{' || buffer[0] == '[') {
        Value json_arg = (Value){VAL_STRING, {.string = buffer}};
        
        return native_json_parse(1, &json_arg);
    } 

    char* endptr;
    double num = strtod(buffer, &endptr);
    if (endptr != buffer) {
        Value result;
        result.type = VAL_NUMBER;
        result.as.number = num;
        result.gc_info = NULL;
        return result;
    }

    return (Value){VAL_NIL};
}

Value native_jk_remote_call(int arity, Value* args) {
    if (arity < 3) return (Value){VAL_NIL};

    char* ip = AS_CSTRING(args[0]);
    int port = (int)args[1].as.number;
    char* func_name = AS_CSTRING(args[2]);

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) return (Value){VAL_NIL};

    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    
    if (inet_pton(AF_INET, ip, &serv_addr.sin_addr) <= 0) {
        close(sock);
        return (Value){VAL_NIL};
    }

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        close(sock);
        return (Value){VAL_NIL};
    }

    send(sock, func_name, strlen(func_name), 0);

    char* buffer = malloc(65536);
    int bytes_received = recv(sock, buffer, 65535, 0);
    
    close(sock);

    if (bytes_received <= 0) {
        free(buffer);
        return (Value){VAL_NIL};
    }

    buffer[bytes_received] = '\0';
    Value result = deserialize_to_jackal(buffer, bytes_received);
    
    free(buffer);
    return result;
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
char* clean_section_tags(char* content) {
    char *tags[] = {"@section", "@endsection", NULL};
    for (int i = 0; tags[i] != NULL; i++) {
        char *pos;
        while ((pos = strstr(content, tags[i]))) {
            char *line_end = strchr(pos, '\n');
            if (!line_end) line_end = pos + strlen(pos);
            
            int prefix_len = pos - content;
            int suffix_len = strlen(line_end);
            char *new_content = malloc(prefix_len + suffix_len + 1);
            
            memcpy(new_content, content, prefix_len);
            strcpy(new_content + prefix_len, line_end);
            
            free(content);
            content = new_content;
        }
    }
    return content;
}
char* apply_macro_data(char* blueprint, Value actual_data) {
    if (actual_data.type != VAL_MAP) return strdup(blueprint);
    
    char *result = strdup(blueprint);
    HashMap *map = actual_data.as.map;

    for (int i = 0; i < map->capacity; i++) {
        Entry *entry = &map->entries[i];
        if (entry->key == NULL) continue;

        char placeholder[128];
        snprintf(placeholder, sizeof(placeholder), "{{item.%s}}", entry->key);

        char *val_str = value_to_string(entry->value);
        if (strstr(result, placeholder)) {
            char *new_res = str_replace(result, placeholder, val_str);
            free(result);
            result = new_res;
        }
        free(val_str);
    }
    return result;
}

char* evaluate_template_sections(char* content, const char* target_section) {
    char section_tag[128];
    snprintf(section_tag, sizeof(section_tag), "@section(\"%s\")", target_section);

    char *start = strstr(content, section_tag);
    if (!start) return content; 

    char *block_start = strstr(start, ")") + 1;
    char *end = strstr(block_start, "@endsection");
    if (!end) return content;

    int len = end - block_start;
    char *section_content = strndup(block_start, len);
    
    free(content);
    return section_content;
}


char* render_sub_block(const char* template, const char* alias, Value item) {
    char *result = strdup(template);
    if (item.type != VAL_MAP) return result;

    HashMap *item_map = item.as.map;
    for (int i = 0; i < item_map->capacity; i++) {
        Entry *entry = &item_map->entries[i];
        if (entry->key == NULL) continue;

        char placeholder[128];
        snprintf(placeholder, sizeof(placeholder), "{{%s.%s}}", alias, entry->key);

        char *val_str = value_to_string(entry->value);
        char *pos;
        while ((pos = strstr(result, placeholder))) {
            int prefix_len = pos - result;
            int replace_len = strlen(placeholder);
            int val_len = strlen(val_str);
            
            char *new_res = malloc(strlen(result) - replace_len + val_len + 1);
            memcpy(new_res, result, prefix_len);
            memcpy(new_res + prefix_len, val_str, val_len);
            strcpy(new_res + prefix_len + val_len, pos + replace_len);
            
            free(result);
            result = new_res;
        }
        free(val_str);
    }
    return result;
}

char* evaluate_template_includes(char* content) {
    char *start;
    while ((start = strstr(content, "@include"))) {
        char *open_paren = strchr(start, '(');
        char *close_paren = strchr(start, ')');
        if (!open_paren || !close_paren || open_paren > close_paren) break;

        char filename[128];
        char *quote_start = strpbrk(open_paren, "\"'");
        char *quote_end = quote_start ? strpbrk(quote_start + 1, "\"'") : NULL;

        if (!quote_start || !quote_end || quote_end > close_paren) break;

        int name_len = quote_end - quote_start - 1;
        strncpy(filename, quote_start + 1, name_len);
        filename[name_len] = '\0';

        char *include_content = read_file_to_string(filename);
        if (!include_content) include_content = strdup("");

        int prefix_len = (int)(start - content);
        int include_len = (int)strlen(include_content);
        int suffix_len = (int)strlen(close_paren + 1);

        char *new_content = malloc(prefix_len + include_len + suffix_len + 1);
        if (!new_content) {
            free(include_content);
            break;
        }

        memcpy(new_content, content, prefix_len);
        memcpy(new_content + prefix_len, include_content, include_len);
        strcpy(new_content + prefix_len + include_len, close_paren + 1);

        free(include_content);
        free(content);
        content = new_content;
    }
    return content;
}

char* evaluate_template_loop(char* content, HashMap* data) {
    char *start;
    while ((start = strstr(content, "@for"))) {
        char *end = strstr(start, "@endfor");
        if (!end) break;

        char list_name[64], item_alias[64];
        char *open_p = strchr(start, '(');
        char *close_p = strchr(start, ')');
        if (!open_p || !close_p || open_p > close_p) break;

        char loop_inner[128];
        int inner_len = close_p - open_p - 1;
        strncpy(loop_inner, open_p + 1, inner_len);
        loop_inner[inner_len] = '\0';

        if (sscanf(loop_inner, "%s as %s", list_name, item_alias) != 2) break;

        Value list_val;
        if (!map_get(data, list_name, &list_val) || list_val.type != VAL_ARRAY) {
            int prefix_len = (int)(start - content);
            int suffix_len = (int)strlen(end + 11);
            char *new_content = malloc(prefix_len + suffix_len + 1);
            memcpy(new_content, content, prefix_len);
            strcpy(new_content + prefix_len, end + 11);
            free(content);
            content = new_content;
            continue;
        }

        char *block_start = close_p + 1;
        int block_len = (int)(end - block_start);
        char *template_block = strndup(block_start, block_len);

        char *repeated_html = strdup("");
        ValueArray *items = list_val.as.array;

        for (int i = 0; i < items->count; i++) {
            char *rendered_item = render_sub_block(template_block, item_alias, items->values[i]);
            char *temp = malloc(strlen(repeated_html) + strlen(rendered_item) + 1);
            strcpy(temp, repeated_html);
            strcat(temp, rendered_item);
            free(repeated_html);
            free(rendered_item);
            repeated_html = temp;
        }

        int prefix_len = (int)(start - content);
        int suffix_len = (int)strlen(end + 11);
        char *final_res = malloc(prefix_len + strlen(repeated_html) + suffix_len + 1);
        memcpy(final_res, content, prefix_len);
        memcpy(final_res + prefix_len, repeated_html, strlen(repeated_html));
        strcpy(final_res + prefix_len + strlen(repeated_html), end + 11);

        free(repeated_html);
        free(template_block);
        free(content);
        content = final_res;
    }
    return content;
}


char* evaluate_template_logic(char* content, HashMap* data) {
    char *start;
    while ((start = strstr(content, "@if"))) {
        char *end = strstr(start, "@endif");
        if (!end) break;

        char condition[128];
        if (sscanf(start, "@if(%[^)])", condition) != 1) {
            if (sscanf(start, "@if (%[^)])", condition) != 1) break;
        }

        Value val;
        bool is_true = false;
        if (map_get(data, condition, &val)) {
            if (val.type == VAL_BOOL) is_true = val.as.boolean;
            else if (val.type == VAL_STRING) is_true = strlen(val.as.string) > 0;
            else if (val.type == VAL_NUMBER) is_true = val.as.number != 0;
        }

        char *block_start = strstr(start, ")") + 1;
        char *else_p = strstr(block_start, "@else");
        if (else_p && else_p > end) else_p = NULL;

        char *selected_text = NULL;
        int selected_len = 0;

        if (is_true) {
            char *limit = else_p ? else_p : end;
            selected_len = limit - block_start;
            selected_text = malloc(selected_len + 1);
            memcpy(selected_text, block_start, selected_len);
            selected_text[selected_len] = '\0';
        } else if (else_p) {
            char *else_content_start = else_p + 5;
            selected_len = end - else_content_start;
            selected_text = malloc(selected_len + 1);
            memcpy(selected_text, else_content_start, selected_len);
            selected_text[selected_len] = '\0';
        } else {
            selected_text = strdup("");
            selected_len = 0;
        }

        int prefix_len = start - content;
        int suffix_len = strlen(end + 6);
        char *new_content = malloc(prefix_len + selected_len + suffix_len + 1);

        memcpy(new_content, content, prefix_len);
        memcpy(new_content + prefix_len, selected_text, selected_len);
        strcpy(new_content + prefix_len + selected_len, end + 6);

        free(selected_text);
        free(content);
        content = new_content;
    }
    return content;
}

Value native_render_file(int arity, Value *args) {
    if (arity < 1 || args[0].type != VAL_STRING) return (Value){VAL_NIL};

    char *content = read_file_to_string(args[0].as.string);
    if (!content) return (Value){VAL_NIL};

    content = evaluate_template_includes(content);

    if (arity == 3 && args[2].type == VAL_STRING) {
        content = evaluate_template_sections(content, args[2].as.string);
    }

    if (arity >= 2 && args[1].type == VAL_MAP) {
        HashMap *data = args[1].as.map;

        content = evaluate_template_loop(content, data);
        content = evaluate_template_logic(content, data);

        for (int i = 0; i < data->capacity; i++) {
            Entry *entry = &data->entries[i];
            if (entry->key == NULL) continue;

            char placeholder[256];
            snprintf(placeholder, sizeof(placeholder), "{{%s}}", entry->key);

            char *val_str = value_to_string(entry->value);
            char *tmp = strstr(content, placeholder);

            if (tmp) {
                int count = 0;
                char *ins = content;
                int len_rep = (int)strlen(placeholder);
                int len_with = (int)strlen(val_str);

                while ((tmp = strstr(ins, placeholder))) {
                    count++;
                    ins = tmp + len_rep;
                }

                char *result = malloc(strlen(content) + (len_with - len_rep) * count + 1);
                if (!result) { 
                    free(val_str); 
                    break; 
                }

                char *dest = result;
                char *src = content;
                while (count--) {
                    ins = strstr(src, placeholder);
                    int len_front = (int)(ins - src);
                    memcpy(dest, src, len_front);
                    dest += len_front;
                    memcpy(dest, val_str, len_with);
                    dest += len_with;
                    src = ins + len_rep;
                }
                strcpy(dest, src);
                
                free(content); 
                content = result; 
            }
            free(val_str); 
        }
    }

    content = clean_section_tags(content);

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
    if (arity != 2 || args[0].type != VAL_NUMBER || args[1].type != VAL_STRING) {
        return (Value){VAL_NIL};
    }

    int client_socket = (int)args[0].as.number;
    const char *html_content = args[1].as.string;
    size_t content_len = strlen(html_content);

    char header[512];
    int header_len = snprintf(header, sizeof(header),
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: text/html\r\n"
        "Content-Length: %zu\r\n"
        "Server: SnapEngine/1.0\r\n"
        "Connection: close\r\n\r\n", 
        content_len);

    send(client_socket, header, header_len, 0);

    send(client_socket, html_content, content_len, 0);

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

Value native_web_send_file(int arity, Value* args) {
    if (arity < 2 || args[1].type != VAL_STRING) return (Value){VAL_NIL, {0}};

    int client_socket;
    if (args[0].type == VAL_MAP) {
        Value socket_val;
        if (!map_get(args[0].as.map, "socket_fd", &socket_val)) return (Value){VAL_NIL, {0}};
        client_socket = (int)socket_val.as.number;
    } else {
        client_socket = (int)args[0].as.number;
    }

    const char* file_path = args[1].as.string;
    FILE* f = fopen(file_path, "rb");
    if (!f) {
        char* error404 = "HTTP/1.1 404 Not Found\r\nContent-Length: 0\r\nConnection: close\r\n\r\n";
        send(client_socket, error404, strlen(error404), 0);
        close(client_socket);
        return (Value){VAL_BOOL, {.boolean = false}};
    }

    fseek(f, 0, SEEK_END);
    long file_size = ftell(f);
    fseek(f, 0, SEEK_SET);

    char header[512];
    int header_len = snprintf(header, sizeof(header),
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: %s\r\n"
        "Content-Length: %ld\r\n"
        "Connection: close\r\n\r\n", 
        get_mime_type(file_path), file_size);
    
    send(client_socket, header, header_len, 0);

    char buffer[8192];
    size_t bytes_read;
    while ((bytes_read = fread(buffer, 1, sizeof(buffer), f)) > 0) {
        send(client_socket, buffer, bytes_read, 0);
    }

    fclose(f);
    close(client_socket);
    return (Value){VAL_BOOL, {.boolean = true}};
}

Value native_web_send_auto(int arity, Value* args) {
    if (arity < 2) return (Value){VAL_NIL, {0}};

    int client_socket;
    if (args[0].type == VAL_MAP) {
        Value socket_val;
        if (!map_get(args[0].as.map, "socket_fd", &socket_val)) return (Value){VAL_NIL, {0}};
        client_socket = (int)socket_val.as.number;
    } else {
        client_socket = (int)args[0].as.number;
    }

    Value data = args[1];

    if (data.type == VAL_MAP || data.type == VAL_ARRAY) {
        AsyncResponseData* async_data = malloc(sizeof(AsyncResponseData));
        async_data->client_socket = client_socket;
        async_data->data = data;

        pthread_t thread;
        pthread_create(&thread, NULL, async_send_thread, async_data);
        pthread_detach(thread);
    } 
    else {
        char* raw_content = value_to_string(data);
        size_t content_len = strlen(raw_content);

        const char* content_type = strchr(raw_content, '<') ? "text/html" : "text/plain";

        char header[512];
        int header_len = snprintf(header, sizeof(header),
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: %s\r\n"
            "Content-Length: %zu\r\n"
            "Connection: close\r\n\r\n", content_type, content_len);

        send(client_socket, header, header_len, 0);
        send(client_socket, raw_content, content_len, 0);
        
        close(client_socket);
        free(raw_content);
    }

    return (Value){VAL_BOOL, {.boolean = true}}; 
}


Value native_node_listen(int arity, Value* args) {
    if (arity < 1 || args[0].type != VAL_NUMBER) return (Value){VAL_NIL};
    
    int port = (int)args[0].as.number;
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    int opt = 1;
    
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    
    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("[NODE] BIND Failed");
        close(server_fd);
        return (Value){VAL_BOOL, {.boolean = false}};
    }
    
    listen(server_fd, 5);
    printf("\n[NODE] Listener active on port %d...\n", port);
    fflush(stdout);

    while (1) {
        int new_socket = accept(server_fd, NULL, NULL);
        if (new_socket < 0) continue;

        char buffer[8192] = {0};
        int valread = read(new_socket, buffer, 8191);
        
        if (valread <= 0) {
            close(new_socket);
            continue;
        }

        buffer[valread] = '\0';

        if (strncmp(buffer, "SPAWN:", 6) == 0) {
            char* code_to_run = buffer + 6;
            printf("[NODE] Spawning dynamic code execution...\n");
            
            execute_source(code_to_run, global_env);
            
            send(new_socket, "{\"status\":\"spawned\"}", 20, 0);
        } else {
            buffer[strcspn(buffer, "\r\n ")] = 0;
            
            Var* func_var = find_var(global_env, buffer); 
            
            if (func_var && func_var->value.type == VAL_FUNCTION) {
                Env* context_env = func_var->value.as.function->env;
                if (context_env == NULL) context_env = global_env;

                Value res = call_jackal_function(context_env, func_var->value, 0, NULL);
                Value json_str_val = builtin_json_encode(1, &res);
                
                if (json_str_val.type == VAL_STRING) {
                    send(new_socket, json_str_val.as.string, strlen(json_str_val.as.string), 0);
                } else {
                    send(new_socket, "null", 4, 0);
                }
            } else {
                send(new_socket, "null", 4, 0);
            }
        }
        
        fflush(stdout);
        close(new_socket);
    }
    return (Value){VAL_NIL};
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
    JWEB_REGISTER(env,"__send_file__",native_web_send_file);
    JWEB_REGISTER(env,"__send__",native_web_send_auto);
    JWEB_REGISTER(env,"__remote__",native_jk_remote_call);
    JWEB_REGISTER(env,"__node_listen__",native_node_listen);
}