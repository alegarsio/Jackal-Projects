#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../../include/vm/vm.h"
#include "../../include/vm/opcode.h"
#include "../../include/value.h"
#include "../../include/env.h"

#include "../../include/env.h"

#define STACK_MAX 256
#define FRAMES_MAX 64

typedef struct
{
    uint8_t *return_ip;
    Env *env_snap;
} CallFrame;

static Value stack[STACK_MAX];
static Value *sp = stack;
static Env *current_env;

static CallFrame frames[FRAMES_MAX];
static int frame_count = 0;

static uint8_t *code;
static uint8_t *ip;

static void push(Value v) { *sp++ = v; }
static Value pop() { return *--sp; }

static uint8_t read_byte() { return *ip++; }

static double read_double()
{
    double val;
    memcpy(&val, ip, sizeof(double));
    ip += sizeof(double);
    return val;
}

static Value peek(int distance)
{
    return sp[-1 - distance];
}

static int is_falsey(Value v)
{
    if (v.type == VAL_NIL)
        return 1;

    if (v.type == VAL_NUMBER)
        return v.as.number == 0.0;

    return 0;
}

static int values_equal(Value a, Value b)
{
    if (a.type != b.type)
        return 0;
    switch (a.type)
    {
    case VAL_NIL:
        return 1;
    case VAL_NUMBER:
        return a.as.number == b.as.number;
    case VAL_STRING:
        return strcmp(a.as.string, b.as.string) == 0;
    default:
        return 0;
    }
}

static uint16_t read_short()
{
    uint8_t high = read_byte();
    uint8_t low = read_byte();
    return (high << 8) | low;
}

static char *read_string()
{
    int len;
    memcpy(&len, ip, sizeof(int));
    ip += sizeof(int);

    char *str = malloc(len + 1);
    memcpy(str, ip, len);
    str[len] = '\0';
    ip += len;
    return str;
}

long get_func_addr(const char *name)
{
    Var *v = find_var(current_env, name);
    if (v && v->value.type == VAL_NUMBER)
    {
        return (long)v->value.as.number;
    }
    return -1;
}

