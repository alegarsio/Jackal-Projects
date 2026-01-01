#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include <errno.h>
#include <curl/curl.h>
#include <cjson/cJSON.h>
#include <time.h>
/**
 * @include socket built in
 * represents the usage
 */
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
/**
 * @incude network DB
 */
#include <netdb.h>
/**
 * Main entry point for the Jackal interpreter.
 * @include necessary headers
 * Initialize lexer, parser, and environment.
 * Read source file and execute statements.
 */
#include "common.h"
#include "lexer.h"
#include "env.h"
#include "value.h"
#include "parser.h"
#include "eval.h"

/**
 * @include vm debug option
 */
#include "vm/debug.h"
/**
 * @include compiler option
 */
#include "compiler/compiler.h"
#include "vm/vm.h"

#define HTTP_DEFAULT_PORT 80
#define BUFFER_SIZE 4096

char* value_to_string(Value value) {
    char buffer[128];

    switch (value.type) {
        case VAL_NUMBER:
            snprintf(buffer, sizeof(buffer), "%g", value.as.number);
            return strdup(buffer);
        
        case VAL_STRING:
            return strdup(value.as.string);
        
        case TOKEN_TRUE:
            return strdup("true");
        
        case TOKEN_FALSE:
            return strdup("false");
        
        case VAL_NIL:
            return strdup("nil");

        case VAL_MAP:
            return strdup("[Map]");

        case VAL_ARRAY:
            return strdup("[Array]");

        default:
            return strdup("[Unknown]");
    }
}
Value builtin_vm_memory(int argCount, Value *args)
{
    (void)argCount;
    (void)args;
    return (Value){VAL_NUMBER, {.number = (double)bytesAllocated}};
}

/**
 * return true if file extension is jlo for compliled result and jackal for base file
 * @param filename represents the filename
 * @param ext
 */
int has_extension(const char *filename, const char *ext)
{
    const char *dot = strrchr(filename, '.');
    return (dot && strcmp(dot, ext) == 0);
}
/**
 * read file content
 * @param path represents the path of the file
 */
char *read_file_content(const char *path)
{
    FILE *f = fopen(path, "r");
    if (!f)
        return NULL;
    fseek(f, 0, SEEK_END);
    long len = ftell(f);
    rewind(f);
    char *buf = malloc(len + 1);
    fread(buf, 1, len, f);
    buf[len] = 0;
    fclose(f);
    return buf;
}
struct Memory
{
    char *memory;
    size_t size;
};

Value builtin_net_listen(int argCount, Value *args)
{
    if (argCount != 1 || args[0].type != VAL_NUMBER)
        return (Value){VAL_NIL, {0}};

    int port = (int)args[0].as.number;
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (server_fd == -1)
        return (Value){VAL_NUMBER, {.number = -1.0}};

    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0)
    {
        close(server_fd);
        return (Value){VAL_NUMBER, {.number = -1.0}};
    }

    if (listen(server_fd, 5) < 0)
    {
        close(server_fd);
        return (Value){VAL_NUMBER, {.number = -1.0}};
    }

    return (Value){VAL_NUMBER, {.number = (double)server_fd}};
}

Value builtin_net_accept(int argCount, Value *args)
{
    if (argCount != 1 || args[0].type != VAL_NUMBER)
        return (Value){VAL_NIL, {0}};

    int server_fd = (int)args[0].as.number;
    struct sockaddr_in address;
    socklen_t addrlen = sizeof(address);
    int new_socket = accept(server_fd, (struct sockaddr *)&address, &addrlen);

    if (new_socket < 0)
        return (Value){VAL_NUMBER, {.number = -1.0}};

    return (Value){VAL_NUMBER, {.number = (double)new_socket}};
}

Value builtin_net_send(int argCount, Value *args)
{
    if (argCount != 2 || args[0].type != VAL_NUMBER || args[1].type != VAL_STRING)
        return (Value){VAL_NIL, {0}};

    int socket_fd = (int)args[0].as.number;
    const char *message = args[1].as.string;

    ssize_t sent = send(socket_fd, message, strlen(message), 0);
    return (Value){VAL_NUMBER, {.number = (double)sent}};
}

Value builtin_net_close(int argCount, Value *args)
{
    if (argCount != 1 || args[0].type != VAL_NUMBER)
        return (Value){VAL_NIL, {0}};

    int socket_fd = (int)args[0].as.number;
    int res = close(socket_fd);
    return (Value){VAL_NUMBER, {.number = (double)res}};
}

