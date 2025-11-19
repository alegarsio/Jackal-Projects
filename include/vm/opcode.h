#ifndef OPCODE_H
#define OPCODE_H

#include <stdint.h>

/**
 * this enum contains the opcode for jackal compiler mode
 * @typedef @enum OpCode
 */
typedef enum {
    OP_HALT,        
    OP_CONST_NUM,  
    OP_CONST_STR,   
    OP_ADD,         
    OP_SUB,        
    OP_MUL,         
    OP_DIV,         
    OP_PRINT,       
    OP_POP,        
    OP_GET_VAR,     
    OP_SET_VAR,

    OP_NIL,
    
    OP_JUMP,        
    OP_JUMP_IF_FALSE, 
    OP_DEF_FUNC,   
    OP_CALL,        
    OP_RETURN,

    /**
     * OP for class
     */
    OP_CLASS,       
    OP_METHOD,      
    OP_GET_PROP,   
    OP_SET_PROP,
    OP_INVOKE,

    /**
     * OP Code for match
     */
    OP_DUP,     
    OP_EQUAL,

    /**
     * OP For loop
     */

    OP_LOOP,

    /**
     * OP For > and < 
     */

    OP_GREATER,
    OP_LESS,

    /**
     * OP For Not
     */

    OP_NOT
         
} OpCode;

#endif