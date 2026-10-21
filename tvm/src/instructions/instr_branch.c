#include <tvm/cpu_procs.h>
#include <tvm/cpu_spec.h>

int tvmInstrProc_BRC(uint8_t variant, uint16_t dest, uint16_t src) {
    switch (variant) {
        case 0:
            if (dest > 7) return TVM_SIGNAL_ILLEGAL;
            return tvmInstrProc_LDW(1, TVM_REGID_PC, regs[dest]); // register, immediate

        case 1:
            return tvmInstrProc_LDW(1, TVM_REGID_PC, dest); // register, immediate

        case 2:
            if (dest > 7) return TVM_SIGNAL_ILLEGAL;
            return tvmInstrProc_LDW(1, TVM_REGID_PC, TVM_READ_U16_LENDIAN(memory + regs[dest])); // register, immediate

        case 3:
            return tvmInstrProc_LDW(1, TVM_REGID_PC, TVM_READ_U16_LENDIAN(memory + dest)); // register, &immediate

        default:
            return TVM_SIGNAL_ILLEGAL;
    }
}

int tvmInstrProc_BEQ(uint8_t variant, uint16_t dest, uint16_t src) {
    if (regs[TVM_REGID_FL] & TVM_ZERO_FLAG_BIT) {
        return tvmInstrProc_BRC(variant, dest, src);
    }

    return TVM_SIGNAL_PROCEED;
}

int tvmInstrProc_BNE(uint8_t variant, uint16_t dest, uint16_t src) {
    if (!(regs[TVM_REGID_FL] & TVM_ZERO_FLAG_BIT)) {
        return tvmInstrProc_BRC(variant, dest, src);
    }

    return TVM_SIGNAL_PROCEED;
}

int tvmInstrProc_BCA(uint8_t variant, uint16_t dest, uint16_t src) {
    if (regs[TVM_REGID_FL] & TVM_CARRY_FLAG_BIT) {
        return tvmInstrProc_BRC(variant, dest, src);
    }

    return TVM_SIGNAL_PROCEED;
}

int tvmInstrProc_BNC(uint8_t variant, uint16_t dest, uint16_t src) {
    if (!(regs[TVM_REGID_FL] & TVM_CARRY_FLAG_BIT)) {
        return tvmInstrProc_BRC(variant, dest, src);
    }

    return TVM_SIGNAL_PROCEED;
}

int tvmInstrProc_BMI(uint8_t variant, uint16_t dest, uint16_t src) {
    if (regs[TVM_REGID_FL] & TVM_SIGN_FLAG_BIT) {
        return tvmInstrProc_BRC(variant, dest, src);
    }

    return TVM_SIGNAL_PROCEED;
}

int tvmInstrProc_BPL(uint8_t variant, uint16_t dest, uint16_t src) {
    if (!(regs[TVM_REGID_FL] & TVM_SIGN_FLAG_BIT)) {
        return tvmInstrProc_BRC(variant, dest, src);
    }

    return TVM_SIGNAL_PROCEED;
}

int tvmInstrProc_BOV(uint8_t variant, uint16_t dest, uint16_t src) {
    if (regs[TVM_REGID_FL] & TVM_OVERFLOW_FLAG_BIT) {
        return tvmInstrProc_BRC(variant, dest, src);
    }

    return TVM_SIGNAL_PROCEED;
}

int tvmInstrProc_BNO(uint8_t variant, uint16_t dest, uint16_t src) {
    if (!(regs[TVM_REGID_FL] & TVM_OVERFLOW_FLAG_BIT)) {
        return tvmInstrProc_BRC(variant, dest, src);
    }

    return TVM_SIGNAL_PROCEED;
}

int tvmInstrProc_CLL(uint8_t variant, uint16_t dest, uint16_t src) {
    switch (variant) {
        case 0: {
            if (dest > 7) return TVM_SIGNAL_ILLEGAL;
            tvmInstrProc_PSW(0, TVM_REGID_PC, 0); // register
            tvmInstrProc_BRC(0, dest, 0); // register
            break;
        }
        case 1: {
            tvmInstrProc_PSW(0, TVM_REGID_PC, 0); // register
            tvmInstrProc_BRC(1, dest, 0); // immediate
            break;
        }
        case 2: {
            if (dest > 7) return TVM_SIGNAL_ILLEGAL;
            tvmInstrProc_PSW(0, TVM_REGID_PC, 0); // register
            tvmInstrProc_BRC(2, dest, 0); // &register
            break;
        }
        case 3: {
            tvmInstrProc_PSW(0, TVM_REGID_PC, 0); // register
            tvmInstrProc_BRC(3, dest, 0); // &immediate
            break;
        }

        default:
            return TVM_SIGNAL_ILLEGAL;
    }

    return TVM_SIGNAL_PROCEED;
}

int tvmInstrProc_RET(uint8_t variant, uint16_t dest, uint16_t src) {
    if (variant != 0) {
        return TVM_SIGNAL_ILLEGAL;
    }

    tvmInstrProc_PPW(0, TVM_REGID_PC, 0);

    return TVM_SIGNAL_PROCEED;
} 