#ifndef JACKAL_VM_H
#define JACKAL_VM_H

#include "vm/chunk.h"
#include "env.h"
#include "value.h"

// Kita perlu ukuran tumpukan tetap (fixed stack size) untuk kesederhanaan
#define STACK_MAX 256

/**
 * @typedef @struct VM
 * Represents the Jackal Virtual Machine structure.
 */
typedef struct {
    Chunk* chunk;     
    uint8_t* ip;      // Instruction Pointer: menunjuk ke byte instruksi berikutnya yang akan dieksekusi.
    
    Value stack[STACK_MAX]; // Execution Stack: array tempat operan disimpan.
    Value* stackTop;        // Pointer ke lokasi di atas nilai teratas di tumpukan.
    
    // Environment global untuk menyimpan variabel global/fungsi
    Env* globalEnv; 

} VM;

/**
 * @typedef @enum InterpretResult
 * Represents the result of the interpretation process.
 */
typedef enum {
    INTERPRET_OK,
    INTERPRET_COMPILE_ERROR,
    INTERPRET_RUNTIME_ERROR
} InterpretResult;


void initVM(VM* vm);
void freeVM(VM* vm);
InterpretResult interpret(VM* vm, Chunk* chunk);
void run_binary(const char* path);
#endif