Value builtin_writeline(int argCount, Value *args)
{
    if (argCount != 1)
    {
        print_error("Error: 'println' requires exactly one argument.");
        return (Value){.type = VAL_NIL, .as = {0}};
    }

    print_value(args[0]);

    printf("\n");
    fflush(stdout);

    return (Value){.type = VAL_NIL, .as = {0}};
}

Value builtin_write(int argCount, Value *args)
{
    if (argCount != 1)
    {
        print_error("Error: 'println' requires exactly one argument.");
        return (Value){.type = VAL_NIL, .as = {0}};
    }

    print_value(args[0]);

    fflush(stdout);

    return (Value){.type = VAL_NIL, .as = {0}};
}
Value builtin_net_resolve_ip(int argCount, Value *args)
{
    if (argCount != 1 || args[0].type != VAL_STRING)
    {
        print_error("resolve_ip() requires one string argument (hostname).");
        return (Value){VAL_NIL, {0}};
    }

    const char *hostname = args[0].as.string;
    struct addrinfo hints, *res, *p;
    int status;
    char ipstr[INET6_ADDRSTRLEN];

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    if ((status = getaddrinfo(hostname, NULL, &hints, &res)) != 0)
    {
        print_error("getaddrinfo error: %s", gai_strerror(status));
        return (Value){VAL_NIL, {0}};
    }

    for (p = res; p != NULL; p = p->ai_next)
    {
        void *addr;
        char *ipver;

        if (p->ai_family == AF_INET)
        {
            struct sockaddr_in *ipv4 = (struct sockaddr_in *)p->ai_addr;
            addr = &(ipv4->sin_addr);
            ipver = "IPv4";
        }
        else
        {
            struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *)p->ai_addr;
            addr = &(ipv6->sin6_addr);
            ipver = "IPv6";
        }

        inet_ntop(p->ai_family, addr, ipstr, sizeof ipstr);
        break;
    }

    freeaddrinfo(res);

    if (ipstr[0] == '\0')
    {
        print_error("Could not find IP address for hostname '%s'.", hostname);
        return (Value){VAL_NIL, {0}};
    }

    char *ip_copy = malloc(strlen(ipstr) + 1);
    strcpy(ip_copy, ipstr);

    return (Value){VAL_STRING, {.string = ip_copy}};
}

static size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
    size_t realsize = size * nmemb;
    struct Memory *mem = (struct MemoryStruct *)userp;

    char *ptr = realloc(mem->memory, mem->size + realsize + 1);
    if (ptr == NULL)
    {
        return 0;
    }

    mem->memory = ptr;
    memcpy(&(mem->memory[mem->size]), contents, realsize);
    mem->size += realsize;
    mem->memory[mem->size] = 0;

    return realsize;
}


