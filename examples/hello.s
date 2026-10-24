# standard "Hello, World!"

SEGMENT CODE:
main:
    STB 0x2000, "Hello, World!\n"
    LDW R0, 1
    LDW R1, 0x2000
    LDW R2, 14
    SYS
    HLT