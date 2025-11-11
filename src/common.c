#include "common.h"
#include <stdio.h> 
#include <stdlib.h> 


/**
 * src/common.c
 * @param message Error message to be printed.
 * print_error prints the provided error message to stderr.
 */
void print_error(const char *format, ...) {
    fprintf(stderr, "Error: "); 
    
    
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
    
    fprintf(stderr, "\n"); 
}