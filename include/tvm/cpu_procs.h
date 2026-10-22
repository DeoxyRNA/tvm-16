#if !defined(INCLUDE_CPUPROCS_H)
#define INCLUDE_CPUPROCS_H

#include <stdint.h>

/// @brief Reads a 16-bit unsigned integer from a memory location in little-endian format.
/// @param _Source The memory location to read from.
#define TVM_READ_U16_LENDIAN(_Source) ((( \
    (uint16_t)((uint8_t*)(_Source))[0] << 0) | \
    ((uint16_t)((uint8_t*)(_Source))[1] << 8) \
))

/// @brief Writes a 16-bit unsigned integer to a memory location in little-endian format.
/// @param _Destination The memory location to write to.
/// @param _Source The value to write.
#define TVM_WRITE_U16_LENDIAN(_Destination, _Source) do { \
    ((uint8_t*)(_Destination))[0] = (uint8_t)((_Source) >> 0); \
    ((uint8_t*)(_Destination))[1] = (uint8_t)((_Source) >> 8); \
} while(0)

/// @brief Extracts the lower 8 bts of a 16-bit signed integer (endian-agnostic).
/// @param _Source The 16-bit integer whose low byte to extract.
#define TVM_EXTRACT_U16_LOBYTE(_Source) ((uint8_t)((_Source) & 0x00FFu))

/// @brief Extracts the higher 8 bts of a 16-bit signed integer (endian-agnostic).
/// @param _Source The 16-bit integer whose high byte to extract.
#define TVM_EXTRACT_U16_HIBYTE(_Source) ((uint8_t)((_Source) & 0xFF00u))

/// @brief Overwrites the lower 8 bits of a 16-bit unsigned integer (endian-agnostic).
/// @param _Destination The 16 bit integer to overwrite.
/// @param _Source The value to write.
#define TVM_OVERWRITE_U16_LOBYTE(_Destination, _Source) do { \
    (_Destination) = (uint16_t)(((_Destination) & 0xFF00u) | \
    ((uint8_t)(_Source))); \
} while (0)

/// @brief Overwrites the higher 8 bits of a 16-bit unsigned integer (endian-agnostic).
/// @param _Destination The 16 bit integer to overwrite.
/// @param _Source The value to write.
#define TVM_OVERWRITE_U16_HIBYTE(_Destination, _Source) do { \
    (_Destination) = (uint16_t)(((_Destination) & 0x00FFu) | \
    ((uint16_t)(uint8_t)(_Source) << 8)); \
} while (0)

/// @brief A function delegated to the execution of an instruction.
/// @param variant The instruction's variant.
/// @param dest The instruction's destination operand.
/// @param src The instruction's source operand.
typedef int (*TVM_InstructionProc)(uint8_t variant, uint16_t dest, uint16_t src);

inline int tvmInstrProc_NOP(uint8_t variant, uint16_t dest, uint16_t src); // 0x00 - No Operation
inline int tvmInstrProc_LDW(uint8_t variant, uint16_t dest, uint16_t src); // 0x01 - Load Word
inline int tvmInstrProc_LDB(uint8_t variant, uint16_t dest, uint16_t src); // 0x02 - Load Byte
inline int tvmInstrProc_STW(uint8_t variant, uint16_t dest, uint16_t src); // 0x03 - Store Word
inline int tvmInstrProc_STB(uint8_t variant, uint16_t dest, uint16_t src); // 0x04 - Store Byte
inline int tvmInstrProc_PSW(uint8_t variant, uint16_t dest, uint16_t src); // 0x05 - Push Word
inline int tvmInstrProc_PSB(uint8_t variant, uint16_t dest, uint16_t src); // 0x06 - Push Byte
inline int tvmInstrProc_PPW(uint8_t variant, uint16_t dest, uint16_t src); // 0x07 - Pop Word
inline int tvmInstrProc_PPB(uint8_t variant, uint16_t dest, uint16_t src); // 0x08 - Pop Byte
inline int tvmInstrProc_HLT(uint8_t variant, uint16_t dest, uint16_t src); // 0x09 - Halt
inline int tvmInstrProc_SYS(uint8_t variant, uint16_t dest, uint16_t src); // 0x0A - System Call
inline int tvmInstrProc_BRC(uint8_t variant, uint16_t dest, uint16_t src); // 0x0B - Branch
inline int tvmInstrProc_BEQ(uint8_t variant, uint16_t dest, uint16_t src); // 0x0C - Branch If Equal
inline int tvmInstrProc_BNE(uint8_t variant, uint16_t dest, uint16_t src); // 0x0D - Branch If Not Equal
inline int tvmInstrProc_BCA(uint8_t variant, uint16_t dest, uint16_t src); // 0x0E - Branch If Carry
inline int tvmInstrProc_BNC(uint8_t variant, uint16_t dest, uint16_t src); // 0x0F - Branch If Not Carry
inline int tvmInstrProc_BMI(uint8_t variant, uint16_t dest, uint16_t src); // 0x10 - Branch If Minus
inline int tvmInstrProc_BPL(uint8_t variant, uint16_t dest, uint16_t src); // 0x11 - Branch If Plus
inline int tvmInstrProc_BOV(uint8_t variant, uint16_t dest, uint16_t src); // 0x12 - Branch If Overflow
inline int tvmInstrProc_BNO(uint8_t variant, uint16_t dest, uint16_t src); // 0x13 - Branch If Not Overflow
inline int tvmInstrProc_CLL(uint8_t variant, uint16_t dest, uint16_t src); // 0x14 - Call
inline int tvmInstrProc_RET(uint8_t variant, uint16_t dest, uint16_t src); // 0x15 - Return
inline int tvmInstrProc_ADD(uint8_t variant, uint16_t dest, uint16_t src); // 0x16 - Add
inline int tvmInstrProc_ADS(uint8_t variant, uint16_t dest, uint16_t src); // 0x17 - Add Signed 
inline int tvmInstrProc_ADC(uint8_t variant, uint16_t dest, uint16_t src); // 0x18 - Add With Carry
inline int tvmInstrProc_SUB(uint8_t variant, uint16_t dest, uint16_t src); // 0x19 - Subtract
inline int tvmInstrProc_SBS(uint8_t variant, uint16_t dest, uint16_t src); // 0x1A - Subtract Signed 
inline int tvmInstrProc_SBB(uint8_t variant, uint16_t dest, uint16_t src); // 0x1B - Subtract With Borrow
inline int tvmInstrProc_AND(uint8_t variant, uint16_t dest, uint16_t src); // 0x1C - Bitwise AND
inline int tvmInstrProc_IOR(uint8_t variant, uint16_t dest, uint16_t src); // 0x1D - Bitwise Inclusive OR
inline int tvmInstrProc_XOR(uint8_t variant, uint16_t dest, uint16_t src); // 0x1E - Bitwise Exclusive OR
inline int tvmInstrProc_NOT(uint8_t variant, uint16_t dest, uint16_t src); // 0x1F - Bitwise NOT
inline int tvmInstrProc_SHL(uint8_t variant, uint16_t dest, uint16_t src); // 0x20 - Bitwise Shift Left
inline int tvmInstrProc_SHR(uint8_t variant, uint16_t dest, uint16_t src); // 0x21 - Bitwise Shift Right
inline int tvmInstrProc_CMP(uint8_t variant, uint16_t dest, uint16_t src); // 0x22 - Compare
inline int tvmInstrProc_CPS(uint8_t variant, uint16_t dest, uint16_t src); // 0x23 - Compare Signed

#endif // INCLUDE_CPUPROCS_H