Value builtin_http_request(int argCount, Value *args)
{
    if (argCount < 2 || args[0].type != VAL_STRING || args[1].type != VAL_STRING)
    {
        print_error("Usage: http_request(method, url, [body], [headers_map])");
        return (Value){VAL_NIL, {0}};
    }

    const char *method = args[0].as.string;
    const char *url = args[1].as.string;
    const char *body = (argCount > 2 && args[2].type == VAL_STRING) ? args[2].as.string : NULL;

    CURL *curl_handle = curl_easy_init();
    if (!curl_handle)
        return (Value){VAL_NIL, {0}};

    struct Memory chunk = {.memory = malloc(1), .size = 0};
    struct curl_slist *headers = NULL;

    curl_easy_setopt(curl_handle, CURLOPT_URL, url);
    curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
    curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void *)&chunk);
    curl_easy_setopt(curl_handle, CURLOPT_TIMEOUT, 30L);
    curl_easy_setopt(curl_handle, CURLOPT_FOLLOWLOCATION, 1L);

    if (argCount > 3 && args[3].type == VAL_MAP)
    {
        HashMap *map = args[3].as.map;
        for (int i = 0; i < map->capacity; i++)
        {
            Entry *entry = &map->entries[i];
            if (entry->key != NULL)
            {
                char header_buffer[1024];
                snprintf(header_buffer, sizeof(header_buffer), "%s: %s",
                         entry->key, value_to_string(entry->value));
                headers = curl_slist_append(headers, header_buffer);
            }
        }
    }
    else
    {
        headers = curl_slist_append(headers, "Accept: application/json");
        if (body)
        {
            headers = curl_slist_append(headers, "Content-Type: application/json");
        }
    }

    curl_easy_setopt(curl_handle, CURLOPT_HTTPHEADER, headers);

    if (strcmp(method, "POST") == 0)
    {
        curl_easy_setopt(curl_handle, CURLOPT_POST, 1L);
        if (body)
            curl_easy_setopt(curl_handle, CURLOPT_POSTFIELDS, body);
    }
    else if (strcmp(method, "PUT") == 0)
    {
        curl_easy_setopt(curl_handle, CURLOPT_CUSTOMREQUEST, "PUT");
        if (body)
            curl_easy_setopt(curl_handle, CURLOPT_POSTFIELDS, body);
    }
    else if (strcmp(method, "DELETE") == 0)
    {
        curl_easy_setopt(curl_handle, CURLOPT_CUSTOMREQUEST, "DELETE");
    }
    else if (strcmp(method, "PATCH") == 0)
    {
        curl_easy_setopt(curl_handle, CURLOPT_CUSTOMREQUEST, "PATCH");
        if (body)
            curl_easy_setopt(curl_handle, CURLOPT_POSTFIELDS, body);
    }

    CURLcode res = curl_easy_perform(curl_handle);

    if (res != CURLE_OK)
    {
        print_error("Network Error: %s", curl_easy_strerror(res));
        goto cleanup;
    }

    long http_code = 0;
    curl_easy_getinfo(curl_handle, CURLINFO_RESPONSE_CODE, &http_code);

    if (http_code >= 400)
    {
        print_error("HTTP Error %ld", http_code);
    }

cleanup:
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl_handle);

    Value result;
    result.type = VAL_STRING;
    result.as.string = strdup(chunk.memory); 

    free(chunk.memory);
    return result;
}
Value builtin_json_encode(int argCount, Value *args)
{
    if (argCount != 1)
    {
        print_error("json_encode requires one argument.");
        return (Value){VAL_NIL, {0}};
    }

    cJSON *root = jackal_value_to_cjson(args[0]);

    if (root == NULL)
    {
        print_error("json_encode failed to serialize object (Unsupported type).");
        return (Value){VAL_NIL, {0}};
    }

    char *json_string = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);

    if (json_string == NULL)
    {
        print_error("cJSON_PrintUnformatted failed.");
        return (Value){VAL_NIL, {0}};
    }

    Value result = (Value){VAL_STRING, {.string = json_string}};

    return result;
}

Value builtin_time_sleep(int argCount, Value *args)
{
    if (argCount != 1 || args[0].type != VAL_NUMBER)
        return (Value){VAL_NIL, {0}};

    long ms = (long)args[0].as.number;
    usleep(ms * 1000);
    return (Value){VAL_NIL, {0}};
}

Value builtin_time_now(int argCount, Value *args)
{
    if (argCount != 0)
        return (Value){VAL_NIL, {0}};

    time_t current_time = time(NULL);
    return (Value){VAL_NUMBER, {.number = (double)current_time}};
}
Value builtin_time_get_local_hour(int argCount, Value *args)
{
    if (argCount != 0)
        return (Value){VAL_NIL, {0}};

    time_t rawtime;
    struct tm *info;

    time(&rawtime);
    info = localtime(&rawtime);

    return (Value){VAL_NUMBER, {.number = (double)info->tm_hour}};
}
/**
 * this method is part of std.io library
 * std.io.File Built In Open ( Create ) Method
 * this method is for create and open file
 * @param argCount
 * @param args
 */
Value builtin_io_open(int argCount, Value *args)
{

    if (argCount != 2 || args[0].type != VAL_STRING || args[1].type != VAL_STRING)
        return (Value){VAL_NIL, {0}};

    FILE *f = fopen(args[0].as.string, args[1].as.string);

    if (f == NULL)
        return (Value){VAL_NIL, {0}};
    return (Value){VAL_FILE, {.file = f}};
}

