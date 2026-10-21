#if defined(_WIN32)
    #include <winbase.h>
    #include <fileapi.h>
    #include <memoryapi.h>
    #include <wchar.h>
#elif defined(__unix__) || defined(__APPLE__)
    #include <unistd.h>
    #include <sys/mman.h>
    #include <sys/stat.h>
    #include <fcntl.h>
#else
    #error "Unsupported platform - compile with GCC or Clang on Windows, Linux, or macOS"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <tvm/cpuprocs.h>
#include <tvm/cpuspec.h>

uint16_t regs[8];
uint8_t memory[TVM_ADDRESS_SPACE_WIDTH];

TVM_InstructionProc instrProcs[TVM_NUM_OPCODES];

int _instrProc_NOP(uint8_t variant, uint16_t dest, uint16_t src); // 0x00 - No Operation
int _instrProc_LDW(uint8_t variant, uint16_t dest, uint16_t src); // 0x01 - Load Word
int _instrProc_LDB(uint8_t variant, uint16_t dest, uint16_t src); // 0x02 - Load Byte
int _instrProc_STW(uint8_t variant, uint16_t dest, uint16_t src); // 0x03 - Store Word
int _instrProc_STB(uint8_t variant, uint16_t dest, uint16_t src); // 0x04 - Store Byte
int _instrProc_PSW(uint8_t variant, uint16_t dest, uint16_t src); // 0x05 - Push Word
int _instrProc_PSB(uint8_t variant, uint16_t dest, uint16_t src); // 0x06 - Push Byte
int _instrProc_PPW(uint8_t variant, uint16_t dest, uint16_t src); // 0x07 - Pop Word
int _instrProc_PPB(uint8_t variant, uint16_t dest, uint16_t src); // 0x08 - Pop Byte
int _instrProc_HLT(uint8_t variant, uint16_t dest, uint16_t src); // 0x09 - Halt
int _instrProc_SYS(uint8_t variant, uint16_t dest, uint16_t src); // 0x0A - System Call
int _instrProc_BRC(uint8_t variant, uint16_t dest, uint16_t src); // 0x0B - Branch
int _instrProc_BEQ(uint8_t variant, uint16_t dest, uint16_t src); // 0x0C - Branch If Equal
int _instrProc_BNE(uint8_t variant, uint16_t dest, uint16_t src); // 0x0D - Branch If Not Equal
int _instrProc_BCA(uint8_t variant, uint16_t dest, uint16_t src); // 0x0E - Branch If Carry
int _instrProc_BNC(uint8_t variant, uint16_t dest, uint16_t src); // 0x0F - Branch If Not Carry
int _instrProc_BMI(uint8_t variant, uint16_t dest, uint16_t src); // 0x10 - Branch If Minus
int _instrProc_BPL(uint8_t variant, uint16_t dest, uint16_t src); // 0x11 - Branch If Plus
int _instrProc_BOV(uint8_t variant, uint16_t dest, uint16_t src); // 0x12 - Branch If Overflow
int _instrProc_BNO(uint8_t variant, uint16_t dest, uint16_t src); // 0x13 - Branch If Not Overflow
int _instrProc_CLL(uint8_t variant, uint16_t dest, uint16_t src); // 0x14 - Call
int _instrProc_RET(uint8_t variant, uint16_t dest, uint16_t src); // 0x15 - Return
int _instrProc_ADD(uint8_t variant, uint16_t dest, uint16_t src); // 0x16 - Add
int _instrProc_ADS(uint8_t variant, uint16_t dest, uint16_t src); // 0x17 - Add Signed 
int _instrProc_ADC(uint8_t variant, uint16_t dest, uint16_t src); // 0x18 - Add With Carry
int _instrProc_SUB(uint8_t variant, uint16_t dest, uint16_t src); // 0x19 - Subtract
int _instrProc_SBS(uint8_t variant, uint16_t dest, uint16_t src); // 0x1A - Subtract Signed 
int _instrProc_SBB(uint8_t variant, uint16_t dest, uint16_t src); // 0x1B - Subtract With Borrow
int _instrProc_AND(uint8_t variant, uint16_t dest, uint16_t src); // 0x1C - Bitwise AND
int _instrProc_IOR(uint8_t variant, uint16_t dest, uint16_t src); // 0x1D - Bitwise Inclusive OR
int _instrProc_XOR(uint8_t variant, uint16_t dest, uint16_t src); // 0x1E - Bitwise Exclusive OR
int _instrProc_NOT(uint8_t variant, uint16_t dest, uint16_t src); // 0x1F - Bitwise NOT
int _instrProc_SHL(uint8_t variant, uint16_t dest, uint16_t src); // 0x20 - Bitwise Shift Left
int _instrProc_SHR(uint8_t variant, uint16_t dest, uint16_t src); // 0x21 - Bitwise Shift Right
int _instrProc_CMP(uint8_t variant, uint16_t dest, uint16_t src); // 0x22 - Compare
int _instrProc_CPS(uint8_t variant, uint16_t dest, uint16_t src); // 0x23 - Compare Signed

