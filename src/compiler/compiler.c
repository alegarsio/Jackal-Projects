#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * @include compiler file
 */
#include "../../include/compiler/compiler.h"
#include "../../include/vm/opcode.h"

static FILE *file_out;

/**
 * @brief compile node
 * @param p
 */
void compile_node(Node *n);

void emit_byte(uint8_t byte)
{
    fwrite(&byte, sizeof(uint8_t), 1, file_out);
}

void emit_double(double value)
{
    fwrite(&value, sizeof(double), 1, file_out);
}

void emit_string(const char *str)
{
    int len = strlen(str);
    fwrite(&len, sizeof(int), 1, file_out);
    fwrite(str, sizeof(char), len, file_out);
}

void emit_loop(int loop_start)
{
    emit_byte(OP_LOOP);

    int offset = ftell(file_out) - loop_start + 2;
    if (offset > 65535)
    {
        printf("Error: Loop body too large.\n");
        exit(1);
    }

    emit_byte((offset >> 8) & 0xff);
    emit_byte(offset & 0xff);
}

/**
 * @brief placehoder jump address
 * @param instruction
 */
int emit_jump(uint8_t instruction)
{
    emit_byte(instruction);
    emit_byte(0xff);
    emit_byte(0xff);
    return ftell(file_out) - 2;
}
int compile_args_reversed(Node *arg)
{
    if (!arg)
        return 0;

    int count = compile_args_reversed(arg->next);

    compile_node(arg);

    return count + 1;
}

void patch_jump(int offset)
{
    long current = ftell(file_out);
    int jump_len = current - offset - 2;

    fseek(file_out, offset, SEEK_SET);
    uint8_t high = (jump_len >> 8) & 0xff;
    uint8_t low = jump_len & 0xff;

    fwrite(&high, 1, 1, file_out);
    fwrite(&low, 1, 1, file_out);

    fseek(file_out, current, SEEK_SET);
}

