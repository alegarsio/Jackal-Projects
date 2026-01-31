#include "http/native_http.h"
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <math.h>
#include<sys/stat.h>
#include<curl/curl.h>

#define HTTP_REGISTER(env, name, func)                                           \
    do                                                                           \
    {                                                                            \
        if (func != NULL)                                                        \
        {                                                                        \
            set_var(env, name, (Value){VAL_NATIVE, {.native = func}}, true, ""); \
        }                                                                        \
        else                                                                     \
        {                                                                        \
            printf("Error: Native function '%s' not implemented!\n", name);      \
        }                                                                        \
    } while (0)


struct HttpBuffer {
    char *data;
    size_t size;
};

static size_t write_callback(void *contents, size_t size, size_t nmemb, void *userp) {
    size_t realsize = size * nmemb;
    struct HttpBuffer *mem = (struct HttpBuffer *)userp;

    char *ptr = realloc(mem->data, mem->size + realsize + 1);
    if (!ptr) return 0; // Out of memory

    mem->data = ptr;
    memcpy(&(mem->data[mem->size]), contents, realsize);
    mem->size += realsize;
    mem->data[mem->size] = 0;

    return realsize;
}
Value native_http_get(int arity, Value *args) {
    
    if (arity < 1 || args[0].type != VAL_STRING) {
        return (Value){VAL_NIL, {0}};
    }

    CURL *curl_handle;
    CURLcode res;
    struct HttpBuffer chunk = {malloc(1), 0};

    curl_global_init(CURL_GLOBAL_ALL);
    curl_handle = curl_easy_init();

    if (curl_handle) {
        curl_easy_setopt(curl_handle, CURLOPT_URL, args[0].as.string);
        curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, write_callback);
        curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void *)&chunk);
        
        curl_easy_setopt(curl_handle, CURLOPT_TIMEOUT, 10L);

        res = curl_easy_perform(curl_handle);

        if (res != CURLE_OK) {
            free(chunk.data);
            curl_easy_cleanup(curl_handle);
            return (Value){VAL_NIL, {0}};
        }

        curl_easy_cleanup(curl_handle);
    }

    Value result = (Value){VAL_STRING, {.string = strdup(chunk.data)}};
    free(chunk.data);
    return result;
}

Value native_http_post(int arity, Value *args) {
    if (arity < 2 || args[0].type != VAL_STRING || args[1].type != VAL_STRING) {
        return (Value){VAL_NIL, {0}};
    }

    CURL *curl_handle;
    CURLcode res;
    struct HttpBuffer chunk = {malloc(1), 0};

    curl_global_init(CURL_GLOBAL_ALL);
    curl_handle = curl_easy_init();

    if (curl_handle) {
        curl_easy_setopt(curl_handle, CURLOPT_URL, args[0].as.string);
        
        curl_easy_setopt(curl_handle, CURLOPT_POST, 1L);
        
        curl_easy_setopt(curl_handle, CURLOPT_POSTFIELDS, args[1].as.string);

        curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, write_callback);
        curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void *)&chunk);
        curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, "Jackal-Interpreter/1.0");

        res = curl_easy_perform(curl_handle);

        if (res != CURLE_OK) {
            free(chunk.data);
            curl_easy_cleanup(curl_handle);
            return (Value){VAL_NIL, {0}};
        }

        curl_easy_cleanup(curl_handle);
    }

    Value result = (Value){VAL_STRING, {.string = strdup(chunk.data)}};
    free(chunk.data);
    return result;
}
void register_http_natives(Env *env){
    HTTP_REGISTER(env,"__http_get",native_http_get);
}