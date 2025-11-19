#include <stdio.h>
#include <string.h>
#include "../../include/vm/debug.h"
#include "../../include/vm/opcode.h"

static int simple_instruction(const char *name, int offset)
{
    printf("%-16s\n", name);
    return offset + 1;
}

static int const_num_instruction(const char *name, uint8_t *code, int offset)
{
    double value;
    memcpy(&value, code + offset + 1, sizeof(double));
    printf("%-16s %g\n", name, value);
    return offset + 1 + sizeof(double);
}

static int const_str_instruction(const char *name, uint8_t *code, int offset)
{
    int len;
    memcpy(&len, code + offset + 1, sizeof(int));

    printf("%-16s \"", name);
    for (int i = 0; i < len; i++)
    {
        printf("%c", code[offset + 1 + sizeof(int) + i]);
    }
    printf("\"\n");

    return offset + 1 + sizeof(int) + len;
}

static int jump_instruction(const char *name, int sign, uint8_t *code, int offset)
{
    uint16_t jump = (uint16_t)(code[offset + 1] << 8 | code[offset + 2]);
    printf("%-16s %4d -> %d\n", name, offset, offset + 3 + sign * jump);
    return offset + 3;
}

static int loop_instruction(const char *name, uint8_t *code, int offset)
{
    uint16_t jump = (uint16_t)(code[offset + 1] << 8 | code[offset + 2]);
    printf("%-16s %4d -> %d\n", name, offset, offset + 3 - jump);
    return offset + 3;
}

static int invoke_instruction(const char *name, uint8_t *code, int offset)
{
    offset++;

    int len;
    memcpy(&len, code + offset, sizeof(int));
    offset += sizeof(int);

    printf("%-16s \"", name);
    for (int i = 0; i < len; i++)
        printf("%c", code[offset + i]);
    printf("\"");
    offset += len;

    uint8_t args = code[offset];
    printf(" (%d args)\n", args);
    offset++;

    return offset;
}

int disassemble_instruction(uint8_t *code, int offset)
{
    printf("%04d  ", offset);

    uint8_t instruction = code[offset];

    switch (instruction)
    {
    case OP_HALT:
        return simple_instruction("OP_HALT", offset);

    case OP_ADD:
        return simple_instruction("OP_ADD", offset);
    case OP_SUB:
        return simple_instruction("OP_SUB", offset);
    case OP_MUL:
        return simple_instruction("OP_MUL", offset);
    case OP_DIV:
        return simple_instruction("OP_DIV", offset);

    case OP_POP:
        return simple_instruction("OP_POP", offset);
    case OP_DUP:
        return simple_instruction("OP_DUP", offset);

    case OP_EQUAL:
        return simple_instruction("OP_EQUAL", offset);
    case OP_GREATER:
        return simple_instruction("OP_GREATER", offset);
    case OP_LESS:
        return simple_instruction("OP_LESS", offset);
    case OP_NOT:
        return simple_instruction("OP_NOT", offset); 

    case OP_PRINT:
        return simple_instruction("OP_PRINT", offset);

    case OP_CONST_NUM:
        return const_num_instruction("OP_CONST_NUM", code, offset);
    case OP_CONST_STR:
        return const_str_instruction("OP_CONST_STR", code, offset);
    case OP_SET_VAR:
        return const_str_instruction("OP_SET_VAR", code, offset);
    case OP_GET_VAR:
        return const_str_instruction("OP_GET_VAR", code, offset);

    case OP_JUMP:
        return jump_instruction("OP_JUMP", 1, code, offset);
    case OP_JUMP_IF_FALSE:
        return jump_instruction("OP_JUMP_IF_FALSE", 1, code, offset);
    case OP_LOOP:
        return loop_instruction("OP_LOOP", code, offset);

    case OP_DEF_FUNC:
        return const_str_instruction("OP_DEF_FUNC", code, offset);
    case OP_RETURN:
        return simple_instruction("OP_RETURN", offset);
    case OP_CLASS:
        return const_str_instruction("OP_CLASS", code, offset);
    case OP_METHOD:
        return const_str_instruction("OP_METHOD", code, offset);
    case OP_GET_PROP:
        return const_str_instruction("OP_GET_PROP", code, offset);
    case OP_SET_PROP:
        return const_str_instruction("OP_SET_PROP", code, offset);

    case OP_CALL:
        return invoke_instruction("OP_CALL", code, offset);
    case OP_INVOKE:
        return invoke_instruction("OP_INVOKE", code, offset);

    default:
        printf("Unknown opcode %d\n", instruction);
        return offset + 1;
    }
}

void disassemble_chunk(const char *name, uint8_t *code, int length)
{
    printf("== %s Dump ==\n", name);

    int offset = 0;

    if (length > 3 && code[0] == 'J' && code[1] == 'L' && code[2] == 'O')
    {
        printf("Header: JLO v1 (Skipping 3 bytes)\n");
        offset = 3;
    }

    while (offset < length)
    {
        offset = disassemble_instruction(code, offset);
    }
}