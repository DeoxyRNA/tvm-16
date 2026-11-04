# standard "Hello, World!"

SEGMENT DATA:
    DFB msg, "Hello, World!\n"

SEGMENT CODE:
main:
    LDW R0, 1
    LDW R1, msg
    LDW R2, 14
    SYS
    HLT