void run_binary(const char *filename)
{
    FILE *f = fopen(filename, "rb");
    if (!f)
        return;

    fseek(f, 0, SEEK_END);
    long fsize = ftell(f);
    fseek(f, 0, SEEK_SET);

    code = malloc(fsize);
    fread(code, 1, fsize, f);
    fclose(f);

    ip = code;
    if (code[0] != 'J' || code[1] != 'L')
    {
        printf("Invalid .jlo file\n");
        return;
    }
    ip += 3;

    current_env = env_new(NULL);
    sp = stack;

    frame_count = 0;

    while (ip < code + fsize)
    {
        uint8_t opcode = read_byte();

        switch (opcode)
        {
        case OP_HALT:
            goto end_vm;

        case OP_JUMP:
        {
            uint8_t high = read_byte();
            uint8_t low = read_byte();
            uint16_t offset = (high << 8) | low;
            ip += offset;
            break;
        }
        case OP_DUP:
        {

            push(peek(0));
            break;
        }

        case OP_EQUAL:
        {
            Value b = pop();
            Value a = pop();

            int res = 0;
            if (a.type == b.type)
            {
                if (a.type == VAL_NUMBER)
                    res = (a.as.number == b.as.number);
                else if (a.type == VAL_NIL)
                    res = 1;
                else if (a.type == VAL_STRING)
                    res = (strcmp(a.as.string, b.as.string) == 0);
            }
            push((Value){VAL_NUMBER, {.number = (double)res}});
            break;
        }

        case OP_NOT:
        {
            Value v = pop();

            push((Value){VAL_NUMBER, {.number = (double)is_falsey(v)}});
            break;
        }

        case OP_JUMP_IF_FALSE:
        {
            uint16_t offset = read_short();

            if (sp == stack)
            {
                printf("Runtime Error: Stack Underflow in Condition Check!\n");
                goto end_vm;
            }

            Value condition = *(sp - 1);

            if (is_falsey(condition))
            {
                ip += offset;
            }
            break;
        }

        case OP_POP:
        {
            pop();
            break;
        }

        case OP_CLASS:
        {
            char *name = read_string();

            Class *klass = malloc(sizeof(Class));
            strcpy(klass->name, name);
            klass->methods = env_new(NULL);

            Value val = (Value){VAL_CLASS, {.class_obj = klass}};

            set_var(current_env, name, val, false);

            push(val);

            free(name);
            break;
        }

        case OP_METHOD:
        {
            char *methodName = read_string();

            read_byte();
            double addr = read_double();

            Value classVal = *(sp - 1);

            if (classVal.type != VAL_CLASS)
            {
                printf("Error: OP_METHOD called without class on stack.\n");
                goto end_vm;
            }

            Value methodVal = (Value){VAL_FUNCTION, {0}};

            methodVal.type = VAL_NUMBER;
            methodVal.as.number = addr;

            set_var(classVal.as.class_obj->methods, methodName, methodVal, false);

            free(methodName);
            break;
        }

        case OP_INVOKE:
        {
            char *methodName = read_string();
            uint8_t arg_count = read_byte();

            Value instanceVal = *(sp - 1 - arg_count);

            if (instanceVal.type != VAL_INSTANCE)
            {
                printf("Error: Only instances have methods. Got type %d\n", instanceVal.type);
                goto end_vm;
            }

            Instance *inst = instanceVal.as.instance;
            Class *klass = inst->class_val->as.class_obj;

            Var *method = find_var(klass->methods, methodName);
            if (!method)
            {
                printf("Error: Method '%s' not found in class '%s'.\n", methodName, klass->name);
                goto end_vm;
            }

            if (frame_count == FRAMES_MAX)
            {
                printf("Stack Overflow\n");
                goto end_vm;
            }

            frames[frame_count].return_ip = ip;
            frames[frame_count].env_snap = current_env;
            frame_count++;

            current_env = env_new(current_env);

            set_var(current_env, "this", instanceVal, false);

            long addr = (long)method->value.as.number;
            ip = code + addr;

            free(methodName);
            break;
        }

        case OP_GREATER:
        {
            Value b = pop();
            Value a = pop();

            if (a.type == VAL_NUMBER && b.type == VAL_NUMBER)
            {
                push((Value){VAL_NUMBER, {.number = (a.as.number > b.as.number)}});
            }
            else
            {
                push((Value){VAL_NUMBER, {.number = 0}});
            }
            break;
        }
        case OP_LESS:
        {
            Value b = pop();
            Value a = pop();
            if (a.type == VAL_NUMBER && b.type == VAL_NUMBER)
            {
                push((Value){VAL_NUMBER, {.number = (a.as.number < b.as.number)}});
            }
            else
            {
                push((Value){VAL_NUMBER, {.number = 0}});
            }
            break;
        }

        case OP_DEF_FUNC:
        {
            char *name = read_string();
            read_byte();
            double addr = read_double();
            set_var(current_env, name, (Value){VAL_NUMBER, {.number = addr}}, false);
            free(name);
            break;
        }

        case OP_LOOP:
        {
            uint16_t offset = read_short();
            ip -= offset;
            break;
        }

        case OP_CALL:
        {
            char *name = read_string();
            uint8_t arg_count = read_byte();

            Var *v = find_var(current_env, name);

            if (v && v->value.type == VAL_CLASS)
            {
                Class *klass = v->value.as.class_obj;

                Instance *inst = malloc(sizeof(Instance));
                inst->class_val = malloc(sizeof(Value));
                *inst->class_val = v->value;
                inst->fields = env_new(NULL);

                Value instVal = (Value){VAL_INSTANCE, {.instance = inst}};

                Var *initMethod = find_var(klass->methods, "init");

                if (initMethod)
                {
                    if (frame_count == FRAMES_MAX)
                    {
                        printf("Stack Overflow\n");
                        goto end_vm;
                    }
                    frames[frame_count].return_ip = ip;
                    frames[frame_count].env_snap = current_env;
                    frame_count++;

                    current_env = env_new(current_env);

                    set_var(current_env, "this", instVal, false);

                    long addr = (long)initMethod->value.as.number;
                    ip = code + addr;
                }
                else
                {
                    push(instVal);
                }

                free(name);
                break;
            }

            long addr = get_func_addr(name);

            if (addr == -1)
            {
                printf("Runtime Error: Function or Class '%s' not found.\n", name);
                goto end_vm;
            }

            if (frame_count == FRAMES_MAX)
            {
                printf("Stack Overflow\n");
                goto end_vm;
            }
            frames[frame_count].return_ip = ip;
            frames[frame_count].env_snap = current_env;
            frame_count++;

            current_env = env_new(current_env);

            // 3. Lompat
            ip = code + addr;

            free(name);
            break;
        }

        case OP_GET_PROP:
        {
            char *propName = read_string();
            Value obj = pop();

            if (obj.type == VAL_INSTANCE)
            {
                Var *field = find_var(obj.as.instance->fields, propName);
                if (field)
                {
                    push(field->value);
                }

                else
                {
                    printf("Error: Property/Method '%s' not found.\n", propName);
                    goto end_vm;
                }
            }
            else
            {
                printf("Error: Not an instance.\n");
                goto end_vm;
            }
            free(propName);
            break;
        }

        case OP_SUB:
        {
            Value b = pop();
            Value a = pop();

            if (a.type == VAL_NUMBER && b.type == VAL_NUMBER)
            {
                push((Value){VAL_NUMBER, {.number = a.as.number - b.as.number}});
            }
            else
            {
                printf("Runtime Error: Operands for - must be numbers.\n");
                goto end_vm;
            }
            break;
        }

        case OP_SET_PROP:
        {
            char *propName = read_string();
            Value val = pop();
            Value obj = pop();

            if (obj.type == VAL_INSTANCE)
            {

                set_var(obj.as.instance->fields, propName, val, false);
                push(val);
            }
            else
            {
                printf("Error: Only instances have fields.\n");
                goto end_vm;
            }
            free(propName);
            break;
        }

        case OP_RETURN:
        {
            Value retVal = pop();
            if (frame_count == 0)
                goto end_vm;

            frame_count--;
            ip = frames[frame_count].return_ip;

            current_env = frames[frame_count].env_snap;

            push(retVal);
            break;
        }

        case OP_CONST_NUM:
            push((Value){VAL_NUMBER, {.number = read_double()}});
            break;
        case OP_CONST_STR:
        {
            char *s = read_string();
            push((Value){VAL_STRING, {.string = s}});
            break;
        }

        case OP_MUL:
        {
            Value b = pop();
            Value a = pop();
            if (a.type == VAL_NUMBER && b.type == VAL_NUMBER)
            {
                push((Value){VAL_NUMBER, {.number = a.as.number * b.as.number}});
            }
            else
            {
                printf("Runtime Error: Operands for * must be numbers.\n");
                goto end_vm;
            }
            break;
        }
        case OP_ADD:
        {
            Value b = pop();
            Value a = pop();

            if (a.type == VAL_NUMBER && b.type == VAL_NUMBER)
            {
                push((Value){VAL_NUMBER, {.number = a.as.number + b.as.number}});
            }
            else if (a.type == VAL_STRING && b.type == VAL_STRING)
            {

                int lenA = strlen(a.as.string);
                int lenB = strlen(b.as.string);

                char *result = malloc(lenA + lenB + 1);

                strcpy(result, a.as.string);
                strcat(result, b.as.string);

                push((Value){VAL_STRING, {.string = result}});
            }
            else if (a.type == VAL_STRING && b.type == VAL_NUMBER)
            {
                char numStr[32];
                snprintf(numStr, 32, "%g", b.as.number);

                int lenA = strlen(a.as.string);
                int lenB = strlen(numStr);
                char *result = malloc(lenA + lenB + 1);

                strcpy(result, a.as.string);
                strcat(result, numStr);

                push((Value){VAL_STRING, {.string = result}});
            }
            else
            {
                printf("Runtime Error: Invalid operand types for +\n");
                goto end_vm;
            }
            break;
        }

        case OP_DIV:
        {
            Value b = pop();
            Value a = pop();
            if (a.type == VAL_NUMBER && b.type == VAL_NUMBER)
            {
                if (b.as.number == 0)
                {
                    printf("Runtime Error: Division by zero.\n");
                    goto end_vm;
                }
                push((Value){VAL_NUMBER, {.number = a.as.number / b.as.number}});
            }
            else
            {
                printf("Runtime Error: Operands for / must be numbers.\n");
                goto end_vm;
            }
            break;
        }
        case OP_PRINT:
        {
            print_value(pop());
            printf("\n");
            break;
        }
        case OP_SET_VAR:
        {
            char *name = read_string();
            Value val = pop();

            if (assign_var(current_env, name, val))
            {
            }
            else
            {
                set_var(current_env, name, val, false);
            }

            push(val);

            free(name);
            break;
        }
        case OP_GET_VAR:
        {
            char *name = read_string();
            Var *v = find_var(current_env, name);

            if (v)
            {
                push(v->value);
            }
            else
            {
                printf("Runtime Error: Undefined variable '%s'\n", name);
                goto end_vm;
            }
            free(name);
            break;
        }
        }
    }

end_vm:
    free(code);
    env_free(current_env);
}