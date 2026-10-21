#if !defined(TVM_SRC_SYS_H)
#define TVM_SRC_SYS_H

#include <stdint.h>

#include <tvm/cpu_spec.h>

/// @brief Executes the specified system call.
/// @param syscallNum The system call number.
/// @return The signal produced by the system call. 
int tvmExecuteSyscall(uint8_t syscallNum);

/// @brief Loads the system call procedures.
/// @return EXIT_SUCCESS on success, EXIT_FAILURE on failure.
int tvmLoadSyscallProcs();

#define TVM_SYSCALL_READ        ((uint8_t)(0x00)) // 0x00 - Read
#define TVM_SYSCALL_WRITE       ((uint8_t)(0x01)) // 0x01 - Write
#define TVM_SYSCALL_OPEN        ((uint8_t)(0x02)) // 0x02 - Open
#define TVM_SYSCALL_CLOSE       ((uint8_t)(0x03)) // 0x03 - Close
#define TVM_SYSCALL_SEEK        ((uint8_t)(0x04)) // 0x04 - Seek
#define TVM_SYSCALL_TELL        ((uint8_t)(0x05)) // 0x05 - Tell
#define TVM_SYSCALL_EXIT        ((uint8_t)(0x06)) // 0x06 - Exit
#define TVM_SYSCALL_SLEEP       ((uint8_t)(0x07)) // 0x07 - Sleep
#define TVM_SYSCALL_TIME        ((uint8_t)(0x08)) // 0x08 - Time
#define TVM_SYSCALL_SRAND       ((uint8_t)(0x09)) // 0x09 - Seed Random
#define TVM_SYSCALL_RAND        ((uint8_t)(0x0A)) // 0x0A - Random
#define TVM_NUM_SYSCALLS        11

#endif // TVM_SRC_SYS_H