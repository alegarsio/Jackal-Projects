#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

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
 * 
 */

Value builtin_file_open(int argCount, Value* args) {
    
    if (argCount != 2) {
        fprintf(stderr, "[DEBUG] Error: Argumen File() kurang atau lebih dari 2\n");
        return (Value){VAL_NIL, {0}};
    }
    if (args[0].type != VAL_STRING || args[1].type != VAL_STRING) {
        fprintf(stderr, "[DEBUG] Error: Argumen File() bukan string\n");
        return (Value){VAL_NIL, {0}};
    }

    const char* path = args[0].as.string;
    const char* mode = args[1].as.string;

    FILE* f = fopen(path, mode);
    if (f == NULL) {
        perror(""); 
        return (Value){VAL_NIL, {0}};
    }

   
    return (Value){VAL_FILE, {.file = f}};
}

/**
 * @brief Built-in function 'len' to get the length of a string or array.
 * @param argCount The number of arguments passed to the function.
 * @param args The array of argument Values.
 * @return A Value representing the length.
 */

Value builtin_len(int argCount, Value* args) {
    if (argCount != 1) {
        fprintf(stderr, "Runtime Error: 'len' takes exactly 1 argument.\n");
        return (Value){VAL_NIL, {0}};
    }

    Value arg = args[0];
    
    if (arg.type == VAL_STRING) {
        return (Value){VAL_NUMBER, {.number = (double)strlen(arg.as.string)}};
    }
    
    if (arg.type == VAL_ARRAY) {
        return (Value){VAL_NUMBER, {.number = (double)arg.as.array->count}};
    }

    fprintf(stderr, "[DEBUG] len() called on invalid type: %d\n", arg.type);
    
    if (arg.type == VAL_NIL) {
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
Value builtin_push(int argCount, Value* args) {
    if (argCount != 2) {
        fprintf(stderr, "Runtime Error: 'push' takes exactly 2 arguments (array, value).\n");
        return (Value){VAL_NIL, {0}};
    }
    if (args[0].type != VAL_ARRAY) {
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

Value builtin_pop(int argCount, Value* args) {
    if (argCount != 1) {
        fprintf(stderr, "Runtime Error: 'pop' takes exactly 1 argument (array).\n");
        return (Value){VAL_NIL, {0}};
    }
    if (args[0].type != VAL_ARRAY) {
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

Value builtin_remove(int argCount, Value* args) {
    if (argCount != 2) {
        fprintf(stderr, "Runtime Error: 'remove' takes exactly 2 arguments (array, index).\n");
        return (Value){VAL_NIL, {0}};
    }
    if (args[0].type != VAL_ARRAY || args[1].type != VAL_NUMBER) {
        fprintf(stderr, "Runtime Error: Invalid arguments for 'remove'. Expected (array, index number).\n");
        return (Value){VAL_NIL, {0}};
    }
    
    int index = (int)args[1].as.number;
    ValueArray* arr = args[0].as.array;
    
    if (index < 0 || index >= arr->count) {
        fprintf(stderr, "Runtime Error: Array index out of bounds.\n");
        return (Value){VAL_NIL, {0}};
    }

    array_delete(arr, index);
    return (Value){VAL_NIL, {0}};
}





int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <file.jackal>\n", argv[0]);
        return 1;
    }

    const char *filename = argv[1];

    const char *ext = strrchr(filename, '.');
    if (!ext || strcmp(ext, ".jackal") != 0) {
        fprintf(stderr, "Error: file must have .jackal extension\n");
        return 1;
    }

    FILE *f = fopen(filename, "r");
    if (!f) {
        perror("Failed to open file");
        return 1;
    }

    fseek(f, 0, SEEK_END);
    long len = ftell(f);
    rewind(f);
    char *source = malloc(len + 1);
    if (!source) {
        fprintf(stderr, "Failed to allocate memory for file\n");
        fclose(f);
        return 1;
    }
    fread(source, 1, len, f);
    source[len] = '\0';
    fclose(f);

    Lexer L;
    Parser P;
    Env* env = env_new(NULL);
    set_var(env, "nil", (Value){VAL_NIL, {0}},true);

    #define DEFINE_NATIVE(name_str, func_ptr) \
        do { \
            Value val = (Value){VAL_NATIVE, {.native = func_ptr}}; \
            set_var(env, name_str, val,true); \
        } while(0)


    DEFINE_NATIVE("len", builtin_len);
    DEFINE_NATIVE("push", builtin_push);
    DEFINE_NATIVE("pop", builtin_pop);
    DEFINE_NATIVE("remove", builtin_remove);
    DEFINE_NATIVE("File", builtin_file_open);

    lexer_init(&L, source);
    parser_init(&P, &L);

    while (P.current.kind != TOKEN_END) {
        Node* stmt = parse_stmt(&P);

        if (stmt) {
            Value result = eval_node(env, stmt);
            free_value(result); 
            free_node(stmt);
        } else {
            break;
        }
    }

    free(source);
    env_free(env);

    return 0;
}