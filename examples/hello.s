# standard hello world

SEGMENT DATA:
    DFB msg, "Hello, World!\n"
    ASN len, $ - msg

SEGMENT CODE:
main:
    LDW R0, 1
    LDW R1, msg
    LDW R2, len
    SYS
    HLT