Value builtin_io_readAll(int argCount, Value *args)
{

    if (argCount != 1 || args[0].type != VAL_FILE)
        return (Value){VAL_NIL, {0}};

    FILE *f = args[0].as.file;

    if (f == NULL)
        return (Value){VAL_NIL, {0}};

    fseek(f, 0, SEEK_END);
    long fsize = ftell(f);
    rewind(f);
    char *content = malloc(fsize + 1);
    fread(content, 1, fsize, f);
    content[fsize] = '\0';
    return (Value){VAL_STRING, {.string = content}};
}

Value builtin_io_write(int argCount, Value *args)
{
    if (argCount != 2 || args[0].type != VAL_FILE || args[1].type != VAL_STRING)
        return (Value){VAL_NIL, {0}};
    FILE *f = args[0].as.file;
    if (f == NULL)
        return (Value){VAL_NIL, {0}};
    fputs(args[1].as.string, f);
    return (Value){VAL_NIL, {0}};
}

Value builtin_io_close(int argCount, Value *args)
{
    if (argCount != 1 || args[0].type != VAL_FILE)
        return (Value){VAL_NIL, {0}};
    FILE *f = args[0].as.file;
    if (f == NULL)
        return (Value){VAL_NIL, {0}};
    fclose(f);
    return (Value){VAL_NIL, {0}};
}

Value builtin_net_htons(int argCount, Value *args)
{
    if (argCount != 1 || args[0].type != VAL_NUMBER)
    {
        print_error("htons() requires one number argument (port).");
        return (Value){VAL_NUMBER, {.number = -1.0}};
    }

    uint16_t port = (uint16_t)args[0].as.number;

    uint16_t net_port = htons(port);

    return (Value){VAL_NUMBER, {.number = (double)net_port}};
}

Value builtin_math_fmod(int argCount, Value *args)
{
    if (argCount != 2 || args[0].type != VAL_NUMBER || args[1].type != VAL_NUMBER)
    {
        print_error("fmod() requires two number arguments (numerator, denominator).");
        return (Value){VAL_NIL, {0}};
    }
    return (Value){VAL_NUMBER, {.number = fmod(args[0].as.number, args[1].as.number)}};
}

Value builtin_net_aton(int argCount, Value *args)
{
    if (argCount != 1 || args[0].type != VAL_STRING)
    {
        print_error("aton() requires one string argument (IPv4 address).");
        return (Value){VAL_NUMBER, {.number = -1.0}};
    }

    const char *ip_string = args[0].as.string;

    in_addr_t binary_address = inet_addr(ip_string);

    if (binary_address == INADDR_NONE)
    {
        print_error("Invalid IPv4 address format: %s", ip_string);
        return (Value){VAL_NUMBER, {.number = -1.0}};
    }

    return (Value){VAL_NUMBER, {.number = (double)binary_address}};
}

Value builtin_net_ntoa(int argCount, Value *args)
{
    if (argCount != 1 || args[0].type != VAL_NUMBER)
    {
        print_error("ntoa() requires one number argument (binary IP).");
        return (Value){VAL_NIL, {0}};
    }

    in_addr_t binary_address = (in_addr_t)args[0].as.number;

    struct in_addr addr;
    addr.s_addr = binary_address;

    char *ip_string = inet_ntoa(addr);

    if (ip_string == NULL)
    {
        return (Value){VAL_NIL, {0}};
    }

    char *ip_copy = malloc(strlen(ip_string) + 1);
    strcpy(ip_copy, ip_string);

    return (Value){VAL_STRING, {.string = ip_copy}};
}

Value builtin_jackal_sleep(int argCount, Value *args)
{
    if (argCount != 1 || args[0].type != VAL_NUMBER)
        return (Value){VAL_NIL, {0}};

    long ms = (long)args[0].as.number;
    usleep(ms * 1000);
    return (Value){VAL_NIL, {0}};
}

/**
 * Math Built In operation
 * (Beta)
 */

Value builtin_math_sqrt(int argCount, Value *args)
{
    if (argCount != 1 || args[0].type != VAL_NUMBER)
    {
        print_error("sqrt() requires one number argument.");
        return (Value){VAL_NIL, {0}};
    }
    return (Value){VAL_NUMBER, {.number = sqrt(args[0].as.number)}};
}

Value builtin_math_pow(int argCount, Value *args)
{
    if (argCount != 2 || args[0].type != VAL_NUMBER || args[1].type != VAL_NUMBER)
    {
        print_error("pow() requires two number arguments (base, exponent).");
        return (Value){VAL_NIL, {0}};
    }
    return (Value){VAL_NUMBER, {.number = pow(args[0].as.number, args[1].as.number)}};
}

