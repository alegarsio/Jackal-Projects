#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
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
 * @include entry collections dsa
 */
#include "collections/linkedlist.c"


Value builtin_io_open(int argCount, Value* args) {

    if (argCount != 2 || args[0].type != VAL_STRING || args[1].type != VAL_STRING) 
        return (Value){VAL_NIL, {0}};

    FILE* f = fopen(args[0].as.string, args[1].as.string);

    if (f == NULL) 
        return (Value){VAL_NIL, {0}};
    return (Value){VAL_FILE, {.file = f}};
}

Value builtin_io_readAll(int argCount, Value* args) {

    if (argCount != 1 || args[0].type != VAL_FILE) 
        return (Value){VAL_NIL, {0}};

    FILE* f = args[0].as.file;

    if (f == NULL) 
        return (Value){VAL_NIL, {0}};
    
    fseek(f, 0, SEEK_END);
    long fsize = ftell(f);
    rewind(f);
    char* content = malloc(fsize + 1);
    fread(content, 1, fsize, f);
    content[fsize] = '\0';
    return (Value){VAL_STRING, {.string = content}};
}

Value builtin_io_write(int argCount, Value* args) {
    if (argCount != 2 || args[0].type != VAL_FILE || args[1].type != VAL_STRING) return (Value){VAL_NIL, {0}};
    FILE* f = args[0].as.file;
    if (f == NULL) return (Value){VAL_NIL, {0}};
    fputs(args[1].as.string, f);
    return (Value){VAL_NIL, {0}};
}

Value builtin_io_close(int argCount, Value* args) {
    if (argCount != 1 || args[0].type != VAL_FILE) return (Value){VAL_NIL, {0}};
    FILE* f = args[0].as.file;
    if (f == NULL) return (Value){VAL_NIL, {0}};
    fclose(f);
    return (Value){VAL_NIL, {0}};
}




/**
 * Math Built In operation 
 * (Beta) 
 */

Value builtin_math_sqrt(int argCount, Value* args) {
    if (argCount != 1 || args[0].type != VAL_NUMBER) {
        print_error("sqrt() requires one number argument.");
        return (Value){VAL_NIL, {0}};
    }
    return (Value){VAL_NUMBER, {.number = sqrt(args[0].as.number)}};
}

Value builtin_math_pow(int argCount, Value* args) {
    if (argCount != 2 || args[0].type != VAL_NUMBER || args[1].type != VAL_NUMBER) {
        print_error("pow() requires two number arguments (base, exponent).");
        return (Value){VAL_NIL, {0}};
    }
    return (Value){VAL_NUMBER, {.number = pow(args[0].as.number, args[1].as.number)}};
}


/**
 * @brief Built-in function typeof 
 * @param argCount The number of arguments passed to the function.
 * @param args The array of argument Values.
 * @return A Value representing the length.
 */
Value builtin_typeof(int argCount, Value* args) {
    if (argCount != 1) {
        fprintf(stderr, "Runtime Error: typeof() takes exactly 1 argument.\n");
        return (Value){VAL_NIL, {0}};
    }

    Value arg = args[0];
    const char* type_string;

    switch (arg.type) {
        case VAL_NIL:       type_string = "nil"; break;
        case VAL_NUMBER:    type_string = "number"; break;
        case VAL_STRING:    type_string = "string"; break;
        case VAL_FUNCTION:  type_string = "function"; break;
        case VAL_NATIVE:    type_string = "function"; break;
        case VAL_ARRAY:     type_string = "array"; break;
        case VAL_MAP:       type_string = "map"; break;
        case VAL_CLASS:     type_string = "class"; break;
        case VAL_INSTANCE:  type_string = "instance"; break;
        case VAL_INTERFACE: type_string = "interface"; break;
        case VAL_ENUM:      type_string = "enum"; break;
        case VAL_FILE:      type_string = "file"; break;
        default:            type_string = "unknown"; break;
    }

    char* str_copy = malloc(strlen(type_string) + 1);
    strcpy(str_copy, type_string);
    return (Value){VAL_STRING, {.string = str_copy}};
}

