#include <tvm/cpu_procs.h>
#include <tvm/cpu_spec.h>

int tvmInstrProc_ADD(uint8_t variant, uint16_t dest, uint16_t src) {
    switch (variant) {
        case 0: {
            if (dest > 7 || src > 7) return TVM_SIGNAL_ILLEGAL;
            uint32_t result = (uint32_t)regs[dest] + (uint32_t)regs[src];
            regs[dest] = (uint16_t)result;
            regs[TVM_REGID_FL] = (regs[TVM_REGID_FL] & ~(TVM_ZERO_FLAG_BIT | TVM_CARRY_FLAG_BIT)) |
                ((result == 0) ? TVM_ZERO_FLAG_BIT : 0) |
                ((result > 0xFFFF) ? TVM_CARRY_FLAG_BIT : 0);
            break;
        }
        case 1: {
            if (dest > 7) return TVM_SIGNAL_ILLEGAL;
            uint32_t result = (uint32_t)regs[dest] + (uint32_t)src;
            regs[dest] = (uint16_t)result;
            regs[TVM_REGID_FL] = (regs[TVM_REGID_FL] & ~(TVM_ZERO_FLAG_BIT | TVM_CARRY_FLAG_BIT)) |
                ((result == 0) ? TVM_ZERO_FLAG_BIT : 0) |
                ((result > 0xFFFF) ? TVM_CARRY_FLAG_BIT : 0);
            break;
        }
        case 2: {
            if (dest > 7 || src > 7) return TVM_SIGNAL_ILLEGAL;
            uint32_t result = (uint32_t)regs[dest] + (uint32_t)TVM_READ_U16_LENDIAN(memory + regs[src]);
            regs[dest] = (uint16_t)result;
            regs[TVM_REGID_FL] = (regs[TVM_REGID_FL] & ~(TVM_ZERO_FLAG_BIT | TVM_CARRY_FLAG_BIT)) |
                ((result == 0) ? TVM_ZERO_FLAG_BIT : 0) |
                ((result > 0xFFFF) ? TVM_CARRY_FLAG_BIT : 0);
            break;
        }
        case 3: {
            if (dest > 7) return TVM_SIGNAL_ILLEGAL;
            uint32_t result = (uint32_t)regs[dest] + (uint32_t)TVM_READ_U16_LENDIAN(memory + src);
            regs[dest] = (uint16_t)result;
            regs[TVM_REGID_FL] = (regs[TVM_REGID_FL] & ~(TVM_ZERO_FLAG_BIT | TVM_CARRY_FLAG_BIT)) |
                ((result == 0) ? TVM_ZERO_FLAG_BIT : 0) |
                ((result > 0xFFFF) ? TVM_CARRY_FLAG_BIT : 0);
            break;
        }

        default:
            return TVM_SIGNAL_ILLEGAL;
    }

    return TVM_SIGNAL_PROCEED;
}

