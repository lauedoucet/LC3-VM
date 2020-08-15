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
    R_COND,     // conditional flags
    R_COUNT     // total registers = 10
};

uint16_t reg[R_COUNT];

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
    OP_TRAP     // execute trap
};

/*  condition flags  */
enum {
    FL_POS = 1 << 0,    // P
    FL_ZRO = 1 << 1,    // Z
    FL_NEG = 1 << 2     // N
};

/*  memory mapped registers  */
/*  trap codes  */

/*  memory storage  */
/*  register storage  */

/*  sign extension  */
uint16_t sign_extend(uint16_t x, int bit_count) {
    // checks if leftmost bit is 1
    // '& 1' retains only rightmost digit
    if ((x >> (bit_count - 1)) & 1) {
        // adds 1s to left bits (if not already there)
        x |= (0xFFFF << bit_count);
    }
    return x;
}

/*  swap  */
/*  update flags  */
void update_flags(uint16_t r) {
    // updates the flags of R_COND according to register r
    if (reg[r] == 0) {
        reg[R_COND] = FL_ZRO;
    } else if (reg[r] >> 5) {
        // leftmost bit is 1
        reg[R_COND] = FL_NEG;
    } else {
        reg[R_COND] = FL_POS;
    }
}

/*  read image file  */
/*  read image  */
/*  check key  */
/*  memory access  */
/*  input buffering  */
/*  handle interrupt  */

/*  main loop  */
int main(int argc, const char* argv[]) {
    /*  load arguments  */
    // checking the image of the VM
    if (argc < 2) {
        printf("lc3 [image-file1] ...\n");
        exit(2);
    }

    for (int i = 1; i < argc; ++i) {
        if (!read_image(argv[i])) {
            printf("failed to load image: %s\n", argv[i]);
            exit(1);
        }
    }

    /*  setup  */

    /*  set PC to default position 0x3000  */
    enum {  PC_START = 0x3000 };
    reg[R_PC] = PC_START;

    int running = 1;
    while (running) {

        /*  fetch instruction from next line from PC  */
        uint16_t instr = mem_read(reg[R_PC]++);
        uint16_t op = instr >> 12;

        switch (op) {
            case OP_ADD:
                /*  add  */ {
                    // '& 0x7' retains 3 rightmost digits
                    uint16_t DR = (instr >> 9) & 0x7;       // destination reg
                    uint16_t SR1 = (instr >> 6) & 0x7;      // first operand
                    uint16_t imm_flag = (instr >> 5) & 1;

                    if (imm_flag) {
                        // sign extends 5 bit imm into 16 bit immediate
                        uint16_t imm5 = sign_extend(instr & 0x1F, 5);
                        reg[DR] = reg[SR1] + imm5;
                    } else {
                        uint16_t SR2 = instr & 0x7;
                        reg[DR] = reg[SR1] + reg[SR2];
                    }
                    update_flags(DR);
                }
                break;
            case OP_AND:
                /*  and  */ {
                    uint16_t DR = (instr >> 9) & 0x7;
                    uint16_t SR1 = (instr >> 6) & 0x7;
                    uint16_t imm_flag = (instr >> 5) & 1;

                    if (imm_flag) {
                        uint16_t imm5 = sign_extend(instr & 0x1F, 5);
                        reg[DR] = reg[SR1] & imm5;
                    } else {
                        uint16_t SR2 = instr & 0x7;
                        reg[DR] = reg[SR1] & reg[SR2];
                    }
                    update_flags(DR);
                }
                break;
            case OP_NOT:
                /*  not  */ {
                    uint16_t DR = (instr >> 9) & 0x7;
                    uint16_t SR = (instr >> 6) & 0x7;

                    reg[DR] = ~reg[SR];
                    update_flags(DR);
                }
                break;
            case OP_BR:
                /*  branch  */ {
                    uint16_t pc_offset = sign_extend(instr & 0x1FF, 9);
                    uint16_t cond_flag = (instr >> 9) & 0x7;
                    // checks if any condition is fulfilled (n,z,p)
                    if (cond_flag & reg[R_COND]) {
                        reg[R_PC] += pc_offset;
                    }
                }
                break;
            case OP_JMP:
                /*  jump  */ {
                    // unconditional jump to register specified (o/w to R_R7)
                    uint16_t BaseR = (instr >> 6) & 0x7;
                    reg[R_PC] = reg[BaseR];
                }
                break;
            case OP_JSR:
                /*  subroutine jump  */ {
                    // save current PC into R7
                    reg[R_R7] = reg[R_PC];

                    uint16_t flag = (instr >> 11) & 1;

                    if (flag) {
                        uint16_t pc_offset = sign_extend(instr & 0x7FF, 11);
                        reg[R_PC] += pc_offset;
                    } else {
                        uint16_t BaseR = (instr >> 6) & 0x7;
                        reg[R_PC] = reg[BaseR];
                    }
                }
                break;
            case OP_LD:
                /*  load  */ {
                    uint16_t DR = (instr >> 9) & 0x7;
                    uint16_t pc_offset = sign_extend(instr & 0x1FF, 9);
                    reg[DR] = mem_read(reg[R_PC] + pc_offset);
                    update_flags(DR);
                }
                break;
            case OP_LDI:
                /*  load indirect  */ {
                    uint16_t DR = (instr >> 9) & 0x7;
                    uint16_t pc_offset = sign_extend(instr & 0x1FF, 9);
                    // offset is used as a pointer, not actual data address
                    reg[DR] = mem_read(mem_read(reg[R_PC] = pc_offset));
                    update_flags(DR);
                }
                break;
            case OP_LDR:
                /*  load register */ {
                    uint16_t DR = (instr >> 9) & 0x7;
                    uint16_t BaseR = (instr >> 6) & 0x7;
                    uint16_t offset = sign_extend(instr & 0x3F, 6);
                    reg[DR] = mem_read(reg[BaseR] + offset);
                    update_flags(DR);
                }
                break;
            case OP_LEA:
                /*  load effective address */ {
                    uint64_t DR = (instr >> 9) & 0x7;
                    uint16_t pc_offset = sign_extend(instr & 0x1FF, 9);
                    reg[DR] = reg[R_PC] + pc_offset;
                    update_flags(DR);
                }
                break;
            case OP_ST: 
                /*  store  */ {
                    uint16_t SR = (instr >> 9) & 0x7;
                    uint16_t pc_offset = sign_extend(instr & 0x1FF, 9);
                    mem_write(reg[R_PC] + pc_offset, SR);
                }
                break;
            case OP_STI:
                /*  store indirect  */ {
                    uint16_t SR = (instr >> 9) & 0x7;
                    uint16_t pc_offset = sign_extend(instr & 0x1FF, 9);
                    mem_write(mem_read(reg[R_PC] + pc_offset), SR);
                }
                break;
            case OP_STR:
                /*  store register  */ {
                    uint16_t SR = (instr >> 9) & 0x7;
                    uint16_t BaseR = (instr >> 6) & 0x7;
                    uint16_t offset = sign_extend(instr & 0x3F, 6);
                    mem_write(reg[BaseR] + offset, SR);                }
                break;
            case OP_TRAP: 
                /*  trap routines  */ {

                }
                break;
            case OP_RES:
            case OP_RTI:
            default:
                // deal with bad code or unused regs
                break;
        }
    }

    /*  shutdown  */
}