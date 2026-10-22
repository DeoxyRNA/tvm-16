#if !defined(TVM_SRC_SYS_H)
#define TVM_SRC_SYS_H

#include <stdint.h>

/// @brief Loads the system call procedures.
/// @return EXIT_SUCCESS on success, EXIT_FAILURE on failure.
int tvmLoadSyscallProcs();

/// @brief Executes the specified system call.
/// @param syscallNum The system call number.
/// @return The signal produced by the system call. 
int tvmExecuteSyscall(uint8_t syscallNum);

/// @brief Sleeps for the specified number of milliseconds.
/// @param ms The number of milliseconds to sleep.
/// @return EXIT_SUCCESS on success, EXIT_FAILURE on failure.
int tvmSleep(size_t ms);

#endif // TVM_SRC_SYS_H