int tvmInstrProc_ADS(uint8_t variant, uint16_t dest, uint16_t src) {
    switch (variant) {
        case 0: {
            if (dest > 7 || src > 7) return TVM_SIGNAL_ILLEGAL;
            int32_t result = (int32_t)(int16_t)regs[dest] + (int32_t)(int16_t)regs[src];
            regs[dest] = (uint16_t)result;
            regs[TVM_REGID_FL] = (regs[TVM_REGID_FL] & ~(TVM_ZERO_FLAG_BIT | TVM_SIGN_FLAG_BIT | TVM_OVERFLOW_FLAG_BIT)) |
                ((result == 0) ? TVM_ZERO_FLAG_BIT : 0) |
                ((result < -32768 || result > 32767) ? TVM_OVERFLOW_FLAG_BIT : 0) |
                ((result < 0) ? TVM_SIGN_FLAG_BIT : 0);
            break;
        }
        case 1: {
            if (dest > 7) return TVM_SIGNAL_ILLEGAL;
            int32_t result = (int32_t)(int16_t)regs[dest] + (int32_t)(int16_t)src;
            regs[dest] = (uint16_t)result;
            regs[TVM_REGID_FL] = (regs[TVM_REGID_FL] & ~(TVM_ZERO_FLAG_BIT | TVM_SIGN_FLAG_BIT | TVM_OVERFLOW_FLAG_BIT)) |
                ((result == 0) ? TVM_ZERO_FLAG_BIT : 0) |
                ((result < -32768 || result > 32767) ? TVM_OVERFLOW_FLAG_BIT : 0) |
                ((result < 0) ? TVM_SIGN_FLAG_BIT : 0);
            break;
        }
        case 2: {
            if (dest > 7 || src > 7) return TVM_SIGNAL_ILLEGAL;
            int32_t result = (int32_t)(int16_t)regs[dest] + (int32_t)(int16_t)TVM_READ_U16_LENDIAN(memory + regs[src]);
            regs[dest] = (uint16_t)result;
            regs[TVM_REGID_FL] = (regs[TVM_REGID_FL] & ~(TVM_ZERO_FLAG_BIT | TVM_SIGN_FLAG_BIT | TVM_OVERFLOW_FLAG_BIT)) |
                ((result == 0) ? TVM_ZERO_FLAG_BIT : 0) |
                ((result < -32768 || result > 32767) ? TVM_OVERFLOW_FLAG_BIT : 0) |
                ((result < 0) ? TVM_SIGN_FLAG_BIT : 0);
            break;
        }
        case 3: {
            if (dest > 7) return TVM_SIGNAL_ILLEGAL;
            int32_t result = (int32_t)(int16_t)regs[dest] + (int32_t)(int16_t)TVM_READ_U16_LENDIAN(memory + src);
            regs[dest] = (uint16_t)result;
            regs[TVM_REGID_FL] = (regs[TVM_REGID_FL] & ~(TVM_ZERO_FLAG_BIT | TVM_SIGN_FLAG_BIT | TVM_OVERFLOW_FLAG_BIT)) |
                ((result == 0) ? TVM_ZERO_FLAG_BIT : 0) |
                ((result < -32768 || result > 32767) ? TVM_OVERFLOW_FLAG_BIT : 0) |
                ((result < 0) ? TVM_SIGN_FLAG_BIT : 0);
            break;
        }

        default:
            return TVM_SIGNAL_ILLEGAL;
    }

    return TVM_SIGNAL_PROCEED;
}

int tvmInstrProc_ADC(uint8_t variant, uint16_t dest, uint16_t src) {
    uint16_t operand;

    switch (variant) {
        case 0:
            if (dest > 7 || src > 7) return TVM_SIGNAL_ILLEGAL;
            operand = regs[src];
            break;
        case 1:
            if (dest > 7) return TVM_SIGNAL_ILLEGAL;
            operand = src;
            break;
        case 2:
            if (dest > 7 || src > 7) return TVM_SIGNAL_ILLEGAL;
            operand = TVM_READ_U16_LENDIAN(memory + regs[src]);
            break;
        case 3:
            if (dest > 7) return TVM_SIGNAL_ILLEGAL;
            operand = TVM_READ_U16_LENDIAN(memory + src);
            break;

        default:
            return TVM_SIGNAL_ILLEGAL;
    }

    uint32_t result = (uint32_t)regs[dest] + operand +
        ((regs[TVM_REGID_FL] & TVM_CARRY_FLAG_BIT) ? 1u : 0u);
    regs[dest] = (uint16_t)result;
    regs[TVM_REGID_FL] = (regs[TVM_REGID_FL] & ~(TVM_ZERO_FLAG_BIT | TVM_CARRY_FLAG_BIT)) |
        ((regs[dest] == 0) ? TVM_ZERO_FLAG_BIT : 0) |
        ((result > 0xFFFFu) ? TVM_CARRY_FLAG_BIT : 0);

    return TVM_SIGNAL_PROCEED;
}

