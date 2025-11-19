#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * @include compiler file
 */
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

/**
 * @brief placehoder jump address
 * @param instruction
 */
int emit_jump(uint8_t instruction) {
    emit_byte(instruction);
    emit_byte(0xff); 
    emit_byte(0xff); 
    return ftell(file_out) - 2; 
}


void patch_jump(int offset) {
    long current = ftell(file_out); 
    int jump_len = current - offset - 2;

    fseek(file_out, offset, SEEK_SET);
    uint8_t high = (jump_len >> 8) & 0xff;
    uint8_t low = jump_len & 0xff;
    
    fwrite(&high, 1, 1, file_out);
    fwrite(&low, 1, 1, file_out);
    
    fseek(file_out, current, SEEK_SET); 
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

        case NODE_FUNC_DEF: {
            int jump = emit_jump(OP_JUMP);

            long func_start_addr = ftell(file_out); 
            
            Node* param = n->left; 
            while (param) {
                emit_byte(OP_SET_VAR);
                emit_string(param->name);
                param = param->next;
            }

            compile_node(n->right);

            emit_byte(OP_NIL); 
            emit_byte(OP_RETURN);

            patch_jump(jump);

            emit_byte(OP_DEF_FUNC);
            emit_string(n->name); 
            
            emit_byte(OP_CONST_NUM);
            emit_double((double)func_start_addr); 

            break;
        }


        case NODE_FUNC_CALL: {
            Node* arg = n->right;
            int arg_count = 0;
            while (arg) {
                compile_node(arg);
                arg_count++;
                arg = arg->next;
            }

            emit_byte(OP_CALL);
            emit_string(n->left->name); 
            emit_byte((uint8_t)arg_count); 
            break;
        }


        case NODE_CLASS_DEF: {
            emit_byte(OP_CLASS);
            emit_string(n->name);
            Node* method = n->left;
            while (method) {
                
                int jump = emit_jump(OP_JUMP);
                long func_start = ftell(file_out);

                Node* param = method->left;
                while (param) {
                    emit_byte(OP_SET_VAR);
                    emit_string(param->name);
                    param = param->next;
                }
                compile_node(method->right); 
                
                emit_byte(OP_NIL);
                emit_byte(OP_RETURN);
                patch_jump(jump);

                emit_byte(OP_METHOD);
                emit_string(method->name);
               
                emit_byte(OP_CONST_NUM);
                emit_double((double)func_start);

                method = method->next;
            }
            break;
        }

        case NODE_GET: {
            compile_node(n->left); 
            emit_byte(OP_GET_PROP);
            emit_string(n->name);  
            break;
        }

        case NODE_SET: {
          
            compile_node(n->left->left); 
            compile_node(n->right);      
            emit_byte(OP_SET_PROP);
            emit_string(n->left->name);  
            break;
        }


        case NODE_RETURN_STMT:
            if (n->left) compile_node(n->left); 
            else emit_byte(OP_NIL); 
            emit_byte(OP_RETURN);
            break;

        case NODE_PRINT:
            compile_node(n->right);
            emit_byte(OP_PRINT);
            break;

        
        case NODE_ASSIGN:
            compile_node(n->left);  
            emit_byte(OP_SET_VAR);  
            emit_string(n->name);   
            break;
        
        case NODE_IDENT:
            emit_byte(OP_GET_VAR);  
            emit_string(n->name);   
        break;

        case NODE_VARDECL:
            compile_node(n->right); 
            emit_byte(OP_SET_VAR);  
            emit_string(n->name);   
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
            
            break;
    }
}


/**
 * @brief Compile code to binary
 * @param ast
 * @param filename
 */
void compile_to_binary(Node* ast, const char* filename) {
    file_out = fopen(filename, "wb");
    if (!file_out) {
        printf("Error: Cannot open file %s for writing.\n", filename);
        return;
    }

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