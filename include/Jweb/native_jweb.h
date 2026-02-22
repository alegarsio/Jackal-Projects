#ifndef JWEB_REGISTRY_H
#define JWEB_REGISTRY_H

#include "common.h"
#include "env.h"
#include "value.h"
#include "eval.h"

void register_jweb_natives(Env* env);

Value wrap_request_to_map(char* buffer, char* method, char* path);
Value wrap_response_to_obj(int socket_fd);

#endif