int tvmInstrProc_SUB(uint8_t variant, uint16_t dest, uint16_t src) {
    switch (variant) {
        case 0: {
            if (dest > 7 || src > 7) return TVM_SIGNAL_ILLEGAL;
            uint32_t a = regs[dest], b = regs[src];
            uint32_t result = a - b;
            regs[dest] = (uint16_t)result;
            regs[TVM_REGID_FL] = (regs[TVM_REGID_FL] & ~(TVM_ZERO_FLAG_BIT | TVM_CARRY_FLAG_BIT)) |
                ((result == 0) ? TVM_ZERO_FLAG_BIT : 0) |
                ((b > a) ? TVM_CARRY_FLAG_BIT : 0);
            break;
        }
        case 1: {
            if (dest > 7) return TVM_SIGNAL_ILLEGAL;
            uint32_t a = regs[dest], b = src;
            uint32_t result = a - b;
            regs[dest] = (uint16_t)result;
            regs[TVM_REGID_FL] = (regs[TVM_REGID_FL] & ~(TVM_ZERO_FLAG_BIT | TVM_CARRY_FLAG_BIT)) |
                ((result == 0) ? TVM_ZERO_FLAG_BIT : 0) |
                ((b > a) ? TVM_CARRY_FLAG_BIT : 0);
            break;
        }
        case 2: {
            if (dest > 7 || src > 7) return TVM_SIGNAL_ILLEGAL;
            uint32_t a = regs[dest], b = TVM_READ_U16_LENDIAN(memory + regs[src]);
            uint32_t result = a - b;
            regs[dest] = (uint16_t)result;
            regs[TVM_REGID_FL] = (regs[TVM_REGID_FL] & ~(TVM_ZERO_FLAG_BIT | TVM_CARRY_FLAG_BIT)) |
                ((result == 0) ? TVM_ZERO_FLAG_BIT : 0) |
                ((b > a) ? TVM_CARRY_FLAG_BIT : 0);
            break;
        }
        case 3: {
            if (dest > 7) return TVM_SIGNAL_ILLEGAL;
            uint32_t a = regs[dest], b = TVM_READ_U16_LENDIAN(memory + src);
            uint32_t result = a - b;
            regs[dest] = (uint16_t)result;
            regs[TVM_REGID_FL] = (regs[TVM_REGID_FL] & ~(TVM_ZERO_FLAG_BIT | TVM_CARRY_FLAG_BIT)) |
                ((result == 0) ? TVM_ZERO_FLAG_BIT : 0) |
                ((b > a) ? TVM_CARRY_FLAG_BIT : 0);
            break;
        }

        default:
            return TVM_SIGNAL_ILLEGAL;
    }

    return TVM_SIGNAL_PROCEED;
}

int tvmInstrProc_SBS(uint8_t variant, uint16_t dest, uint16_t src) {
    switch (variant) {
        case 0: {
            if (dest > 7 || src > 7) return TVM_SIGNAL_ILLEGAL;
            int32_t a = (int32_t)(int16_t)regs[dest], b = (int32_t)(int16_t)regs[src];
            int32_t result = a - b;
            regs[dest] = (uint16_t)result;
            regs[TVM_REGID_FL] = (regs[TVM_REGID_FL] & ~(TVM_ZERO_FLAG_BIT | TVM_SIGN_FLAG_BIT | TVM_OVERFLOW_FLAG_BIT)) |
                ((result == 0) ? TVM_ZERO_FLAG_BIT : 0) |
                ((result < -0x8000 || result > 0x7FFF) ? TVM_OVERFLOW_FLAG_BIT : 0) |
                ((result < 0) ? TVM_SIGN_FLAG_BIT : 0);
            break;
        }
        case 1: {
            if (dest > 7) return TVM_SIGNAL_ILLEGAL;
            int32_t a = (int32_t)(int16_t)regs[dest], b = (int32_t)(int16_t)src;
            int32_t result = a - b;
            regs[dest] = (uint16_t)result;
            regs[TVM_REGID_FL] = (regs[TVM_REGID_FL] & ~(TVM_ZERO_FLAG_BIT | TVM_SIGN_FLAG_BIT | TVM_OVERFLOW_FLAG_BIT)) |
                ((result == 0) ? TVM_ZERO_FLAG_BIT : 0) |
                ((result < -0x8000 || result > 0x7FFF) ? TVM_OVERFLOW_FLAG_BIT : 0) |
                ((result < 0) ? TVM_SIGN_FLAG_BIT : 0);
            break;
        }
        case 2: {
            if (dest > 7 || src > 7) return TVM_SIGNAL_ILLEGAL;
            int32_t a = (int32_t)(int16_t)regs[dest], b = (int32_t)(int16_t)TVM_READ_U16_LENDIAN(memory + regs[src]);
            int32_t result = a - b;
            regs[dest] = (uint16_t)result;
            regs[TVM_REGID_FL] = (regs[TVM_REGID_FL] & ~(TVM_ZERO_FLAG_BIT | TVM_SIGN_FLAG_BIT | TVM_OVERFLOW_FLAG_BIT)) |
                ((result == 0) ? TVM_ZERO_FLAG_BIT : 0) |
                ((result < -0x8000 || result > 0x7FFF) ? TVM_OVERFLOW_FLAG_BIT : 0) |
                ((result < 0) ? TVM_SIGN_FLAG_BIT : 0);
            break;
        }
        case 3: {
            if (dest > 7) return TVM_SIGNAL_ILLEGAL;
            int32_t a = (int32_t)(int16_t)regs[dest], b = (int32_t)(int16_t)TVM_READ_U16_LENDIAN(memory + src);
            int32_t result = a - b;
            regs[dest] = (uint16_t)result;
            regs[TVM_REGID_FL] = (regs[TVM_REGID_FL] & ~(TVM_ZERO_FLAG_BIT | TVM_SIGN_FLAG_BIT | TVM_OVERFLOW_FLAG_BIT)) |
                ((result == 0) ? TVM_ZERO_FLAG_BIT : 0) |
                ((result < -0x8000 || result > 0x7FFF) ? TVM_OVERFLOW_FLAG_BIT : 0) |
                ((result < 0) ? TVM_SIGN_FLAG_BIT : 0);
            break;
        }

        default:
            return TVM_SIGNAL_ILLEGAL;
    }

    return TVM_SIGNAL_PROCEED;
}

