#include <tvm/cpu_procs.h>
#include <tvm/env.h>

int (*syscallProcs[TVM_NUM_SYSCALLS])(void);

inline int tvmSyscallProc_READ();   // 0x00 - Read
inline int tvmSyscallProc_WRITE();  // 0x01 - Write
inline int tvmSyscallProc_OPEN();   // 0x02 - Open
inline int tvmSyscallProc_CLOSE();  // 0x03 - Close
inline int tvmSyscallProc_SEEK();   // 0x04 - Seek
inline int tvmSyscallProc_TELL();   // 0x05 - Tell
inline int tvmSyscallProc_EXIT();   // 0x06 - Exit
inline int tvmSyscallProc_SLEEP();  // 0x07 - Sleep
inline int tvmSyscallProc_TIME();   // 0x08 - Time
inline int tvmSyscallProc_SRAND();  // 0x09 - Seed Random
inline int tvmSyscallProc_RAND();   // 0x0A - Random

int tvmLoadSyscallProcs() {
    syscallProcs[TVM_SYSCALL_READ] = tvmSyscallProc_READ;
    syscallProcs[TVM_SYSCALL_WRITE] = tvmSyscallProc_WRITE;
    syscallProcs[TVM_SYSCALL_OPEN] = tvmSyscallProc_OPEN;
    syscallProcs[TVM_SYSCALL_CLOSE] = tvmSyscallProc_CLOSE;
    syscallProcs[TVM_SYSCALL_SEEK] = tvmSyscallProc_SEEK;
    syscallProcs[TVM_SYSCALL_TELL] = tvmSyscallProc_TELL;
    syscallProcs[TVM_SYSCALL_EXIT] = tvmSyscallProc_EXIT;
    syscallProcs[TVM_SYSCALL_SLEEP] = tvmSyscallProc_SLEEP;
    syscallProcs[TVM_SYSCALL_TIME] = tvmSyscallProc_TIME;
    syscallProcs[TVM_SYSCALL_SRAND] = tvmSyscallProc_SRAND;
    syscallProcs[TVM_SYSCALL_RAND] = tvmSyscallProc_RAND;
}

int tvmExecuteSyscall(uint8_t syscallNum) {
    if (syscallNum >= TVM_NUM_SYSCALLS) {
        
    }
}