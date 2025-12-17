#include "compiler/compiler.h"
#include "vm/opcode.h"
#include "common.h"
#include "value.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>


static void emit_byte(Chunk* chunk, uint8_t byte) {
    writeChunk(chunk, byte);
}

static void emit_bytes(Chunk* chunk, uint8_t byte1, uint8_t byte2) {
    emit_byte(chunk, byte1);
    emit_byte(chunk, byte2);
}

static void emit_constant_operand(Chunk* chunk, OpCode op, Value value) {
    int constant_index = addConstant(chunk, value);
    if (constant_index > 255) {
        fprintf(stderr, "Error: Too many constants in one chunk (> 256)!\n");
        return;
    }
    emit_bytes(chunk, op, (uint8_t)constant_index);
}

static void emit_jump(Chunk* chunk, OpCode op, int* offset_placeholder) {
    emit_byte(chunk, op);
    
    *offset_placeholder = chunk->count;
    emit_byte(chunk, 0xff);
    emit_byte(chunk, 0xff);
}

static void patch_jump(Chunk* chunk, int offset) {
    uint16_t jump = (uint16_t)(chunk->count - offset - 2);
    
    chunk->code[offset] = (uint8_t)((jump >> 8) & 0xff);
    chunk->code[offset + 1] = (uint8_t)(jump & 0xff);
}

static void compile_node(Node* n, Chunk* chunk) {
    if (!n) return;

    switch (n->kind) {
        case NODE_NUMBER: {
            Value val = (Value){VAL_NUMBER, {.number = n->value}};
            emit_constant_operand(chunk, OP_CONST_NUM, val);
            break;
        }

        case NODE_STRING: {
            Value val = (Value){VAL_STRING, {.string = strdup(n->name)}};
            emit_constant_operand(chunk, OP_CONST_STR, val);
            free((char*)val.as.string);
            break;
        }

        case NODE_IDENT: {
            Value name_val = (Value){VAL_STRING, {.string = strdup(n->name)}};
            emit_constant_operand(chunk, OP_GET_VAR, name_val);
            free((char*)name_val.as.string);
            break;
        }

        case NODE_UNARY: {
            compile_node(n->right, chunk);
            
            switch (n->op) {
                case TOKEN_MINUS: emit_byte(chunk, OP_SUB); break; 
                case TOKEN_BANG: emit_byte(chunk, OP_NOT); break;
                default: break;
            }
            break;
        }

        case NODE_BINOP: {
            compile_node(n->left, chunk);
            compile_node(n->right, chunk);
            
            switch (n->op) {
                case TOKEN_PLUS: emit_byte(chunk, OP_ADD); break;
                case TOKEN_MINUS: emit_byte(chunk, OP_SUB); break;
                case TOKEN_STAR: emit_byte(chunk, OP_MUL); break;
                case TOKEN_SLASH: emit_byte(chunk, OP_DIV); break;
                case TOKEN_EQUAL_EQUAL: emit_byte(chunk, OP_EQUAL); break;
                case TOKEN_GREATER: emit_byte(chunk, OP_GREATER); break;
                case TOKEN_LESS: emit_byte(chunk, OP_LESS); break;
                default: break;
            }
            break;
        }
        
        case NODE_VARDECL: { 
            compile_node(n->right, chunk); 
            
            Value name_val = (Value){VAL_STRING, {.string = strdup(n->name)}};
            emit_constant_operand(chunk, OP_DEF_GLOBAL, name_val);
            free((char*)name_val.as.string);
            break;
        }

        case NODE_ASSIGN: { 
            compile_node(n->left, chunk); 
            
            Value name_val = (Value){VAL_STRING, {.string = strdup(n->name)}};
            emit_constant_operand(chunk, OP_SET_VAR, name_val);
            free((char*)name_val.as.string);
            break;
        }
        
        case NODE_PRINT: {
            compile_node(n->right, chunk);
            emit_byte(chunk, OP_PRINT);
            break;
        }
        
        case NODE_BLOCK: {
            Node* current = n->left;
            while (current) {
                compile_node(current, chunk);
                emit_byte(chunk, OP_POP); 
                current = current->next;
            }
            break;
        }

        case NODE_IF_STMT: {
            compile_node(n->left, chunk);
            
            int then_jump_placeholder;
            emit_jump(chunk, OP_JUMP_IF_FALSE, &then_jump_placeholder);
            emit_byte(chunk, OP_POP); 
            
            compile_node(n->right->left, chunk);
            
            if (n->right->right) {
                int else_jump_placeholder;
                emit_jump(chunk, OP_JUMP, &else_jump_placeholder);
                
                patch_jump(chunk, then_jump_placeholder);
                emit_byte(chunk, OP_POP); 
                
                compile_node(n->right->right, chunk);
                patch_jump(chunk, else_jump_placeholder);
            } else {
                patch_jump(chunk, then_jump_placeholder);
                emit_byte(chunk, OP_POP); 
            }
            
            break;
        }
        
        case NODE_WHILE_STMT: {
            int loop_start = chunk->count;
            
            compile_node(n->left, chunk);
            
            int exit_jump_placeholder;
            emit_jump(chunk, OP_JUMP_IF_FALSE, &exit_jump_placeholder);
            emit_byte(chunk, OP_POP); 
            
            compile_node(n->right, chunk);
            
            emit_byte(chunk, OP_POP); 
            
            uint16_t jump_back = (uint16_t)(loop_start - chunk->count - 2); 
            emit_byte(chunk, OP_LOOP); 
            emit_byte(chunk, (uint8_t)((jump_back >> 8) & 0xff));
            emit_byte(chunk, (uint8_t)(jump_back & 0xff));
            
            patch_jump(chunk, exit_jump_placeholder);
            emit_byte(chunk, OP_POP); 
            
            break;
        }


        case NODE_FUNC_DEF:
        case NODE_FUNC_CALL:
        case NODE_RETURN_STMT:
        case NODE_CLASS_DEF:
        default:
            break;
    }
}

void compile_to_binary(Node* root, const char* path) {
    (void)root;
    (void)path;
}

bool compile(Node* program_ast, Chunk* chunk) {
    if (!program_ast) return false;
    
    Node* current = program_ast;
    while (current) {
        compile_node(current, chunk);
        current = current->next;
    }
    
    emit_byte(chunk, OP_HALT); 
    return true;
}