int tvmInstrProc_SBB(uint8_t variant, uint16_t dest, uint16_t src) {
    uint16_t operand;

    switch (variant) {
        case 0:
            if (dest > 7 || src > 7) return TVM_SIGNAL_ILLEGAL;
            operand = regs[src];
            break;
        case 1:
            if (dest > 7) return TVM_SIGNAL_ILLEGAL;
            operand = src;
            break;
        case 2:
            if (dest > 7 || src > 7) return TVM_SIGNAL_ILLEGAL;
            operand = TVM_READ_U16_LENDIAN(memory + regs[src]);
            break;
        case 3:
            if (dest > 7) return TVM_SIGNAL_ILLEGAL;
            operand = TVM_READ_U16_LENDIAN(memory + src);
            break;
        default:
            return TVM_SIGNAL_ILLEGAL;
    }

    uint32_t subtrahend = (uint32_t)operand +
        ((regs[TVM_REGID_FL] & TVM_CARRY_FLAG_BIT) ? 1u : 0u);
    uint32_t minuend = regs[dest];
    uint32_t result = minuend - subtrahend;
    regs[dest] = (uint16_t)result;
    regs[TVM_REGID_FL] = (regs[TVM_REGID_FL] & ~(TVM_ZERO_FLAG_BIT | TVM_CARRY_FLAG_BIT)) |
        ((regs[dest] == 0) ? TVM_ZERO_FLAG_BIT : 0) |
        ((minuend < subtrahend) ? TVM_CARRY_FLAG_BIT : 0);

    return TVM_SIGNAL_PROCEED;
}

int tvmInstrProc_AND(uint8_t variant, uint16_t dest, uint16_t src) {
    switch (variant) {
        case 0: {
            if (dest > 7 || src > 7) return TVM_SIGNAL_ILLEGAL;
            regs[dest] &= regs[src];
            regs[TVM_REGID_FL] = (regs[TVM_REGID_FL] & ~TVM_ZERO_FLAG_BIT) |
                ((regs[dest] == 0) ? TVM_ZERO_FLAG_BIT : 0);
            break;
        }
        case 1: {
            if (dest > 7) return TVM_SIGNAL_ILLEGAL;
            regs[dest] &= src;
            regs[TVM_REGID_FL] = (regs[TVM_REGID_FL] & ~TVM_ZERO_FLAG_BIT) |
                ((regs[dest] == 0) ? TVM_ZERO_FLAG_BIT : 0);
            break;
        }
        case 2: {
            if (dest > 7 || src > 7) return TVM_SIGNAL_ILLEGAL;
            regs[dest] &= TVM_READ_U16_LENDIAN(memory + regs[src]);
            regs[TVM_REGID_FL] = (regs[TVM_REGID_FL] & ~TVM_ZERO_FLAG_BIT) |
                ((regs[dest] == 0) ? TVM_ZERO_FLAG_BIT : 0);
            break;
        }
        case 3: {
            if (dest > 7) return TVM_SIGNAL_ILLEGAL;
            regs[dest] &= TVM_READ_U16_LENDIAN(memory + src);
            regs[TVM_REGID_FL] = (regs[TVM_REGID_FL] & ~TVM_ZERO_FLAG_BIT) |
                ((regs[dest] == 0) ? TVM_ZERO_FLAG_BIT : 0);
            break;
        }

        default:
            return TVM_SIGNAL_ILLEGAL;
    }

    return TVM_SIGNAL_PROCEED;
}

