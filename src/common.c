#include "common.h"
#include <stdio.h> // Untuk fprintf, stderr
#include <stdlib.h> // Untuk stderr

void print_error(const char *message) {
    fprintf(stderr, "Error: %s\n", message);
}