#if !defined(INCLUDE_TVM_CPUSPEC_H)
#define INCLUDE_TVM_CPUSPEC_H

#include <stdint.h>

#define TVM_ZERO_FLAG_BIT           0b0001u // ZF
#define TVM_CARRY_FLAG_BIT          0b0010u // CF
#define TVM_SIGN_FLAG_BIT           0b0100u // SF
#define TVM_OVERFLOW_FLAG_BIT       0b1000u // OF

#define TVM_REGID_R0                0 // general-purpose; syscall id; low word of return value
#define TVM_REGID_R1                1 // general-purpose; high word of return value
#define TVM_REGID_R2                2 // general-purpose
#define TVM_REGID_R3                3 // general-purpose
#define TVM_REGID_SP                4 // stack pointer
#define TVM_REGID_BP                5 // base pointer
#define TVM_REGID_PC                6 // program counter
#define TVM_REGID_FL                7 // flags

#define TVM_SIGNAL_PROCEED          0 // Proceed signal
#define TVM_SIGNAL_ILLEGAL          1 // Illegal instruction signal
#define TVM_SIGNAL_SYSCALL          2 // Syscall signal
#define TVM_SIGNAL_HALT             3 // Halt signal

#define TVM_OPCODE_NOP              ((uint8_t)(0x00)) // 0x00 - No Operation
#define TVM_OPCODE_LDW              ((uint8_t)(0x01)) // 0x01 - Load Word
#define TVM_OPCODE_LDB              ((uint8_t)(0x02)) // 0x02 - Load Byte
#define TVM_OPCODE_STW              ((uint8_t)(0x03)) // 0x03 - Store Word
#define TVM_OPCODE_STB              ((uint8_t)(0x04)) // 0x04 - Store Byte
#define TVM_OPCODE_PSW              ((uint8_t)(0x05)) // 0x05 - Push Word
#define TVM_OPCODE_PSB              ((uint8_t)(0x06)) // 0x06 - Push Byte
#define TVM_OPCODE_PPW              ((uint8_t)(0x07)) // 0x07 - Pop Word
#define TVM_OPCODE_PPB              ((uint8_t)(0x08)) // 0x08 - Pop Byte
#define TVM_OPCODE_HLT              ((uint8_t)(0x09)) // 0x09 - Halt
#define TVM_OPCODE_SYS              ((uint8_t)(0x0A)) // 0x0A - System Call
#define TVM_OPCODE_BRC              ((uint8_t)(0x0B)) // 0x0B - Branch
#define TVM_OPCODE_BEQ              ((uint8_t)(0x0C)) // 0x0C - Branch If Equal
#define TVM_OPCODE_BNE              ((uint8_t)(0x0D)) // 0x0D - Branch If Not Equal
#define TVM_OPCODE_BCA              ((uint8_t)(0x0E)) // 0x0E - Branch If Carry
#define TVM_OPCODE_BNC              ((uint8_t)(0x0F)) // 0x0F - Branch If Not Carry
#define TVM_OPCODE_BMI              ((uint8_t)(0x10)) // 0x10 - Branch If Minus
#define TVM_OPCODE_BPL              ((uint8_t)(0x11)) // 0x11 - Branch If Plus
#define TVM_OPCODE_BOV              ((uint8_t)(0x12)) // 0x12 - Branch If Overflow
#define TVM_OPCODE_BNO              ((uint8_t)(0x13)) // 0x13 - Branch If Not Overflow
#define TVM_OPCODE_CLL              ((uint8_t)(0x14)) // 0x14 - Call
#define TVM_OPCODE_RET              ((uint8_t)(0x15)) // 0x15 - Return
#define TVM_OPCODE_ADD              ((uint8_t)(0x16)) // 0x16 - Add
#define TVM_OPCODE_ADS              ((uint8_t)(0x17)) // 0x17 - Add Signed
#define TVM_OPCODE_ADC              ((uint8_t)(0x18)) // 0x18 - Add With Carry
#define TVM_OPCODE_SUB              ((uint8_t)(0x19)) // 0x19 - Subtract
#define TVM_OPCODE_SBS              ((uint8_t)(0x1A)) // 0x1A - Subtract Signed
#define TVM_OPCODE_SBB              ((uint8_t)(0x1B)) // 0x1B - Subtract With Borrow
#define TVM_OPCODE_AND              ((uint8_t)(0x1C)) // 0x1C - Bitwise And
#define TVM_OPCODE_IOR              ((uint8_t)(0x1D)) // 0x1D - Bitwise Inclusive Or
#define TVM_OPCODE_XOR              ((uint8_t)(0x1E)) // 0x1E - Bitwise Exclusive Or
#define TVM_OPCODE_NOT              ((uint8_t)(0x1F)) // 0x1F - Bitwise Not
#define TVM_OPCODE_SHL              ((uint8_t)(0x20)) // 0x20 - Bitwise Shift Left
#define TVM_OPCODE_SHR              ((uint8_t)(0x21)) // 0x21 - Bitwise Shift Right
#define TVM_OPCODE_CMP              ((uint8_t)(0x22)) // 0x22 - Compare
#define TVM_OPCODE_CPS              ((uint8_t)(0x23)) // 0x23 - Compare Signed
#define TVM_NUM_OPCODES             35

#define TVM_ADDRESS_SPACE_WIDTH     65536u // 64 KiB

extern uint16_t regs[];
extern uint8_t memory[];

#endif // INCLUDE_TVM_CPUSPEC_H