int tvmInstrProc_IOR(uint8_t variant, uint16_t dest, uint16_t src) {
    switch (variant) {
        case 0: {
            if (dest > 7 || src > 7) return TVM_SIGNAL_ILLEGAL;
            regs[dest] |= regs[src];
            regs[TVM_REGID_FL] = (regs[TVM_REGID_FL] & ~TVM_ZERO_FLAG_BIT) |
                ((regs[dest] == 0) ? TVM_ZERO_FLAG_BIT : 0);
            break;
        }
        case 1: {
            if (dest > 7) return TVM_SIGNAL_ILLEGAL;
            regs[dest] |= src;
            regs[TVM_REGID_FL] = (regs[TVM_REGID_FL] & ~TVM_ZERO_FLAG_BIT) |
                ((regs[dest] == 0) ? TVM_ZERO_FLAG_BIT : 0);
            break;
        }
        case 2: {
            if (dest > 7 || src > 7) return TVM_SIGNAL_ILLEGAL;
            regs[dest] |= TVM_READ_U16_LENDIAN(memory + regs[src]);
            regs[TVM_REGID_FL] = (regs[TVM_REGID_FL] & ~TVM_ZERO_FLAG_BIT) |
                ((regs[dest] == 0) ? TVM_ZERO_FLAG_BIT : 0);
            break;
        }
        case 3: {
            if (dest > 7) return TVM_SIGNAL_ILLEGAL;
            regs[dest] |= TVM_READ_U16_LENDIAN(memory + src);
            regs[TVM_REGID_FL] = (regs[TVM_REGID_FL] & ~TVM_ZERO_FLAG_BIT) |
                ((regs[dest] == 0) ? TVM_ZERO_FLAG_BIT : 0);
            break;
        }

        default:
            return TVM_SIGNAL_ILLEGAL;
    }

    return TVM_SIGNAL_PROCEED;
}

int tvmInstrProc_XOR(uint8_t variant, uint16_t dest, uint16_t src) {
    switch (variant) {
        case 0: {
            if (dest > 7 || src > 7) return TVM_SIGNAL_ILLEGAL;
            regs[dest] ^= regs[src];
            regs[TVM_REGID_FL] = (regs[TVM_REGID_FL] & ~TVM_ZERO_FLAG_BIT) |
                ((regs[dest] == 0) ? TVM_ZERO_FLAG_BIT : 0);
            break;
        }
        case 1: {
            if (dest > 7) return TVM_SIGNAL_ILLEGAL;
            regs[dest] ^= src;
            regs[TVM_REGID_FL] = (regs[TVM_REGID_FL] & ~TVM_ZERO_FLAG_BIT) |
                ((regs[dest] == 0) ? TVM_ZERO_FLAG_BIT : 0);
            break;
        }
        case 2: {
            if (dest > 7 || src > 7) return TVM_SIGNAL_ILLEGAL;
            regs[dest] ^= TVM_READ_U16_LENDIAN(memory + regs[src]);
            regs[TVM_REGID_FL] = (regs[TVM_REGID_FL] & ~TVM_ZERO_FLAG_BIT) |
                ((regs[dest] == 0) ? TVM_ZERO_FLAG_BIT : 0);
            break;
        }
        case 3: {
            if (dest > 7) return TVM_SIGNAL_ILLEGAL;
            regs[dest] ^= TVM_READ_U16_LENDIAN(memory + src);
            regs[TVM_REGID_FL] = (regs[TVM_REGID_FL] & ~TVM_ZERO_FLAG_BIT) |
                ((regs[dest] == 0) ? TVM_ZERO_FLAG_BIT : 0);
            break;
        }

        default:
            return TVM_SIGNAL_ILLEGAL;
    }

    return TVM_SIGNAL_PROCEED;
}

