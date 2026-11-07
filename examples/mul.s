# multiply two 16-bit integers

SEGMENT STATIC:
    RES a, 2
    RES b, 2

SEGMENT CODE:
main:
    LDW R0, 0

    LDW R1, a
    LDW R2, 2
    SYS

    LDW R1, b
    LDW R2, 2
    SYS

    # mul(a, b)
    PSW @a
    PSW @b
    CLL mul

    ADD SP, 4

    LDW R1, R0
    LDW R0, 2
    SYS


# WORD mul(WORD _a, WORD _b) 
mul:
    PSW BP
    LDW BP, SP

    LDW R3, BP
    ADD R3, 4
    LDW R1, @R3         # R1 = _b
    ADD R3, 2
    LDW R2, @R3         # R2 = _a

    LDW R0, 0
    .loop:
        ADD R0, R2
        SUB R1, 1
        CMP R1, 0
        BNE .loop
    .end:

    LDW SP, BP          # epilogue
    PPW BP
    RET