void compile_node(Node *n)
{
    if (!n)
        return;

    switch (n->kind)
    {
    case NODE_BLOCK: {
            Node* stmt = n->left;
            while (stmt) {
                compile_node(stmt);
                
                
                if (stmt->kind == NODE_ASSIGN || 
                    stmt->kind == NODE_POST_INC || 
                    stmt->kind == NODE_POST_DEC || 
                    stmt->kind == NODE_FUNC_CALL) 
                {
                    emit_byte(OP_POP); 
                }
                
                stmt = stmt->next;
            }
            break;
        }

    case NODE_FUNC_DEF:
    {
        int jump = emit_jump(OP_JUMP);

        long func_start_addr = ftell(file_out);

        Node *param = n->left;
        while (param)
        {
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


        case NODE_WHEN_EXPR: {
            int exitJumps[256];
            int jumpCount = 0;

            Node* caseNode = n->left; 
            
            while (caseNode) {
                if (caseNode->kind != NODE_WHEN_CASE) {
                    caseNode = caseNode->next;
                    continue;
                }

                if (caseNode->left != NULL) {
                    compile_node(caseNode->left);

                    int nextJump = emit_jump(OP_JUMP_IF_FALSE);

                    emit_byte(OP_POP); 
                    compile_node(caseNode->right); 

                    if (jumpCount < 256) {
                        exitJumps[jumpCount++] = emit_jump(OP_JUMP);
                    }

                    patch_jump(nextJump);
                    emit_byte(OP_POP); 
                }
                
                else {
                    compile_node(caseNode->right);
                }

                caseNode = caseNode->next;
            }

            for (int i = 0; i < jumpCount; i++) {
                patch_jump(exitJumps[i]);
            }
            break;
        }

        case NODE_WHEN_CASE:
            break;

    case NODE_FUNC_CALL:
    {
        if (n->left->kind == NODE_GET)
        {
            compile_node(n->left->left);

            int arg_count = compile_args_reversed(n->right);

            emit_byte(OP_INVOKE);
            emit_string(n->left->name);
            emit_byte((uint8_t)arg_count);
        }
        else
        {
            int arg_count = compile_args_reversed(n->right);

            emit_byte(OP_CALL);
            emit_string(n->left->name);
            emit_byte((uint8_t)arg_count);
        }
        break;
    }

    case NODE_POST_INC:
    {
        if (n->left->kind != NODE_IDENT)
        {
            printf("Error: Invalid operand for ++.\n");
            exit(1);
        }

        emit_byte(OP_GET_VAR);
        emit_string(n->left->name);

        emit_byte(OP_CONST_NUM);
        emit_double(1.0);
        emit_byte(OP_ADD);

        emit_byte(OP_DUP);

        emit_byte(OP_SET_VAR);
        emit_string(n->left->name);

        break;
    }

        case NODE_POST_DEC: {
            emit_byte(OP_GET_VAR);
            emit_string(n->left->name);

            emit_byte(OP_CONST_NUM);
            emit_double(1.0);

            emit_byte(OP_SUB);

            emit_byte(OP_SET_VAR);
            emit_string(n->left->name);
            
            break;
        }

    case NODE_FOR_STMT:
    {
        Node *init = n->left;
        Node *cond = n->right->left;
        Node *incr = n->right->right->left;
        Node *body = n->right->right->right;

        if (init)
            compile_node(init);

        int loopStart = ftell(file_out);

        compile_node(cond);

        int exitJump = emit_jump(OP_JUMP_IF_FALSE);

        emit_byte(OP_POP);

        compile_node(body);

        if (incr)
        {
            compile_node(incr);
            emit_byte(OP_POP);
        }

        emit_loop(loopStart);

        patch_jump(exitJump);

        emit_byte(OP_POP);

        break;
    }

    case NODE_MATCH_STMT:
    {
        compile_node(n->left); // Push Target

        int exitJumps[256];
        int jumpCount = 0;

        Node *caseNode = n->right;

        while (caseNode)
        {
            if (caseNode->left != NULL)
            {
                emit_byte(OP_DUP);
                compile_node(caseNode->left);
                emit_byte(OP_EQUAL);

                int nextJump = emit_jump(OP_JUMP_IF_FALSE);

                emit_byte(OP_POP);
                emit_byte(OP_POP);

                compile_node(caseNode->right);

                if (jumpCount < 256)
                {
                    exitJumps[jumpCount++] = emit_jump(OP_JUMP);
                }

                patch_jump(nextJump);

                emit_byte(OP_POP);
            }
            else
            {
                emit_byte(OP_POP);
                compile_node(caseNode->right);
            }

            caseNode = caseNode->next;
        }

        emit_byte(OP_POP);

        for (int i = 0; i < jumpCount; i++)
        {
            patch_jump(exitJumps[i]);
        }
        break;
    }

    case NODE_CLASS_DEF:
    {
        emit_byte(OP_CLASS);
        emit_string(n->name);
        Node *method = n->left;
        while (method)
        {

            int jump = emit_jump(OP_JUMP);
            long func_start = ftell(file_out);

            Node *param = method->left;
            while (param)
            {
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

    case NODE_THIS:
        emit_byte(OP_GET_VAR);
        emit_string("this");
        break;

    case NODE_GET:
    {
        compile_node(n->left);
        emit_byte(OP_GET_PROP);
        emit_string(n->name);
        break;
    }

    case NODE_SET:
    {

        compile_node(n->left->left);
        compile_node(n->right);
        emit_byte(OP_SET_PROP);
        emit_string(n->left->name);
        break;
    }

    case NODE_RETURN_STMT:
        if (n->left)
            compile_node(n->left);
        else
            emit_byte(OP_NIL);
        emit_byte(OP_RETURN);
        break;

    case NODE_PRINT:
        compile_node(n->right);
        emit_byte(OP_PRINT);
        break;

    case NODE_IF_STMT:
    {
        compile_node(n->left);

        int thenJump = emit_jump(OP_JUMP_IF_FALSE);

        emit_byte(OP_POP);

        compile_node(n->right->left);

        int elseJump = emit_jump(OP_JUMP);

        patch_jump(thenJump);

        emit_byte(OP_POP);

        if (n->right->right)
        {
            compile_node(n->right->right);
        }

        patch_jump(elseJump);
        break;
    }

    case NODE_WHILE_STMT: {
        
            int loopStart = ftell(file_out);

            compile_node(n->left);
            int exitJump = emit_jump(OP_JUMP_IF_FALSE);
            emit_byte(OP_POP); 

            compile_node(n->right); 
            
            if (n->right->kind != NODE_BLOCK) {
                emit_byte(OP_POP); 
            }

            emit_loop(loopStart);

            patch_jump(exitJump);
            emit_byte(OP_POP); 
            break;
        }

    case NODE_ASSIGN:
        compile_node(n->left);

        emit_byte(OP_DUP);

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

    case NODE_BINOP: {
            if (n->op == TOKEN_AND_AND) {
                compile_node(n->left);
                int endJump = emit_jump(OP_JUMP_IF_FALSE); 
                emit_byte(OP_POP); 
                compile_node(n->right); 
                patch_jump(endJump); 
                return;
            }

            if (n->op == TOKEN_PIPE_PIPE) {
                compile_node(n->left);
                int elseJump = emit_jump(OP_JUMP_IF_FALSE);
                int endJump = emit_jump(OP_JUMP); 
                
                patch_jump(elseJump);
                emit_byte(OP_POP); 
                compile_node(n->right);
                
                patch_jump(endJump);
                return;
            }

            compile_node(n->left);
            compile_node(n->right);

            if (n->op == TOKEN_PLUS) emit_byte(OP_ADD);
            if (n->op == TOKEN_MINUS) emit_byte(OP_SUB);
            if (n->op == TOKEN_STAR) emit_byte(OP_MUL);
            if (n->op == TOKEN_SLASH) emit_byte(OP_DIV);
            
            if (n->op == TOKEN_EQUAL_EQUAL) emit_byte(OP_EQUAL);
            if (n->op == TOKEN_GREATER) emit_byte(OP_GREATER);
            if (n->op == TOKEN_LESS) emit_byte(OP_LESS);
            
            if (n->op == TOKEN_GREATER_EQUAL) {
                emit_byte(OP_LESS); 
                emit_byte(OP_NOT);
            }
            if (n->op == TOKEN_LESS_EQUAL) {
                emit_byte(OP_GREATER);
                emit_byte(OP_NOT);
            }

            if (n->op == TOKEN_BANG_EQUAL) {
                emit_byte(OP_EQUAL);
                emit_byte(OP_NOT);
            }
            break;
        }

    default:

        break;
    }
}

/**
 * @brief Compile code to binary
 * @param ast
 * @param filename
 */
void compile_to_binary(Node *ast, const char *filename)
{
    file_out = fopen(filename, "wb");
    if (!file_out)
    {
        printf("Error: Cannot open file %s for writing.\n", filename);
        return;
    }

    uint8_t magic[] = {'J', 'L', 0x01};
    fwrite(magic, 1, 3, file_out);

    Node *curr = ast;
    while (curr)
    {
        compile_node(curr);
        curr = curr->next;
    }

    emit_byte(OP_HALT);
    fclose(file_out);
    printf("[Compiler] Successfully created '%s'\n", filename);
}
