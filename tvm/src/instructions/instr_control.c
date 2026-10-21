#include <tvm/cpu_procs.h>
#include <tvm/cpu_spec.h>

int tvmInstrProc_NOP(uint8_t variant, uint16_t dest, uint16_t src) {
    return TVM_SIGNAL_PROCEED;
}

int tvmInstrProc_LDW(uint8_t variant, uint16_t dest, uint16_t src) {
    if (dest > 7) {
        return TVM_SIGNAL_ILLEGAL;
    }

    switch (variant) {
        case 0: {
            if (src > 7) return TVM_SIGNAL_ILLEGAL;
            TVM_WRITE_U16_LENDIAN(&regs[dest], regs[src]);
            break;
        }
        case 1: {
            TVM_WRITE_U16_LENDIAN(&regs[dest], src);
            break;
        }
        case 2: {
            if (src > 7) return TVM_SIGNAL_ILLEGAL;
            TVM_WRITE_U16_LENDIAN(
                &regs[dest],
                TVM_READ_U16_LENDIAN(memory + regs[src])
            );
            break;
        }
        case 3: {
            TVM_WRITE_U16_LENDIAN(
                &regs[dest],
                TVM_READ_U16_LENDIAN(memory + src)
            );
            break;
        }

        default:
            return TVM_SIGNAL_ILLEGAL;
    }

    return TVM_SIGNAL_PROCEED;
}

int tvmInstrProc_LDB(uint8_t variant, uint16_t dest, uint16_t src) {
    if (dest > 7) {
        return TVM_SIGNAL_ILLEGAL;
    }
    
    switch (variant) {
        case 0: {
            if (src > 7) return TVM_SIGNAL_ILLEGAL;
            TVM_OVERWRITE_U16_LOBYTE(regs[dest], regs[src]);
            break;
        }
        case 1: {
            TVM_OVERWRITE_U16_LOBYTE(regs[dest], src);
            break;
        }
        case 2: {
            if (src > 7) return TVM_SIGNAL_ILLEGAL;
            TVM_OVERWRITE_U16_LOBYTE(
                regs[dest],
                TVM_READ_U16_LENDIAN(memory + regs[src])
            );
            break;
        }
        case 3: {
            TVM_OVERWRITE_U16_LOBYTE(
                regs[dest],
                TVM_READ_U16_LENDIAN(memory + src)
            );
            break;
        }

        default:
            return TVM_SIGNAL_ILLEGAL;
    }

    return TVM_SIGNAL_PROCEED;
}

int tvmInstrProc_STW(uint8_t variant, uint16_t dest, uint16_t src) {
    switch (variant) {
        case 0: {
            if (dest > 7 || src > 7) return TVM_SIGNAL_ILLEGAL;
            TVM_WRITE_U16_LENDIAN(memory + regs[dest], regs[src]);
            break;
        }
        case 1: {
            if (dest > 7) return TVM_SIGNAL_ILLEGAL;
            TVM_WRITE_U16_LENDIAN(memory + regs[dest], src);
            break;
        }
        case 2: {
            if (src > 7) return TVM_SIGNAL_ILLEGAL;
            TVM_WRITE_U16_LENDIAN(memory + dest, regs[src]);
            break;
        }
        case 3: {
            TVM_WRITE_U16_LENDIAN(memory + dest, src);
            break;
        }

        default:
            return TVM_SIGNAL_ILLEGAL;
    }

    return TVM_SIGNAL_PROCEED;
}