/**
 * @brief Built-in function File() to read and write FILE
 * @param argCount The number of arguments passed to the function.
 * @param args The array of argument Values.
 * @return A Value representing the length.
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



Value builtin_ll_new(int argCount, Value* args) {
    (void)argCount; (void)args; // Unused
    return (Value){VAL_LINKEDLIST, {.list = linkedlist_new()}};
}
Value builtin_ll_append(int argCount, Value* args) {
    linkedlist_append(args[0].as.list, args[1]);
    return args[0];
}
Value builtin_ll_prepend(int argCount, Value* args) {
    linkedlist_prepend(args[0].as.list, args[1]);
    return args[0];
}
Value builtin_ll_remove_first(int argCount, Value* args) {
    return linkedlist_remove_first(args[0].as.list);
}
Value builtin_ll_size(int argCount, Value* args) {
    return (Value){VAL_NUMBER, {.number = (double)linkedlist_size(args[0].as.list)}};
}



/**
 * Execute Source file
 */

void execute_source(const char* source, Env* env) {
    Lexer L;
    Parser P;
    
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
}


/**
 * REPL initial
 */
void runRepl(Env* env) {
    char line[1024];
    printf("Jackal Beta Version  Shell\n");
    printf("Type 'exit' to quit.\n");

    while (1) {
        printf(">> ");
        
        if (!fgets(line, sizeof(line), stdin)) {
            printf("\n");
            break;
        }

        if (strncmp(line, "exit", 4) == 0) break;

        execute_source(line, env);
    }
}

/**
 * Runfile
 */
void runFile(const char* path, Env* env) {
    FILE *f = fopen(path, "r");
    if (!f) {
        perror("Failed to open file");
        exit(1);
    }
    fseek(f, 0, SEEK_END);
    long len = ftell(f);
    rewind(f);
    char *source = malloc(len + 1);
    if (!source) { fclose(f); return; }
    fread(source, 1, len, f);
    source[len] = '\0';
    fclose(f);

    execute_source(source, env);
    free(source);
}


int main(int argc, char **argv) {
    
    Env* env = env_new(NULL);
    /**
     * Set Of Jackal Built In Method Entry
     */
    set_var(env, "nil", (Value){VAL_NIL, {0}},true);
    set_var(env, "typeof", (Value){VAL_NATIVE, {.native = builtin_typeof}}, true);


    /**
     * Macro for nativce syntax
     */
    #define DEFINE_NATIVE(name_str, func_ptr) \
        do { \
            Value val = (Value){VAL_NATIVE, {.native = func_ptr}}; \
            set_var(env, name_str, val,true); \
        } while(0)

    /**
     * Register Global function
     */
    #define REGISTER(name, func) set_var(env, name, (Value){VAL_NATIVE, {.native = func}}, true)

    REGISTER("__math_sqrt", builtin_math_sqrt);
    REGISTER("__math_pow", builtin_math_pow);
    set_var(env, "__math_PI", (Value){VAL_NUMBER, {.number = 3.1415926535}}, true);


    REGISTER("__io_open", builtin_io_open);
    REGISTER("__io_readAll", builtin_io_readAll);
    REGISTER("__io_write", builtin_io_write);
    REGISTER("__io_close", builtin_io_close);


    REGISTER("__ll_new", builtin_ll_new);
    REGISTER("__ll_append", builtin_ll_append);
    REGISTER("__ll_prepend", builtin_ll_prepend);
    REGISTER("__ll_removeFirst", builtin_ll_remove_first);
    REGISTER("__ll_size", builtin_ll_size);


    DEFINE_NATIVE("len", builtin_len);
    DEFINE_NATIVE("push", builtin_push);
    DEFINE_NATIVE("pop", builtin_pop);
    DEFINE_NATIVE("remove", builtin_remove);
    DEFINE_NATIVE("File", builtin_file_open);
    

    if (argc == 1) {
    
        runRepl(env);
    } else {
       
        runFile(argv[1], env);
    }

    env_free(env);

    return 0;
}