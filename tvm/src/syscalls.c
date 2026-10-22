#include <stdio.h>
#include <time.h>
#include <stdlib.h>

#include <tvm/cpu_procs.h>
#include <tvm/cpu_spec.h>
#include <tvm/env.h>

#include "sys.h"

int tvmSyscallProc_READ() {
    uint16_t addr = TVM_READ_U16_LENDIAN(memory + TVM_READ_U16_LENDIAN(regs + TVM_REGID_R1));
    uint16_t n = TVM_READ_U16_LENDIAN(regs + TVM_REGID_R2);

    uint16_t i;
    for (i = 0; i < n; i++) {
        int c = getchar();
        if (c == EOF) {
            break;
        }

        (memory + i)[addr] = (uint8_t)getchar();
    }

    TVM_WRITE_U16_LENDIAN(regs + TVM_REGID_R2, i);
    return TVM_SIGNAL_PROCEED;
}

int tvmSyscallProc_WRITE() {
    uint16_t addr = TVM_READ_U16_LENDIAN(memory + TVM_READ_U16_LENDIAN(regs + TVM_REGID_R1));
    uint16_t n = TVM_READ_U16_LENDIAN(regs + TVM_REGID_R2);

    uint16_t i;
    for (i = 0; i < n; i++) {
        putchar((memory + i)[addr]);
    }

    TVM_WRITE_U16_LENDIAN(regs + TVM_REGID_R2, i);
    return TVM_SIGNAL_PROCEED;
}

int tvmSyscallProc_EXIT() {
    exitCode = TVM_READ_U16_LENDIAN(regs + TVM_REGID_R1);
    return TVM_SIGNAL_HALT;
}

int tvmSyscallProc_SLEEP() {
    uint16_t msLowWord = TVM_READ_U16_LENDIAN(regs + TVM_REGID_R1);
    uint16_t msHighWord = TVM_READ_U16_LENDIAN(regs + TVM_REGID_R2);
    uint32_t ms = ((uint32_t)msHighWord << 16) | msLowWord;
    tvmSleep(ms);
    return TVM_SIGNAL_PROCEED;
}

int tvmSyscallProc_TIME() {
    uint32_t currentTime = (uint32_t)time(NULL);
    TVM_WRITE_U16_LENDIAN(regs + TVM_REGID_R1, (uint16_t)(currentTime & 0xFFFF));
    TVM_WRITE_U16_LENDIAN(regs + TVM_REGID_R2, (uint16_t)((currentTime >> 16) & 0xFFFF));
    return TVM_SIGNAL_PROCEED;
}

int tvmSyscallProc_SRAND() {
    uint16_t seedLowWord = TVM_READ_U16_LENDIAN(regs + TVM_REGID_R1);
    uint16_t seedHighWord = TVM_READ_U16_LENDIAN(regs + TVM_REGID_R2);
    uint32_t seed = ((uint32_t)seedHighWord << 16) | seedLowWord;
    srand(seed);
    return TVM_SIGNAL_PROCEED;
}

int tvmSyscallProc_RAND() {
    TVM_WRITE_U16_LENDIAN(regs + TVM_REGID_R1, (uint16_t)(rand() & 0xFFFF));
    return TVM_SIGNAL_PROCEED;
}