/**
 * @brief Built-in function typeof
 * @param argCount The number of arguments passed to the function.
 * @param args The array of argument Values.
 * @return A Value representing the length.
 */
Value builtin_typeof(int argCount, Value *args)
{
    if (argCount != 1)
    {
        fprintf(stderr, "Runtime Error: typeof() takes exactly 1 argument.\n");
        return (Value){VAL_NIL, {0}};
    }

    Value arg = args[0];
    const char *type_string;

    switch (arg.type)
    {
    case VAL_NIL:
        type_string = "nil";
        break;
    case VAL_NUMBER:
        type_string = "number";
        break;
    case VAL_STRING:
        type_string = "string";
        break;
    case VAL_FUNCTION:
        type_string = "function";
        break;
    case VAL_NATIVE:
        type_string = "function";
        break;
    case VAL_ARRAY:
        type_string = "array";
        break;
    case VAL_MAP:
        type_string = "map";
        break;
    case VAL_CLASS:
        type_string = "class";
        break;
    case VAL_INSTANCE:
        type_string = "instance";
        break;
    case VAL_INTERFACE:
        type_string = "interface";
        break;
    case VAL_ENUM:
        type_string = "enum";
        break;
    case VAL_FILE:
        type_string = "file";
        break;
    default:
        type_string = "unknown";
        break;
    }

    char *str_copy = malloc(strlen(type_string) + 1);
    strcpy(str_copy, type_string);
    return (Value){VAL_STRING, {.string = str_copy}};
}

/**
 * @brief Built-in function File() to read and write FILE
 * @param argCount The number of arguments passed to the function.
 * @param args The array of argument Values.
 * @return A Value representing the length.
 */

Value builtin_file_open(int argCount, Value *args)
{

    if (argCount != 2)
    {
        fprintf(stderr, "[DEBUG] Error: Argumen File() kurang atau lebih dari 2\n");
        return (Value){VAL_NIL, {0}};
    }
    if (args[0].type != VAL_STRING || args[1].type != VAL_STRING)
    {
        fprintf(stderr, "[DEBUG] Error: Argumen File() bukan string\n");
        return (Value){VAL_NIL, {0}};
    }

    const char *path = args[0].as.string;
    const char *mode = args[1].as.string;

    FILE *f = fopen(path, mode);
    if (f == NULL)
    {
        perror("");
        return (Value){VAL_NIL, {0}};
    }

    return (Value){VAL_FILE, {.file = f}};
}

void load_jackal_file(const char *path, Env *env)
{
    char *source = read_file_content(path);
    if (!source)
    {
        printf("Warning: Standard library file '%s' not found or empty.\n", path);
        return;
    }

    Lexer L;
    Parser P;

    lexer_init(&L, source);
    parser_init(&P, &L);

    while (P.current.kind != TOKEN_END)
    {
        Node *stmt = parse_stmt(&P);
        if (stmt)
        {
            Value result = eval_node(env, stmt);
            free_value(result);
            free_node(stmt);
        }
    }
    free(source);
}

/**
 * @brief Built-in function 'len' to get the length of a string or array.
 * @param argCount The number of arguments passed to the function.
 * @param args The array of argument Values.
 * @return A Value representing the length.
 */

Value builtin_len(int argCount, Value *args)
{
    if (argCount != 1)
    {
        fprintf(stderr, "Runtime Error: 'len' takes exactly 1 argument.\n");
        return (Value){VAL_NIL, {0}};
    }

    Value arg = args[0];

    if (arg.type == VAL_STRING)
    {
        return (Value){VAL_NUMBER, {.number = (double)strlen(arg.as.string)}};
    }

    if (arg.type == VAL_ARRAY)
    {
        return (Value){VAL_NUMBER, {.number = (double)arg.as.array->count}};
    }

    fprintf(stderr, "[DEBUG] len() called on invalid type: %d\n", arg.type);

    if (arg.type == VAL_NIL)
    {
        fprintf(stderr, "Runtime Error: Argument to 'len' is NIL (variable might be empty).\n");
    }

    return (Value){VAL_NIL, {0}};
}

/**
 * @brief Built-in function 'push' to append a value to an array.
 * @param argCount The number of arguments passed to the function.
 * @param args The array of argument Values.
 * @return A Value indicating success or failure.
 */
