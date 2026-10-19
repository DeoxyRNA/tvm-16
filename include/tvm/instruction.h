#ifndef INCLUDE_TVM_INSTRUCTION_H
#define INCLUDE_TVM_INSTRUCTION_H

#include <stdint.h>
#include <shared/tvm16.h>

typedef struct TVM_Instruction {
    uint8_t variant;
    uint8_t opcode;
    uint16_t dest;
    uint16_t src;
} TVM_Instruction;

#endif // INCLUDE_TVM_INSTRUCTION_H