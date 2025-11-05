#include "common.h"
#include <stdio.h> 
#include <stdlib.h> 


/**
 * src/common.c
 * @param message Error message to be printed.
 * print_error prints the provided error message to stderr.
 */
void print_error(const char *message) {
    fprintf(stderr, "Error: %s\n", message);
}