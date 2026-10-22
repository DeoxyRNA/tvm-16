#if defined(_WIN32)
    #include <windows.h>
#elif defined(__unix__) || defined(__APPLE__)
    #include <time.h>
#endif

#include <stdio.h>
#include <stdlib.h>

#include <tvm/cpu_spec.h>
#include <tvm/cpu_procs.h>
#include <tvm/env.h>

int (*syscallProcs[TVM_NUM_SYSCALLS])(void);

int tvmLoadSyscallProcs() {
    syscallProcs[TVM_SYSCALL_READ] = tvmSyscallProc_READ;
    syscallProcs[TVM_SYSCALL_WRITE] = tvmSyscallProc_WRITE;
    syscallProcs[TVM_SYSCALL_EXIT] = tvmSyscallProc_EXIT;
    syscallProcs[TVM_SYSCALL_SLEEP] = tvmSyscallProc_SLEEP;
    syscallProcs[TVM_SYSCALL_TIME] = tvmSyscallProc_TIME;
    syscallProcs[TVM_SYSCALL_SRAND] = tvmSyscallProc_SRAND;
    syscallProcs[TVM_SYSCALL_RAND] = tvmSyscallProc_RAND;
}

int tvmExecuteSyscall(uint8_t syscallNum) {
    if (syscallNum >= TVM_NUM_SYSCALLS) {
        return TVM_SIGNAL_ILLEGAL;
    }

    return syscallProcs[syscallNum]();
}

int tvmSleep(size_t ms) {
    #if defined(_WIN32)
        Sleep(ms);
    #else
        nanosleep(&(struct timespec){.tv_sec = ms / 1000, .tv_nsec = (ms % 1000) * 1000000}, NULL);
    #endif

    return EXIT_SUCCESS;
}