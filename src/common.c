#ifndef COMMON_H
#define COMMON_H
#include <stdio.h>

void print_error(const char *message) {
    fprintf(stderr, "Error: %s\n", message);
}
#endif