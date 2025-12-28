#include "common.h"
#include <stdio.h> 
#include <stdlib.h> 
#include <ctype.h>

size_t bytesAllocated = 0;

GCObject* gc_head = NULL;

void* jackal_allocate_gc(size_t size) {
    GCObject* obj = malloc(sizeof(GCObject) + size);
    if (obj == NULL) return NULL;
    
    obj->is_marked = false;
    obj->next = gc_head;
    gc_head = obj;

    return (void*)(obj + 1);
}

void gc_sweep() {
    GCObject** current = &gc_head;
    while (*current) {
        if (!(*current)->is_marked) {
            GCObject* unreached = *current;
            *current = unreached->next;
            
            free(unreached);
        } else {
            (*current)->is_marked = false;
            current = &(*current)->next;
        }
    }
}
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



void* jackal_malloc(size_t size) {
    void* ptr = malloc(size);
    if (ptr == NULL) {
        fprintf(stderr, "Fatal Error: Out of Memory!\n");
        exit(1);
    }
    bytesAllocated += size; 
    return ptr;
}

void jackal_free(void* ptr, size_t size) {
    if (ptr == NULL) return;
    free(ptr);
    bytesAllocated -= size; 
}