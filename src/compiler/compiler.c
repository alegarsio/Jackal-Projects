#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../include/compiler/compiler.h"
#include "../../include/vm/opcode.h"

static FILE* file_out;

void emit_byte(uint8_t byte) {
    fwrite(&byte, sizeof(uint8_t), 1, file_out);
}

void emit_double(double value) {
    fwrite(&value, sizeof(double), 1, file_out);
}

void emit_string(const char* str) {
    int len = strlen(str);
    fwrite(&len, sizeof(int), 1, file_out);
    fwrite(str, sizeof(char), len, file_out);
}

void compile_node(Node* n) {
    if (!n) return;

    switch (n->kind) {
        case NODE_BLOCK: {
            Node* stmt = n->left;
            while (stmt) {
                compile_node(stmt);
                stmt = stmt->next;
            }
            break;
        }
        case NODE_PRINT:
            compile_node(n->right);
            emit_byte(OP_PRINT);
            break;

        case NODE_NUMBER:
            emit_byte(OP_CONST_NUM);
            emit_double(n->value);
            break;

        case NODE_STRING:
            emit_byte(OP_CONST_STR);
            emit_string(n->name);
            break;

        case NODE_BINOP:
            compile_node(n->left);
            compile_node(n->right);
            if (n->op == TOKEN_PLUS) emit_byte(OP_ADD);
            if (n->op == TOKEN_MINUS) emit_byte(OP_SUB);
            if (n->op == TOKEN_STAR) emit_byte(OP_MUL);
            if (n->op == TOKEN_SLASH) emit_byte(OP_DIV);
            break;
        
        default:
            // printf("Warning: Node %d not implemented in compiler yet.\n", n->kind);
            break;
    }
}

void compile_to_binary(Node* ast, const char* filename) {
    file_out = fopen(filename, "wb");
    if (!file_out) {
        printf("Error: Cannot open file %s for writing.\n", filename);
        return;
    }

    // Header Magic: 'J', 'L', Version 1
    uint8_t magic[] = { 'J', 'L', 0x01 };
    fwrite(magic, 1, 3, file_out);

    Node* curr = ast;
    while (curr) {
        compile_node(curr);
        curr = curr->next;
    }

    emit_byte(OP_HALT);
    fclose(file_out);
    printf("[Compiler] Successfully created '%s'\n", filename);
}