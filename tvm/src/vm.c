#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <tvm/cpu_procs.h>
#include <tvm/cpu_spec.h>
#include <tvm/env.h>

#include "vm.h"
#include "sys.h"

uint16_t regs[8];
uint8_t memory[TVM_ADDRESS_SPACE_WIDTH];
uint16_t exitCode = 0;

static TVM_InstructionProc instrProcs[TVM_NUM_OPCODES];

static void loadInstrProcs(void) {
    instrProcs[TVM_OPCODE_NOP] = tvmInstrProc_NOP;
    instrProcs[TVM_OPCODE_LDW] = tvmInstrProc_LDW;
    instrProcs[TVM_OPCODE_LDB] = tvmInstrProc_LDB;
    instrProcs[TVM_OPCODE_STW] = tvmInstrProc_STW;
    instrProcs[TVM_OPCODE_STB] = tvmInstrProc_STB;
    instrProcs[TVM_OPCODE_PSW] = tvmInstrProc_PSW;
    instrProcs[TVM_OPCODE_PSB] = tvmInstrProc_PSB;
    instrProcs[TVM_OPCODE_PPW] = tvmInstrProc_PPW;
    instrProcs[TVM_OPCODE_PPB] = tvmInstrProc_PPB;
    instrProcs[TVM_OPCODE_HLT] = tvmInstrProc_HLT;
    instrProcs[TVM_OPCODE_SYS] = tvmInstrProc_SYS;
    instrProcs[TVM_OPCODE_BRC] = tvmInstrProc_BRC;
    instrProcs[TVM_OPCODE_BEQ] = tvmInstrProc_BEQ;
    instrProcs[TVM_OPCODE_BNE] = tvmInstrProc_BNE;
    instrProcs[TVM_OPCODE_BCA] = tvmInstrProc_BCA;
    instrProcs[TVM_OPCODE_BNC] = tvmInstrProc_BNC;
    instrProcs[TVM_OPCODE_BMI] = tvmInstrProc_BMI;
    instrProcs[TVM_OPCODE_BPL] = tvmInstrProc_BPL;
    instrProcs[TVM_OPCODE_BOV] = tvmInstrProc_BOV;
    instrProcs[TVM_OPCODE_BNO] = tvmInstrProc_BNO;
    instrProcs[TVM_OPCODE_CLL] = tvmInstrProc_CLL;
    instrProcs[TVM_OPCODE_RET] = tvmInstrProc_RET;
    instrProcs[TVM_OPCODE_ADD] = tvmInstrProc_ADD;
    instrProcs[TVM_OPCODE_ADS] = tvmInstrProc_ADS;
    instrProcs[TVM_OPCODE_ADC] = tvmInstrProc_ADC;
    instrProcs[TVM_OPCODE_SUB] = tvmInstrProc_SUB;
    instrProcs[TVM_OPCODE_SBS] = tvmInstrProc_SBS;
    instrProcs[TVM_OPCODE_SBB] = tvmInstrProc_SBB;
    instrProcs[TVM_OPCODE_AND] = tvmInstrProc_AND;
    instrProcs[TVM_OPCODE_IOR] = tvmInstrProc_IOR;
    instrProcs[TVM_OPCODE_XOR] = tvmInstrProc_XOR;
    instrProcs[TVM_OPCODE_NOT] = tvmInstrProc_NOT;
    instrProcs[TVM_OPCODE_SHL] = tvmInstrProc_SHL;
    instrProcs[TVM_OPCODE_SHR] = tvmInstrProc_SHR;
    instrProcs[TVM_OPCODE_CMP] = tvmInstrProc_CMP;
    instrProcs[TVM_OPCODE_CPS] = tvmInstrProc_CPS;
}

int tvmRun(char** argv) {
    loadInstrProcs();

    TVM_WRITE_U16_LENDIAN(regs + TVM_REGID_PC, TVM_CODE_SEGMENT_OFFSET);
    TVM_WRITE_U16_LENDIAN(regs + TVM_REGID_BP, TVM_STACK_SEGMENT_OFFSET);
    TVM_WRITE_U16_LENDIAN(regs + TVM_REGID_SP, regs[TVM_REGID_BP]);
    
    // push command line arguments onto the stack
    // start at 1 to ignore the program name
    for (size_t i = 1; argv[i] != NULL; i++) {
        for (size_t j = 0; argv[i][j] != 0; j++) {
            tvmInstrProc_PSB(1, (uint8_t)argv[i][j], 0);
        }
    }

    int programRunning = 1;
    int signal = TVM_SIGNAL_PROCEED;
    while (programRunning) {
        switch (signal) {
            case TVM_SIGNAL_PROCEED:
                TVM_Instruction nextInstr;
                memcpy(&nextInstr, memory + TVM_READ_U16_LENDIAN(regs + TVM_REGID_PC), sizeof(TVM_Instruction));
                signal = instrProcs[nextInstr.opcode](nextInstr.variant, nextInstr.dest, nextInstr.src);
                break;
            case TVM_SIGNAL_SYSCALL:
                signal = tvmExecuteSyscall(TVM_READ_U16_LENDIAN(regs + TVM_REGID_R0));
                break;
            case TVM_SIGNAL_HALT:
                programRunning = 0;
                break;
            case TVM_SIGNAL_ILLEGAL:
                exitCode = 0xFFFF;
                fprintf(stderr, "fatal exception: illegal instruction at 0x%X", TVM_READ_U16_LENDIAN(regs + TVM_REGID_PC));
                break;
            
            default:
                exitCode = 0xFFFE;
                fprintf(stderr, "fatal exception: unrecognized signal 0x%X", signal);
                break;
        }
    }

    fprintf(stdout, "program halted with code %d\n", exitCode);
    return EXIT_SUCCESS;
}