void* _createReadonlyFileView(const char* restrict filename, uint64_t* restrict size, uint64_t* restrict error) {
#if defined(_WIN32)
    // win32-compliant implementation

    HANDLE file = CreateFileA(
        filename,
        GENERIC_READ,
        0,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL
    );

    if (file == INVALID_HANDLE_VALUE) {
        if (error != NULL) *error = GetLastError();
        if (size != NULL) *size = 0;
        return NULL;
    }

    LARGE_INTEGER fileSize;
    if (!GetFileSizeEx(file, &fileSize)) {
        if (error != NULL) *error = GetLastError();
        if (size != NULL) *size = 0;
        CloseHandle(file);
        return NULL;
    }

    if (fileSize.QuadPart == 0) {
        if (error != NULL) *error = 0;
        if (size != NULL) *size = 0;
        CloseHandle(file);
        return NULL;
    }

    HANDLE mapping = CreateFileMappingA(
        file,
        NULL,
        PAGE_READONLY,
        fileSize.HighPart,
        fileSize.LowPart,
        NULL
    );

    CloseHandle(file);

    if (mapping == NULL) {
        if (error != NULL) *error = GetLastError();
        if (size != NULL) *size = 0;
        return NULL;
    }

    void* data = MapViewOfFile(mapping, FILE_MAP_READ, 0, 0, 0);

    CloseHandle(mapping); 

    if (data == NULL) {
        if (error != NULL) *error = GetLastError();
        if (size != NULL) *size = 0;
        return NULL;
    }

    if (size != NULL) *size = (uint64_t)fileSize.QuadPart;
    return data;
#else
    // posix-compliant implementation

    int fileDesc = open(filename, O_RDONLY);
    if (fileDesc == -1) {
        if (error != NULL) *error = (uint64_t)errno;
        if (size != NULL) *size = 0;
        return NULL;
    }

    struct stat fileStat;
    if (fstat(fileDesc, &fileStat) == -1) {
        if (error != NULL) *error = (uint64_t)errno;
        if (size != NULL) *size = 0;
        close(fileDesc);
        return NULL;
    }

    if (fileStat.st_size == 0) {
        close(fileDesc);
        if (error != NULL) *error = 0;
        if (size != NULL) *size = 0;
        return NULL;
    }


    void* data = mmap(NULL, fileStat.st_size, PROT_READ, MAP_PRIVATE, fileDesc, 0);
   
    if (data == MAP_FAILED) {
        if (error != NULL) *error = (uint64_t)errno;
        if (size != NULL) *size = 0;
        close(fileDesc);
        return NULL;
    }   

    close(fileDesc);

    if (size != NULL) *size = (uint64_t)fileStat.st_size;
    return data;
#endif
}

void _destroyReadonlyFileView(const void* restrict view, uint64_t size) {
    if (view == NULL || size == 0) return;
#if defined(_WIN32)
    UnmapViewOfFile(view);
#else
    munmap((void*)view, (size_t)size);
#endif
}

