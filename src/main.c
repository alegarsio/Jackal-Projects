#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "common.h"
#include "lexer.h"
#include "env.h"
#include "value.h"
#include "parser.h"
#include "eval.h"

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