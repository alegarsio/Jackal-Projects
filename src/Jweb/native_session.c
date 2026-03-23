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

void register_session_native(Env* env){
    
}