#if !defined(INCLUDE_TVM_CPUPROCS_H)
#define INCLUDE_TVM_CPUPROCS_H

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
    ((uint8_t*)_Destination)[0] = (uint8_t)((_Source) >> 0); \
    ((uint8_t*)_Destination)[1] = (uint8_t)((_Source) >> 8); \
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

#endif // INCLUDE_TVM_CPUPROCS_H