int tvmInstrProc_STB(uint8_t variant, uint16_t dest, uint16_t src) {
    switch (variant) {
        case 0: {
            if (dest > 7 || src > 7) return TVM_SIGNAL_ILLEGAL;
            memory[regs[dest]] = TVM_EXTRACT_U16_LOBYTE(regs[src]);
            break;
        }
        case 1: {
            if (dest > 7) return TVM_SIGNAL_ILLEGAL;
            memory[regs[dest]] = TVM_EXTRACT_U16_LOBYTE(src);
            break;
        }
        case 2: {
            if (src > 7) return TVM_SIGNAL_ILLEGAL;
            memory[dest] = TVM_EXTRACT_U16_LOBYTE(regs[src]);
            break;
        }
        case 3: {
            memory[dest] = TVM_EXTRACT_U16_LOBYTE(src);
            break;
        }

        default:
            return TVM_SIGNAL_ILLEGAL;
    }

    return TVM_SIGNAL_PROCEED;
}

int tvmInstrProc_PSW(uint8_t variant, uint16_t dest, uint16_t src) {
    switch (variant) {
        case 0: {
            if (dest > 7) return TVM_SIGNAL_ILLEGAL;
            tvmInstrProc_LDW(1, TVM_REGID_SP, regs[TVM_REGID_SP] - 2); // register, immediate
            tvmInstrProc_STW(2, regs[TVM_REGID_SP], dest); // immediate, register
            break;
        }
        case 1: {
            tvmInstrProc_LDW(1, TVM_REGID_SP, regs[TVM_REGID_SP] - 2); // register, immediate
            tvmInstrProc_STW(3, regs[TVM_REGID_SP], dest); // immediate, immediate
            break;
        }

        default:
            return TVM_SIGNAL_ILLEGAL;
    }

    return TVM_SIGNAL_PROCEED;
}

int tvmInstrProc_PSB(uint8_t variant, uint16_t dest, uint16_t src) {
    switch (variant) {
        case 0: {
            if (dest > 7) return TVM_SIGNAL_ILLEGAL;
            tvmInstrProc_LDW(1, TVM_REGID_SP, regs[TVM_REGID_SP] - 1); // register, immediate
            tvmInstrProc_STB(2, regs[TVM_REGID_SP], dest); // immediate, register
            break;
        }
        case 1: {
            tvmInstrProc_LDW(1, TVM_REGID_SP, regs[TVM_REGID_SP] - 1); // register, immediate
            tvmInstrProc_STB(3, regs[TVM_REGID_SP], dest); // immediate, immediate
            break;
        }

        default:
            return TVM_SIGNAL_ILLEGAL;
    }

    return TVM_SIGNAL_PROCEED;
}

int tvmInstrProc_PPW(uint8_t variant, uint16_t dest, uint16_t src) {
    if (dest > 7 || variant != 0) {
        return TVM_SIGNAL_ILLEGAL;
    }

    TVM_WRITE_U16_LENDIAN(&regs[dest], TVM_READ_U16_LENDIAN(memory + TVM_READ_U16_LENDIAN(regs + TVM_REGID_SP)));
    tvmInstrProc_LDW(1, TVM_REGID_SP, regs[TVM_REGID_SP] + 2); // register, immediate

    return TVM_SIGNAL_PROCEED;
}

int tvmInstrProc_PPB(uint8_t variant, uint16_t dest, uint16_t src) {
    if (dest > 7 || variant != 0) {
        return TVM_SIGNAL_ILLEGAL;
    }

    TVM_OVERWRITE_U16_LOBYTE(regs[dest], memory[regs[TVM_REGID_SP]]);
    tvmInstrProc_LDW(1, TVM_REGID_SP, regs[TVM_REGID_SP] + 1); // register, immediate

    return TVM_SIGNAL_PROCEED;
}

int tvmInstrProc_HLT(uint8_t variant, uint16_t dest, uint16_t src) {
    if (variant != 0) {
        return TVM_SIGNAL_ILLEGAL;
    }

    return TVM_SIGNAL_HALT;
}

int tvmInstrProc_SYS(uint8_t variant, uint16_t dest, uint16_t src) {
    if (variant != 0) {
        return TVM_SIGNAL_ILLEGAL;
    }

    return TVM_SIGNAL_SYSCALL;
}