#ifndef DEBUG_H
#define DEBUG_H

#include <stdint.h>

/**
 * @brief print all bytecode content
 * @param code Array Instruction Byte
 * @param length Length of the array
 */
void disassemble_chunk(const char* name, uint8_t* code, int length);

/**
 * @brief print 1 insctruction
 * @return Offset for next instruction
 */
int disassemble_instruction(uint8_t* code, int offset);

#endif