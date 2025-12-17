#include "vm/vm.h"
#include "vm/opcode.h"
#include "common.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

static void runtimeError(VM *vm, const char *format, ...);
// --- Stack Management Macros ---

// Membaca 1 byte (Opcode atau 1st operand) dan memajukan IP
#define READ_BYTE() (*vm->ip++)

// Membaca 2 byte operand (index/offset)
#define READ_SHORT() \
    (vm->ip += 2, (uint16_t)((vm->ip[-2] << 8) | vm->ip[-1]))

// Mengambil konstanta dari Constant Pool
#define READ_CONSTANT() (vm->chunk->constants.values[READ_SHORT()])

// Macro untuk mengulang loop eksekusi binary (Pop 2, Lakukan Op, Push 1)
#define BINARY_OP(op_token, op)                                                \
    do                                                                         \
    {                                                                          \
        Value b = pop(vm);                                                     \
        Value a = pop(vm);                                                     \
        if (a.type != VAL_NUMBER || b.type != VAL_NUMBER)                      \
        {                                                                      \
            runtimeError(vm, "Operands for '" #op_token "' must be numbers."); \
            return INTERPRET_RUNTIME_ERROR;                                    \
        }                                                                      \
        push(vm, (Value){VAL_NUMBER, {.number = a.as.number op b.as.number}}); \
    } while (0)

// Macro untuk mengulang loop eksekusi perbandingan (Pop 2, Lakukan Comp, Push Boolean)
#define COMPARE_OP(op)                                                                       \
    do                                                                                       \
    {                                                                                        \
        Value b = pop(vm);                                                                   \
        Value a = pop(vm);                                                                   \
        if (a.type != VAL_NUMBER || b.type != VAL_NUMBER)                                    \
        {                                                                                    \
            runtimeError(vm, "Comparison operands must be numbers.");                        \
            return INTERPRET_RUNTIME_ERROR;                                                  \
        }                                                                                    \
        push(vm, (Value){VAL_NUMBER, {.number = (a.as.number op b.as.number) ? 1.0 : 0.0}}); \
    } while (0)

// --- Fungsi Utilitas VM ---

static void push(VM *vm, Value value)
{
    if (vm->stackTop - vm->stack >= STACK_MAX)
    {
        runtimeError(vm, "Stack overflow.");
        return;
    }
    *vm->stackTop = value;
    vm->stackTop++;
}

static Value pop(VM *vm)
{
    vm->stackTop--;
    return *vm->stackTop;
}

static void runtimeError(VM *vm, const char *format, ...)
{
    va_list args;
    va_start(args, format);
    vfprintf(stderr, "Runtime Error: ", args);
    vfprintf(stderr, format, args);
    va_end(args);
    fprintf(stderr, "\n");

    // Tunjukkan di mana error terjadi (offset)
    size_t instruction = vm->ip - vm->chunk->code - 1;
    fprintf(stderr, "[Bytecode offset %zu]\n", instruction);
}

// --- Implementasi VM ---

void initVM(VM *vm)
{
    vm->chunk = NULL;
    vm->ip = NULL;
    vm->stackTop = vm->stack;
    vm->globalEnv = env_new(NULL); // Inisialisasi Environment Global
}

void freeVM(VM *vm)
{
    env_free(vm->globalEnv);
}

InterpretResult interpret(VM *vm, Chunk *chunk)
{
    vm->chunk = chunk;
    vm->ip = chunk->code;

    // --- Dispatch Loop (Mesin Eksekusi Utama) ---
    for (;;)
    {
        uint8_t instruction = READ_BYTE();

        switch (instruction)
        {

        case OP_HALT:
            return INTERPRET_OK;

        case OP_CONST_NUM:
        {
            Value constant = READ_CONSTANT(); // Membaca 2-byte index
            push(vm, constant);
            break;
        }
        case OP_CONST_STR:
        {
            Value constant = READ_CONSTANT(); // Membaca 2-byte index
            push(vm, constant);
            break;
        }

        case OP_NIL:
            push(vm, (Value){VAL_NIL, {0}});
            break;

        case OP_POP:
        {
            pop(vm);
            break;
        }
        case OP_DUP:
        {
            Value value = pop(vm);
            push(vm, copy_value(value)); // Duplikasi
            push(vm, value);
            break;
        }

        case OP_ADD:
            BINARY_OP(+, +);
            break;
        case OP_SUB:
            BINARY_OP(-, -);
            break;
        case OP_MUL:
            BINARY_OP(*, *);
            break;
        case OP_DIV:
        {
            Value b = pop(vm);
            Value a = pop(vm);
            if (a.type != VAL_NUMBER || b.type != VAL_NUMBER)
            {
                runtimeError(vm, "Operands for '/' must be numbers.");
                return INTERPRET_RUNTIME_ERROR;
            }
            if (b.as.number == 0)
            {
                runtimeError(vm, "Division by zero.");
                return INTERPRET_RUNTIME_ERROR;
            }
            push(vm, (Value){VAL_NUMBER, {.number = a.as.number / b.as.number}});
            break;
        }

        case OP_NEGATE:
        {
            Value val = pop(vm);
            if (val.type != VAL_NUMBER)
            {
                runtimeError(vm, "Operand for negate must be a number.");
                return INTERPRET_RUNTIME_ERROR;
            }
            push(vm, (Value){VAL_NUMBER, {.number = -val.as.number}});
            break;
        }

        case OP_GREATER:
            COMPARE_OP(>);
            break;
        case OP_LESS:
            COMPARE_OP(<);
            break;

        case OP_EQUAL:
        {
            Value b = pop(vm);
            Value a = pop(vm);
            Value result = eval_equals(a, b);
            push(vm, result);
            break;
        }
        case OP_NOT:
        {
            Value val = pop(vm);
            bool is_truthy = is_value_truthy(val);
            push(vm, (Value){VAL_NUMBER, {.number = is_truthy ? 0.0 : 1.0}});
            break;
        }

        case OP_PRINT:
        {
            Value val_to_print = pop(vm);

            print_value(val_to_print);

            printf("\n");
            fflush(stdout);

            break;
        }

        case OP_JUMP:
        {
            uint16_t offset = READ_SHORT();
            vm->ip += offset;
            break;
        }

        case OP_JUMP_IF_FALSE:
        {
            uint16_t offset = READ_SHORT();
            Value condition = pop(vm);
            if (!is_value_truthy(condition))
            {
                vm->ip += offset;
            }
            break;
        }

            // ... Implementasi OpCode lainnya (OP_GET_VAR, OP_CALL, OP_CLASS, dll) di sini

        default:
            runtimeError(vm, "Unknown opcode: %d", instruction);
            return INTERPRET_RUNTIME_ERROR;
        }
    }
}

static Value read_constant_from_binary(FILE *f)
{
    uint8_t type_byte;
    if (fread(&type_byte, sizeof(uint8_t), 1, f) != 1)
    {
        fprintf(stderr, "Error reading constant type.\n");
        return (Value){VAL_NIL, {0}};
    }

    ValueType type = (ValueType)type_byte;

    switch (type)
    {
    case VAL_NUMBER:
    {
        double num;
        if (fread(&num, sizeof(double), 1, f) != 1)
        {
            fprintf(stderr, "Error reading number constant.\n");
            return (Value){VAL_NIL, {0}};
        }
        return (Value){VAL_NUMBER, {.number = num}};
    }
    case VAL_STRING:
    {
        int len;
        if (fread(&len, sizeof(int), 1, f) != 1)
        {
            fprintf(stderr, "Error reading string length.\n");
            return (Value){VAL_NIL, {0}};
        }
        char *str = malloc(len + 1);
        if (!str)
        {
            fprintf(stderr, "Out of memory for string constant.\n");
            return (Value){VAL_NIL, {0}};
        }
        if (fread(str, 1, len, f) != (size_t)len)
        {
            fprintf(stderr, "Error reading string content.\n");
            free(str);
            return (Value){VAL_NIL, {0}};
        }
        str[len] = '\0';
        return (Value){VAL_STRING, {.string = str}};
    }
    default:
        fprintf(stderr, "Unknown value type in binary file: %d\n", type);
        return (Value){VAL_NIL, {0}};
    }
}

void run_binary(const char *path)
{
    FILE *f = fopen(path, "rb");
    if (!f)
    {
        perror("Failed to open compiled file");
        return;
    }

    Chunk chunk;
    initChunk(&chunk);
    bool success = false;

    // --- 1. Deserialize Code Array ---
    int code_size;
    if (fread(&code_size, sizeof(int), 1, f) != 1)
    {
        fprintf(stderr, "Error reading code size from binary file.\n");
        goto cleanup;
    }

    if (code_size > 0)
    {
        chunk.code = malloc(code_size);
        if (!chunk.code)
        {
            fprintf(stderr, "Out of memory for bytecode.\n");
            goto cleanup;
        }
        if (fread(chunk.code, sizeof(uint8_t), code_size, f) != (size_t)code_size)
        {
            fprintf(stderr, "Error reading bytecode from binary file.\n");
            free(chunk.code);
            goto cleanup;
        }
        chunk.count = code_size;
        chunk.capacity = code_size;
    }

    // --- 2. Deserialize Constant Pool ---
    int constant_count;
    if (fread(&constant_count, sizeof(int), 1, f) != 1)
    {
        fprintf(stderr, "Error reading constant count from binary file.\n");
        goto cleanup;
    }

    for (int i = 0; i < constant_count; i++)
    {
        Value constant = read_constant_from_binary(f);
        if (constant.type == VAL_NIL && i < constant_count)
        {
            // Asumsi VAL_NIL hanya dikembalikan saat error membaca
            goto cleanup;
        }
        addConstant(&chunk, constant); // Tambahkan ke Chunk
        free_value(constant);          // addConstant sudah menyalin nilainya, jadi bebaskan yang sementara
    }

    // --- 3. Execute VM ---
    VM vm;
    initVM(&vm);

    InterpretResult result = interpret(&vm, &chunk);

    if (result != INTERPRET_OK)
    {
        fprintf(stderr, "Jackal VM exited with an error during binary execution.\n");
    }
    else
    {
        success = true;
    }

cleanup:
    freeChunk(&chunk);
    fclose(f);
    // Note: freeVM tidak dipanggil di sini karena globalEnv dikelola di main
}