Value builtin_push(int argCount, Value *args)
{
    if (argCount != 2)
    {
        fprintf(stderr, "Runtime Error: 'push' takes exactly 2 arguments (array, value).\n");
        return (Value){VAL_NIL, {0}};
    }
    if (args[0].type != VAL_ARRAY)
    {
        fprintf(stderr, "Runtime Error: First argument to 'push' must be an array.\n");
        return (Value){VAL_NIL, {0}};
    }
    array_append(args[0].as.array, copy_value(args[1]));
    return (Value){VAL_NIL, {0}};
}

/**
 * @brief Built-in function 'pop' to remove and return the last element of an array.
 * @param argCount The number of arguments passed to the function.
 * @param args The array of argument Values.
 * @return The popped Value, or VAL_NIL on error.
 */

Value builtin_pop(int argCount, Value *args)
{
    if (argCount != 1)
    {
        fprintf(stderr, "Runtime Error: 'pop' takes exactly 1 argument (array).\n");
        return (Value){VAL_NIL, {0}};
    }
    if (args[0].type != VAL_ARRAY)
    {
        fprintf(stderr, "Runtime Error: Argument to 'pop' must be an array.\n");
        return (Value){VAL_NIL, {0}};
    }
    return array_pop(args[0].as.array);
}

/**
 * @brief Built-in function 'remove' to delete an element at a specific index from an array.
 * @param argCount The number of arguments passed to the function.
 * @param args The array of argument Values.
 * @return A Value indicating success or failure.
 */

Value builtin_remove(int argCount, Value *args)
{
    if (argCount != 2)
    {
        fprintf(stderr, "Runtime Error: 'remove' takes exactly 2 arguments (array, index).\n");
        return (Value){VAL_NIL, {0}};
    }
    if (args[0].type != VAL_ARRAY || args[1].type != VAL_NUMBER)
    {
        fprintf(stderr, "Runtime Error: Invalid arguments for 'remove'. Expected (array, index number).\n");
        return (Value){VAL_NIL, {0}};
    }

    int index = (int)args[1].as.number;
    ValueArray *arr = args[0].as.array;

    if (index < 0 || index >= arr->count)
    {
        fprintf(stderr, "Runtime Error: Array index out of bounds.\n");
        return (Value){VAL_NIL, {0}};
    }

    array_delete(arr, index);
    return (Value){VAL_NIL, {0}};
}

/**
 * Execute Source file
 */

void execute_source(const char *source, Env *env)
{
    Lexer L;
    Parser P;

    lexer_init(&L, source);
    parser_init(&P, &L);
    
    load_jackal_file("std/io.jackal", env);
    

    while (P.current.kind != TOKEN_END)
    {
        Node *stmt = parse_stmt(&P);
        if (stmt)
        {
            Value result = eval_node(env, stmt);
            free_value(result);
            free_node(stmt);
        }
        else
        {

            break;
        }
    }
}

/**
 * REPL initial
 */
void runRepl(Env *env)
{
    char line[1024];
    printf("Jackal Beta Version  Shell\n");
    printf("Type 'exit' to quit.\n");

    while (1)
    {
        printf(">> ");

        if (!fgets(line, sizeof(line), stdin))
        {
            printf("\n");
            break;
        }

        if (strncmp(line, "exit", 4) == 0)
            break;

        execute_source(line, env);
    }
}

/**
 * Runfile
 */
void runFile(const char *path, Env *env)
{
    FILE *f = fopen(path, "r");
    if (!f)
    {
        perror("Failed to open file");
        exit(1);
    }
    fseek(f, 0, SEEK_END);
    long len = ftell(f);
    rewind(f);
    char *source = malloc(len + 1);
    if (!source)
    {
        fclose(f);
        return;
    }
    fread(source, 1, len, f);
    source[len] = '\0';
    fclose(f);

    execute_source(source, env);
    free(source);
}