void _loadInstrProcs() {
    instrProcs[TVM_OPCODE_NOP] = _instrProc_NOP;
    instrProcs[TVM_OPCODE_LDW] = _instrProc_LDW;
    instrProcs[TVM_OPCODE_LDB] = _instrProc_LDB;
    instrProcs[TVM_OPCODE_STW] = _instrProc_STW;
    instrProcs[TVM_OPCODE_STB] = _instrProc_STB;
    instrProcs[TVM_OPCODE_PSW] = _instrProc_PSW;
    instrProcs[TVM_OPCODE_PSB] = _instrProc_PSB;
    instrProcs[TVM_OPCODE_PPW] = _instrProc_PPW;
    instrProcs[TVM_OPCODE_PPB] = _instrProc_PPB;
    instrProcs[TVM_OPCODE_HLT] = _instrProc_HLT;
    instrProcs[TVM_OPCODE_SYS] = _instrProc_SYS;
    instrProcs[TVM_OPCODE_BRC] = _instrProc_BRC;
    instrProcs[TVM_OPCODE_BEQ] = _instrProc_BEQ;
    instrProcs[TVM_OPCODE_BNE] = _instrProc_BNE;
    instrProcs[TVM_OPCODE_BCA] = _instrProc_BCA;
    instrProcs[TVM_OPCODE_BNC] = _instrProc_BNC;
    instrProcs[TVM_OPCODE_BMI] = _instrProc_BMI;
    instrProcs[TVM_OPCODE_BPL] = _instrProc_BPL;
    instrProcs[TVM_OPCODE_BOV] = _instrProc_BOV;
    instrProcs[TVM_OPCODE_BNO] = _instrProc_BNO;
    instrProcs[TVM_OPCODE_CLL] = _instrProc_CLL;
    instrProcs[TVM_OPCODE_RET] = _instrProc_RET;
    instrProcs[TVM_OPCODE_ADD] = _instrProc_ADD;
    instrProcs[TVM_OPCODE_ADS] = _instrProc_ADS;
    instrProcs[TVM_OPCODE_ADC] = _instrProc_ADC;
    instrProcs[TVM_OPCODE_SUB] = _instrProc_SUB;
    instrProcs[TVM_OPCODE_SBS] = _instrProc_SBS;
    instrProcs[TVM_OPCODE_SBB] = _instrProc_SBB;
    instrProcs[TVM_OPCODE_AND] = _instrProc_AND;
    instrProcs[TVM_OPCODE_IOR] = _instrProc_IOR;
    instrProcs[TVM_OPCODE_XOR] = _instrProc_XOR;
    instrProcs[TVM_OPCODE_NOT] = _instrProc_NOT;
    instrProcs[TVM_OPCODE_SHL] = _instrProc_SHL;
    instrProcs[TVM_OPCODE_SHR] = _instrProc_SHR;
    instrProcs[TVM_OPCODE_CMP] = _instrProc_CMP;
    instrProcs[TVM_OPCODE_CPS] = _instrProc_CPS;
}

int main() {
    _loadInstrProcs();

    const char* path = "file.tvm";

    uint64_t fileSize, error;
    void* fileView = _createReadonlyFileView(path, &fileSize, &error);

    if (fileView == NULL) {
#if defined(_WIN32)
        if (error == ERROR_FILE_NOT_FOUND || error == ERROR_PATH_NOT_FOUND) {
            fprintf(stderr, "error: at \'%s\', no such file exists\n", path);
        }
#else
        if (errno == ENOENT || error == ENOTDIR) {
            fprintf(stderr, "error: at \'%s\', no such file exists\n", path);
        }
#endif
        else if (fileSize == 0) {
            fprintf(stderr, "error: file at \'%s\' contains no data\n", path);
        } else {
            fprintf(stderr, "error: could not access file \'%s\'\n", path);
        }

        return EXIT_FAILURE;
    }
}


int _instrProc_NOP(uint8_t variant, uint16_t dest, uint16_t src) {
    return TVM_SIGNAL_PROCEED;
}

