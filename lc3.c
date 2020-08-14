#include <stdio.h>      // FILE
#include <stdint.h>     // uint16_t
#include <signal.h>     // SIGINT
#include <Windows.h>
#include <conio.h>      //_kbhit

HANDLE hStdin = INVALID_HANDLE_VALUE;

/*  registers  */
enum {
    R_R0 = 0,
    R_R1,
    R_R2,
    R_R3,
    R_R4,
    R_R5,
    R_R6, 
    R_R7, 
    R_PC,       // program counter
    R_COUND,    // conditional flags
    R_COUNT     // total registers = 10
};

/*  opcodes  */
enum {
    OP_BR = 0,  // branch
    OP_ADD,     // add
    OP_LD,      // load
    OP_ST,      // store
    OP_JSR,     // jump register
    OP_AND,     // bit AND
    OP_LDR,     // load register
    OP_STR,     // store register
    OP_RTI,     // unused
    OP_NOT,     // bit NOT
    OP_LDI,     // load indirect
    OP_STI,     // store indirect
    OP_JMP,     // jump
    OP_RES,     // reserved (unused)
    OP_LEA,     // load effective address
    OP_TRAP    // execute trap
};

/*  condition flags  */
enum {
    FL_POS = 1 << 0,    // P
    FL_ZRO = 1 << 1,    // Z
    FL_NEG = 1 << 2     // N
};
