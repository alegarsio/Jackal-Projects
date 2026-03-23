#include "Jweb/native_session.h"
#include "json/native_json.h"
#include "eval.h"
#include "value.h"

#include <stdio.h>      
#include <stdlib.h>     
#include <string.h>     
#include <unistd.h>     
#include <sys/socket.h>  
#include <netinet/in.h>  
#include <arpa/inet.h>   
#include <fcntl.h>       
#include <pthread.h>

static HashMap* sessions_storage = NULL;

#define JWEB_SESSION_REGISTER(env, name, func)                                           \
    do                                                                           \
    {                                                                            \
        if (func != NULL)                                                        \
        {                                                                        \
            set_var(env, name, (Value){VAL_NATIVE, {.native = func}}, true, ""); \
        }                                                                        \
    } while (0)

static void ensure_session_storage() {
    if (sessions_storage == NULL) {
        sessions_storage = map_new();
    }
}

static char* generate_sid() {
    char *sid = malloc(17);
    const char *charset = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    for (int n = 0; n < 16; n++) {
        sid[n] = charset[rand() % 62];
    }
    sid[16] = '\0';
    return sid;
}

Value native_session_start(int arity, Value* args) {
    ensure_session_storage();
    
    char* sid = generate_sid();
    HashMap* session_data = map_new();
    
    map_set(sessions_storage, strdup(sid), (Value){VAL_MAP, {.map = session_data}});
    
    return (Value){VAL_STRING, {.string = sid}};
}

Value native_session_set(int arity, Value* args) {
    if (arity < 3 || args[0].type != VAL_STRING) return (Value){VAL_BOOL, {.boolean = false}};
    ensure_session_storage();

    char* sid = args[0].as.string;
    char* key = args[1].as.string;
    Value val = args[2];

    Value s_map_val;
    if (map_get(sessions_storage, sid, &s_map_val) && s_map_val.type == VAL_MAP) {
        map_set(s_map_val.as.map, strdup(key), val);
        return (Value){VAL_BOOL, {.boolean = true}};
    }
    
    return (Value){VAL_BOOL, {.boolean = false}};
}

Value native_session_get(int arity, Value* args) {
    if (arity < 2 || args[0].type != VAL_STRING) return (Value){VAL_NIL};
    ensure_session_storage();

    char* sid = args[0].as.string;
    char* key = args[1].as.string;

    Value s_map_val;
    if (map_get(sessions_storage, sid, &s_map_val) && s_map_val.type == VAL_MAP) {
        Value result;
        if (map_get(s_map_val.as.map, key, &result)) {
            return result;
        }
    }
    
    return (Value){VAL_NIL};
}

void register_session_native(Env* env){
    JWEB_SESSION_REGISTER(env,"__session_start__",native_session_start);
    JWEB_SESSION_REGISTER(env,"__session_set__",native_session_set);
    JWEB_SESSION_REGISTER(env,"__session_get__",native_session_get);
}