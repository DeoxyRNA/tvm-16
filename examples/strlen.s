# compute the length of a null-terminated string

SEGMENT DATA:
    DFB str, "ts lowk a string\0"

SEGMENT CODE:
main:
    # strlen(str)
    PSW BP
    CLL strlen
    ADD SP, 2
    
    LDW R1, R0
    LDW R0, 0x02
    SYS


# WORD strlen(BYTE* _str)
strlen:
    PSW BP              # prologue
    LDW BP, SP

    LDW R3, BP
    ADD R3, 4
    LDW R1, @R3         # R1 = _str
    LDW R2, 0

    .loop:
        LDB R2, @R1
        CMP R2, 0
        BEQ .end
        ADD R1, 1
        BRC .loop
    .end:
        LDW R0, R1
        SUB R0, @R3         # R0 = end - start

    LDW SP, BP          # epilogue
    PPW BP
    RET