int _instrProc_LDW(uint8_t variant, uint16_t dest, uint16_t src) {
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

int _instrProc_LDB(uint8_t variant, uint16_t dest, uint16_t src) {
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

int _instrProc_STW(uint8_t variant, uint16_t dest, uint16_t src) {
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

int _instrProc_STB(uint8_t variant, uint16_t dest, uint16_t src) {
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

int _instrProc_PSW(uint8_t variant, uint16_t dest, uint16_t src) {
    switch (variant) {
        case 0: {
            if (dest > 7) return TVM_SIGNAL_ILLEGAL;
            _instrProc_LDW(1, TVM_REGID_SP, regs[TVM_REGID_SP] - 2); // register, immediate
            _instrProc_STW(2, regs[TVM_REGID_SP], dest); // immediate, register
            break;
        }
        case 1: {
            _instrProc_LDW(1, TVM_REGID_SP, regs[TVM_REGID_SP] - 2); // register, immediate
            _instrProc_STW(3, regs[TVM_REGID_SP], dest); // immediate, immediate
            break;
        }

        default:
            return TVM_SIGNAL_ILLEGAL;
    }

    return TVM_SIGNAL_PROCEED;
}

int _instrProc_PSB(uint8_t variant, uint16_t dest, uint16_t src) {
    switch (variant) {
        case 0: {
            if (dest > 7) return TVM_SIGNAL_ILLEGAL;
            _instrProc_LDW(1, TVM_REGID_SP, regs[TVM_REGID_SP] - 1); // register, immediate
            _instrProc_STB(2, regs[TVM_REGID_SP], dest); // immediate, register
            break;
        }
        case 1: {
            _instrProc_LDW(1, TVM_REGID_SP, regs[TVM_REGID_SP] - 1); // register, immediate
            _instrProc_STB(3, regs[TVM_REGID_SP], dest); // immediate, immediate
            break;
        }

        default:
            return TVM_SIGNAL_ILLEGAL;
    }

    return TVM_SIGNAL_PROCEED;
}

int _instrProc_PPW(uint8_t variant, uint16_t dest, uint16_t src) {
    if (dest > 7 || variant != 0) {
        return TVM_SIGNAL_ILLEGAL;
    }

    TVM_WRITE_U16_LENDIAN(&regs[dest], TVM_READ_U16_LENDIAN(memory + TVM_READ_U16_LENDIAN(regs + TVM_REGID_SP)));
    _instrProc_LDW(1, TVM_REGID_SP, regs[TVM_REGID_SP] + 2); // register, immediate

    return TVM_SIGNAL_PROCEED;
}

int _instrProc_PPB(uint8_t variant, uint16_t dest, uint16_t src) {
    if (dest > 7 || variant != 0) {
        return TVM_SIGNAL_ILLEGAL;
    }

    TVM_OVERWRITE_U16_LOBYTE(regs[dest], memory[regs[TVM_REGID_SP]]);
    _instrProc_LDW(1, TVM_REGID_SP, regs[TVM_REGID_SP] + 1); // register, immediate

    return TVM_SIGNAL_PROCEED;
}

int _instrProc_HLT(uint8_t variant, uint16_t dest, uint16_t src) {
    if (variant != 0) {
        return TVM_SIGNAL_ILLEGAL;
    }

    return TVM_SIGNAL_HALT;
}

int _instrProc_SYS(uint8_t variant, uint16_t dest, uint16_t src) {
    if (variant != 0) {
        return TVM_SIGNAL_ILLEGAL;
    }

    return TVM_SIGNAL_SYSCALL;
}

int _instrProc_BRC(uint8_t variant, uint16_t dest, uint16_t src) {
    switch (variant) {
        case 0:
            if (dest > 7) return TVM_SIGNAL_ILLEGAL;
            return _instrProc_LDW(1, TVM_REGID_PC, regs[dest]); // register, immediate

        case 1:
            return _instrProc_LDW(1, TVM_REGID_PC, dest); // register, immediate

        case 2:
            if (dest > 7) return TVM_SIGNAL_ILLEGAL;
            return _instrProc_LDW(1, TVM_REGID_PC, TVM_READ_U16_LENDIAN(memory + regs[dest])); // register, immediate

        case 3:
            return _instrProc_LDW(1, TVM_REGID_PC, TVM_READ_U16_LENDIAN(memory + dest)); // register, &immediate

        default:
            return TVM_SIGNAL_ILLEGAL;
    }
}

int _instrProc_BEQ(uint8_t variant, uint16_t dest, uint16_t src) {
    if (regs[TVM_REGID_FL] & TVM_ZERO_FLAG_BIT) {
        return _instrProc_BRC(variant, dest, src);
    }

    return TVM_SIGNAL_PROCEED;
}

int _instrProc_BNE(uint8_t variant, uint16_t dest, uint16_t src) {
    if (!(regs[TVM_REGID_FL] & TVM_ZERO_FLAG_BIT)) {
        return _instrProc_BRC(variant, dest, src);
    }

    return TVM_SIGNAL_PROCEED;
}

int _instrProc_BCA(uint8_t variant, uint16_t dest, uint16_t src) {
    if (regs[TVM_REGID_FL] & TVM_CARRY_FLAG_BIT) {
        return _instrProc_BRC(variant, dest, src);
    }

    return TVM_SIGNAL_PROCEED;
}

int _instrProc_BNC(uint8_t variant, uint16_t dest, uint16_t src) {
    if (!(regs[TVM_REGID_FL] & TVM_CARRY_FLAG_BIT)) {
        return _instrProc_BRC(variant, dest, src);
    }

    return TVM_SIGNAL_PROCEED;
}

int _instrProc_BMI(uint8_t variant, uint16_t dest, uint16_t src) {
    if (regs[TVM_REGID_FL] & TVM_SIGN_FLAG_BIT) {
        return _instrProc_BRC(variant, dest, src);
    }

    return TVM_SIGNAL_PROCEED;
}

int _instrProc_BPL(uint8_t variant, uint16_t dest, uint16_t src) {
    if (!(regs[TVM_REGID_FL] & TVM_SIGN_FLAG_BIT)) {
        return _instrProc_BRC(variant, dest, src);
    }

    return TVM_SIGNAL_PROCEED;
}

int _instrProc_BOV(uint8_t variant, uint16_t dest, uint16_t src) {
    if (regs[TVM_REGID_FL] & TVM_OVERFLOW_FLAG_BIT) {
        return _instrProc_BRC(variant, dest, src);
    }

    return TVM_SIGNAL_PROCEED;
}

int _instrProc_BNO(uint8_t variant, uint16_t dest, uint16_t src) {
    if (!(regs[TVM_REGID_FL] & TVM_OVERFLOW_FLAG_BIT)) {
        return _instrProc_BRC(variant, dest, src);
    }

    return TVM_SIGNAL_PROCEED;
}

int _instrProc_CLL(uint8_t variant, uint16_t dest, uint16_t src) {
    switch (variant) {
        case 0: {
            if (dest > 7) return TVM_SIGNAL_ILLEGAL;
            _instrProc_PSW(0, TVM_REGID_PC, 0); // register
            _instrProc_BRC(0, dest, 0); // register
            break;
        }
        case 1: {
            _instrProc_PSW(0, TVM_REGID_PC, 0); // register
            _instrProc_BRC(1, dest, 0); // immediate
            break;
        }
        case 2: {
            if (dest > 7) return TVM_SIGNAL_ILLEGAL;
            _instrProc_PSW(0, TVM_REGID_PC, 0); // register
            _instrProc_BRC(2, dest, 0); // &register
            break;
        }
        case 3: {
            _instrProc_PSW(0, TVM_REGID_PC, 0); // register
            _instrProc_BRC(3, dest, 0); // &immediate
            break;
        }

        default:
            return TVM_SIGNAL_ILLEGAL;
    }

    return TVM_SIGNAL_PROCEED;
}

int _instrProc_RET(uint8_t variant, uint16_t dest, uint16_t src) {
    if (variant != 0) {
        return TVM_SIGNAL_ILLEGAL;
    }

    _instrProc_PPW(0, TVM_REGID_PC, 0);

    return TVM_SIGNAL_PROCEED;
} 

int _instrProc_ADD(uint8_t variant, uint16_t dest, uint16_t src) {
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

// make sure to update the zero flag, sign flag, and overflow flag for signed addition
int _instrProc_ADS(uint8_t variant, uint16_t dest, uint16_t src) {
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

int _instrProc_ADC(uint8_t variant, uint16_t dest, uint16_t src) {
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

int _instrProc_SUB(uint8_t variant, uint16_t dest, uint16_t src) {
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

int _instrProc_SBS(uint8_t variant, uint16_t dest, uint16_t src) {
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

int _instrProc_SBB(uint8_t variant, uint16_t dest, uint16_t src) {
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

int _instrProc_AND(uint8_t variant, uint16_t dest, uint16_t src) {
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

int _instrProc_IOR(uint8_t variant, uint16_t dest, uint16_t src) {
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

int _instrProc_XOR(uint8_t variant, uint16_t dest, uint16_t src) {
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

int _instrProc_NOT(uint8_t variant, uint16_t dest, uint16_t src) {
    if (dest > 7 || variant != 0) {
        return TVM_SIGNAL_ILLEGAL;
    }

    regs[dest] = ~regs[dest];
    regs[TVM_REGID_FL] = (regs[TVM_REGID_FL] & ~TVM_ZERO_FLAG_BIT) |
        ((regs[dest] == 0) ? TVM_ZERO_FLAG_BIT : 0);

    return TVM_SIGNAL_PROCEED;
}

int _instrProc_SHL(uint8_t variant, uint16_t dest, uint16_t src) {
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

int _instrProc_SHR(uint8_t variant, uint16_t dest, uint16_t src) {
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

int _instrProc_CMP(uint8_t variant, uint16_t dest, uint16_t src) {
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

// compare signed values and update the zero, carry, sign, and overflow flags
int _instrProc_CPS(uint8_t variant, uint16_t dest, uint16_t src) {
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