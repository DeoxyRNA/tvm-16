#pragma once

#if defined(_WIN32)
#include <windows.h>
#else
#include <unistd.h>
#endif

#include <stdint.h>

/// an instruction opcode.
typedef enum opcode {
    /* system */
    NOP, // no operation
    LDW, // load word
    LDB, // load byte
    STW, // store word
    STB, // store byte
    PSW, // push word
    PSB, // push byte
    PPW, // pop word
    PPB, // pop byte
    SYS, // system call
    HLT, // halt

    /* branch */
    BRC, // branch
    BEQ, // branch if equal
    BNE, // branch if not equal
    BMI, // branch if minus
    BPL, // branch if plus
    BCA, // branch if carry
    BNC, // branch if not carry
    BOV, // branch if overflow
    BNO, // branch if not overflow
    CLS, // call subroutine
    RET, // return from subroutine

    /* arithmetic & logic */
    ADD, // add
    ADS, // add signed
    ADC, // add with carry
    SUB, // subtract
    SBS, // subtract signed
    SBB, // subtract with borrow
    AND, // bitwise AND
    IOR, // bitwise inclusive OR
    XOR, // bitwise exclusive OR
    NOT, // bitwise NOT
    SHL, // bitwise shift left
    SHR, // bitwise shift right
    CMP, // compare
    CPS, // compare signed
} opcode_t;

#pragma pack(push, 1)
/// a tvm-16 cpu instruction, occupies 5 bytes
typedef struct instruction {
    /// the instruction's header; stores the variant and opcode
    union {
        uint8_t rawbits; // the raw bits of the header
        struct {
            uint8_t variant : 2; // the instruction's variant
            uint8_t opcode : 6; // the instruction's opcode 
        };
    } header;

    uint16_t lh; // the instruction's right-hand operator (big endian)
    uint16_t rh; // the instruction's left-hand operator (big endian)
} instruction_t;
#pragma pack(pop)

extern uint8_t memory[65536]; // 64 KiB of ram
extern uint8_t r[8];