int tvmInstrProc_NOT(uint8_t variant, uint16_t dest, uint16_t src) {
    if (dest > 7 || variant != 0) {
        return TVM_SIGNAL_ILLEGAL;
    }

    regs[dest] = ~regs[dest];
    regs[TVM_REGID_FL] = (regs[TVM_REGID_FL] & ~TVM_ZERO_FLAG_BIT) |
        ((regs[dest] == 0) ? TVM_ZERO_FLAG_BIT : 0);

    return TVM_SIGNAL_PROCEED;
}

int tvmInstrProc_SHL(uint8_t variant, uint16_t dest, uint16_t src) {
    if (dest > 7 || variant != 0) {
        return TVM_SIGNAL_ILLEGAL;
    }

    uint16_t carry = (TVM_READ_U16_LENDIAN(regs + dest) & 0x8000) ? 1 : 0;
    regs[dest] <<= 1;
    regs[TVM_REGID_FL] = (regs[TVM_REGID_FL] & ~(TVM_ZERO_FLAG_BIT | TVM_CARRY_FLAG_BIT)) |
        ((regs[dest] == 0) ? TVM_ZERO_FLAG_BIT : 0) |
        (carry ? TVM_CARRY_FLAG_BIT : 0);

    return TVM_SIGNAL_PROCEED;
}

int tvmInstrProc_SHR(uint8_t variant, uint16_t dest, uint16_t src) {
    if (dest > 7 || variant != 0) {
        return TVM_SIGNAL_ILLEGAL;
    }

    uint16_t carry = (TVM_READ_U16_LENDIAN(regs + dest) & 0x0001) ? 1 : 0;
    regs[dest] >>= 1;
    regs[TVM_REGID_FL] = (regs[TVM_REGID_FL] & ~(TVM_ZERO_FLAG_BIT | TVM_CARRY_FLAG_BIT)) |
        ((regs[dest] == 0) ? TVM_ZERO_FLAG_BIT : 0) |
        (carry ? TVM_CARRY_FLAG_BIT : 0);

    return TVM_SIGNAL_PROCEED;
}

int tvmInstrProc_CMP(uint8_t variant, uint16_t dest, uint16_t src) {
    switch (variant) {
        case 0: {
            if (dest > 7 || src > 7) return TVM_SIGNAL_ILLEGAL;
            uint32_t a = regs[dest], b = regs[src];
            uint32_t result = a - b;
            regs[TVM_REGID_FL] = (regs[TVM_REGID_FL] & ~(TVM_ZERO_FLAG_BIT | TVM_CARRY_FLAG_BIT)) |
                ((result == 0) ? TVM_ZERO_FLAG_BIT : 0) |
                ((b > a) ? TVM_CARRY_FLAG_BIT : 0);
            break;
        }
        case 1: {
            if (dest > 7) return TVM_SIGNAL_ILLEGAL;
            uint32_t a = regs[dest], b = src;
            uint32_t result = a - b;
            regs[TVM_REGID_FL] = (regs[TVM_REGID_FL] & ~(TVM_ZERO_FLAG_BIT | TVM_CARRY_FLAG_BIT)) |
                ((result == 0) ? TVM_ZERO_FLAG_BIT : 0) |
                ((b > a) ? TVM_CARRY_FLAG_BIT : 0);
            break;
        }
        case 2: {
            if (dest > 7 || src > 7) return TVM_SIGNAL_ILLEGAL;
            uint32_t a = regs[dest], b = TVM_READ_U16_LENDIAN(memory + regs[src]);
            uint32_t result = a - b;
            regs[TVM_REGID_FL] = (regs[TVM_REGID_FL] & ~(TVM_ZERO_FLAG_BIT | TVM_CARRY_FLAG_BIT)) |
                ((result == 0) ? TVM_ZERO_FLAG_BIT : 0) |
                ((b > a) ? TVM_CARRY_FLAG_BIT : 0);
            break;
        }
        case 3: {
            if (dest > 7) return TVM_SIGNAL_ILLEGAL;
            uint32_t a = regs[dest], b = TVM_READ_U16_LENDIAN(memory + src);
            uint32_t result = a - b;
            regs[TVM_REGID_FL] = (regs[TVM_REGID_FL] & ~(TVM_ZERO_FLAG_BIT | TVM_CARRY_FLAG_BIT)) |
                ((result == 0) ? TVM_ZERO_FLAG_BIT : 0) |
                ((b > a) ? TVM_CARRY_FLAG_BIT : 0);
            break;
        }

        default:
            return TVM_SIGNAL_ILLEGAL;
    }

    return TVM_SIGNAL_PROCEED;
}

