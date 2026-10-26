#if !defined(INCLUDE_ENV_H)
#define INCLUDE_ENV_H

#define TVM_SYSCALL_READ        ((uint8_t)(0x00)) // 0x00 - Read
#define TVM_SYSCALL_WRITE       ((uint8_t)(0x01)) // 0x01 - Write
#define TVM_SYSCALL_EXIT        ((uint8_t)(0x02)) // 0x02 - Exit
#define TVM_SYSCALL_SLEEP       ((uint8_t)(0x03)) // 0x03 - Sleep
#define TVM_SYSCALL_TIME        ((uint8_t)(0x04)) // 0x04 - Time
#define TVM_SYSCALL_SRAND       ((uint8_t)(0x05)) // 0x05 - Seed Random
#define TVM_SYSCALL_RAND        ((uint8_t)(0x06)) // 0x06 - Random
#define TVM_NUM_SYSCALLS        7

extern const char* INVOCATION_NAME;

inline int tvmSyscallProc_READ(); // 0x00 - Read
inline int tvmSyscallProc_WRITE(); // 0x01 - Write
inline int tvmSyscallProc_EXIT(); // 0x02 - Exit
inline int tvmSyscallProc_SLEEP(); // 0x03 - Sleep
inline int tvmSyscallProc_TIME(); // 0x04 - Time
inline int tvmSyscallProc_SRAND(); // 0x05 - Seed Random
inline int tvmSyscallProc_RAND(); // 0x06 - Random

#endif // INCLUDE_ENV_H