int main(int argc, char **argv)
{

    Env *env = env_new(NULL);
    /**
     * Set Of Jackal Built In Method Entry
     */
    set_var(env, "nil", (Value){VAL_NIL, {0}}, true,"");
    set_var(env, "typeof", (Value){VAL_NATIVE, {.native = builtin_typeof}}, true,"");

/**
 * Macro for nativce syntax
 */
#define DEFINE_NATIVE(name_str, func_ptr)                      \
    do                                                         \
    {                                                          \
        Value val = (Value){VAL_NATIVE, {.native = func_ptr}}; \
        set_var(env, name_str, val, true,"");                     \
    } while (0)

/**
 * Register Global function
 */
#define REGISTER(name, func) set_var(env, name, (Value){VAL_NATIVE, {.native = func}}, true,"")

    REGISTER("__math_sqrt", builtin_math_sqrt);
    REGISTER("__math_pow", builtin_math_pow);
    set_var(env, "__math_PI", (Value){VAL_NUMBER, {.number = 3.1415926535}}, true,"");
    set_var(env, "nil", (Value){VAL_NIL, {0}}, true,"");
    REGISTER("fmod", builtin_math_fmod);

    REGISTER("__io_open", builtin_io_open);
    REGISTER("__io_readAll", builtin_io_readAll);
    REGISTER("__io_write", builtin_io_write);
    REGISTER("__io_close", builtin_io_close);

    /**
     * Network Built In API
     * e.g., Socket, Inet
     */
    REGISTER("__net_listen", builtin_net_listen);
    REGISTER("__net_accept", builtin_net_accept);
    REGISTER("__net_send", builtin_net_send);
    REGISTER("__net_close", builtin_net_close);
    REGISTER("__net_resolve_ip", builtin_net_resolve_ip);
    REGISTER("__net_htons", builtin_net_htons);
    REGISTER("__net_aton", builtin_net_aton);
    REGISTER("__time_now", builtin_time_now);

    REGISTER("__get_local_hour", builtin_time_get_local_hour);
    REGISTER("_io_plot",builtin_plot);

    /**
     * represents the io std/io
     * console (writeln,write)
     */
    REGISTER("println", builtin_writeline);
    REGISTER("print", builtin_write);
    REGISTER("__io_table_stream", builtin_print_table);
    REGISTER("__io_json", builtin_print_json);

    /**
     * Array built in
     */
    REGISTER("__array_distinct", builtin_array_distinct);
    REGISTER("__array_anyMatch", builtin_array_anyMatch);

    REGISTER("__array_map", builtin_array_map);
    REGISTER("__array_filter", builtin_array_filter);
    REGISTER("__array_reduce", builtin_array_reduce);
    REGISTER("__array_sort", builtin_array_sort);
    REGISTER("__array_statistics", builtin_array_statistics);
    REGISTER("__array_mean", builtin_array_mean);
    REGISTER("__array_max", builtin_array_max);
    REGISTER("__array_to_tree", builtin_array_to_tree);

    REGISTER("__zip",native_zip);
    REGISTER("__read_csv",native_read_csv);

    REGISTER("__knn", native_knn_nd);
    REGISTER("__accuracy", native_accuracy);
    REGISTER("__normalize", native_normalize_nd);
    REGISTER("__knn_predictprob",native_knn_prob);
    REGISTER("__confusion_matrix",native_confusion_matrix);
    REGISTER("__split",native_split);
    REGISTER("__sync_shuffle",native_sync_shuffle);

    REGISTER("__logistic_predict",native_logistic_predict);
    REGISTER("__logistic_fit",native_logistic_fit);

    REGISTER("__nb_fit",native_nb_fit);
    REGISTER("__nb_predict",native_nb_predict);
    
    REGISTER("__array_limit", builtin_array_limit);
    REGISTER("__json_string",builtin_json_stringify);

    REGISTER("__kmeans_fit",native_kmeans_fit);
    REGISTER("__kmeans_predict",native_kmeans_predict);
    REGISTER("__kmeans_loss",native_kmeans_loss);

    REGISTER("__matrix_dot",native_matrix_dot);
    REGISTER("__matrix_add",native_matrix_add);
    REGISTER("__matrix_sub",native_matrix_sub);
    REGISTER("__matrix_det",native_matrix_det);

    REGISTER("__matrix_scalar",native_matrix_scalar_mul);


    REGISTER("__io_read_line", builtin_read_line);
    REGISTER("__io_read_array", builtin_read_array);

    REGISTER("http_serve_internal",builtin_http_serve);
    REGISTER("native_standardnize",native_standardize);
    REGISTER("native_smooth",native_smooth);
    REGISTER("native_correlate",native_correlate);
    REGISTER("__mapstream_stream", builtin_map_forEach);
    REGISTER("__mapstream_keys", builtin_map_keys);
    REGISTER("__mapstream_values", builtin_map_values);
    REGISTER("__map_get", builtin_map_get);
   

    /**
     * HTTP Request Client
     */

    REGISTER("http_request", builtin_http_request);
    REGISTER("__json_parse",builtin_json_parse);
    REGISTER("web_show_internal",builtin_web_show);
    REGISTER("web_sync_internal",builtin_web_sync);

    /**
     * Json Parser
     */
    REGISTER("__json_encode", builtin_json_encode);
    REGISTER("__jackal_sleep", builtin_jackal_sleep);
    REGISTER("plot",native_plot);
    REGISTER("native_transpose",native_transpose);
    REGISTER("_native_linear_predicts",native_linear_regression);

    REGISTER("__tensor_add",native_tensor_add);
    REGISTER("__tensor_mul",native_tensor_mul);
    REGISTER("__tensor_sub",native_tensor_sub);
    REGISTER("_tensor_shape",native_tensor_check_shape);


    DEFINE_NATIVE("len", builtin_len);
    DEFINE_NATIVE("push", builtin_push);
    DEFINE_NATIVE("pop", builtin_pop);
    DEFINE_NATIVE("remove", builtin_remove);
    DEFINE_NATIVE("File", builtin_file_open);


    REGISTER("__typeof",builtin_type);
    REGISTER("systems",builtin_system);

    load_jackal_file("std/io.jackal", env);
    load_jackal_file("std/stream.jackal", env);

    if (argc == 1)
    {
        Env *env = env_new(NULL);

        runRepl(env);
        env_free(env);
        return 0;
    }

    if (strcmp(argv[1], "-c") == 0)
    {
        if (argc < 3)
        {
            printf("Usage: ./jackal -c <file.jackal>\n");
            return 1;
        }

        char *source_file = argv[2];
        char dest_file[256];
        strcpy(dest_file, source_file);
        char *dot = strrchr(dest_file, '.');
        if (dot)
            strcpy(dot, ".jlo");
        else
            strcat(dest_file, ".jlo");

        char *source = read_file_content(source_file);
        if (!source)
        {
            printf("Error: File %s tidak ditemukan.\n", source_file);
            return 1;
        }

        Lexer L;
        lexer_init(&L, source);
        Parser P;
        parser_init(&P, &L);

        Node *root = NULL;
        Node *tail = NULL;

        while (P.current.kind != TOKEN_END)
        {
            Node *stmt = parse_stmt(&P);
            if (stmt)
            {
                if (!root)
                {
                    root = stmt;
                    tail = stmt;
                }
                else
                {
                    tail->next = stmt;
                    tail = stmt;
                }
            }
            else
            {
                P.current = lexer_next(&L);
            }
        }

        compile_to_binary(root, dest_file);
        free(source);
        return 0;
    }

    if (strcmp(argv[1], "--dump") == 0)
    {
        if (argc < 3)
        {
            printf("Usage: ./jackal --dump <file.jackal>\n");
            return 1;
        }

        char *source_path = argv[2];
        char *source = read_file_content(source_path);
        if (!source)
        {
            printf("Error reading source.\n");
            return 1;
        }

        Lexer L;
        lexer_init(&L, source);
        Parser P;
        parser_init(&P, &L);

        Node *root = NULL;
        Node *tail = NULL;
        while (P.current.kind != TOKEN_END)
        {
            Node *stmt = parse_stmt(&P);
            if (stmt)
            {
                if (!root)
                {
                    root = stmt;
                    tail = stmt;
                }
                else
                {
                    tail->next = stmt;
                    tail = stmt;
                }
            }
        }

        const char *temp_file = "dump_temp.jlo";
        compile_to_binary(root, temp_file);

        FILE *f = fopen(temp_file, "rb");
        fseek(f, 0, SEEK_END);
        long fsize = ftell(f);
        rewind(f);
        uint8_t *bytecode = malloc(fsize);
        fread(bytecode, 1, fsize, f);
        fclose(f);

        disassemble_chunk(source_path, bytecode, fsize);

        free(source);
        free(bytecode);
        remove(temp_file);
        return 0;
    }

    char *filename = argv[1];

    if (has_extension(filename, ".jlo"))
    {
        run_binary(filename);
    }
    else
    {

        runFile(filename, env);
        env_free(env);
    }

    return 0;
}