int tvmInstrProc_CPS(uint8_t variant, uint16_t dest, uint16_t src) {
    switch (variant) {
        case 0: {
            if (dest > 7 || src > 7) return TVM_SIGNAL_ILLEGAL;
            int32_t a = (int32_t)(int16_t)regs[dest], b = (int32_t)(int16_t)regs[src];
            int32_t result = a - b;
            regs[TVM_REGID_FL] = (regs[TVM_REGID_FL] & ~(TVM_ZERO_FLAG_BIT | TVM_CARRY_FLAG_BIT | TVM_SIGN_FLAG_BIT | TVM_OVERFLOW_FLAG_BIT)) |
                ((result == 0) ? TVM_ZERO_FLAG_BIT : 0) |
                ((a < b) ? TVM_CARRY_FLAG_BIT : 0) |
                ((result < -32768 || result > 32767) ? TVM_OVERFLOW_FLAG_BIT : 0) |
                ((result < 0) ? TVM_SIGN_FLAG_BIT : 0);
            break;
        }
        case 1: {
            if (dest > 7) return TVM_SIGNAL_ILLEGAL;
            int32_t a = (int32_t)(int16_t)regs[dest], b = (int32_t)(int16_t)src;
            int32_t result = a - b;
            regs[TVM_REGID_FL] = (regs[TVM_REGID_FL] & ~(TVM_ZERO_FLAG_BIT | TVM_CARRY_FLAG_BIT | TVM_SIGN_FLAG_BIT | TVM_OVERFLOW_FLAG_BIT)) |
                ((result == 0) ? TVM_ZERO_FLAG_BIT : 0) |
                ((a < b) ? TVM_CARRY_FLAG_BIT : 0) |
                ((result < -32768 || result > 32767) ? TVM_OVERFLOW_FLAG_BIT : 0) |
                ((result < 0) ? TVM_SIGN_FLAG_BIT : 0);
            break;
        }
        case 2: {
            if (dest > 7 || src > 7) return TVM_SIGNAL_ILLEGAL;
            int32_t a = (int32_t)(int16_t)regs[dest], b = (int32_t)(int16_t)TVM_READ_U16_LENDIAN(memory + regs[src]);
            int32_t result = a - b;
            regs[TVM_REGID_FL] = (regs[TVM_REGID_FL] & ~(TVM_ZERO_FLAG_BIT | TVM_CARRY_FLAG_BIT | TVM_SIGN_FLAG_BIT | TVM_OVERFLOW_FLAG_BIT)) |
                ((result == 0) ? TVM_ZERO_FLAG_BIT : 0) |
                ((a < b) ? TVM_CARRY_FLAG_BIT : 0) |
                ((result < -32768 || result > 32767) ? TVM_OVERFLOW_FLAG_BIT : 0) |
                ((result < 0) ? TVM_SIGN_FLAG_BIT : 0);
            break;
        }
        case 3: {
            if (dest > 7) return TVM_SIGNAL_ILLEGAL;
            int32_t a = (int32_t)(int16_t)regs[dest], b = (int32_t)(int16_t)TVM_READ_U16_LENDIAN(memory + src);
            int32_t result = a - b;
            regs[TVM_REGID_FL] = (regs[TVM_REGID_FL] & ~(TVM_ZERO_FLAG_BIT | TVM_CARRY_FLAG_BIT | TVM_SIGN_FLAG_BIT | TVM_OVERFLOW_FLAG_BIT)) |
                ((result == 0) ? TVM_ZERO_FLAG_BIT : 0) |
                ((a < b) ? TVM_CARRY_FLAG_BIT : 0) |
                ((result < -32768 || result > 32767) ? TVM_OVERFLOW_FLAG_BIT : 0) |
                ((result < 0) ? TVM_SIGN_FLAG_BIT : 0);
            break;
        }

        default:
            return TVM_SIGNAL_ILLEGAL;
    }

    return TVM_SIGNAL_PROCEED;
}