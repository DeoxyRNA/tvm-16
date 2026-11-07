# TVM-16

This repository contains the source code for the official runtime implementation of TVM-16, a 16-bit virtual machine and ISA.

## Installation

Download one of the official binaries or build from source using CMake.

## Usage

Assembling a source file:
```tasm
# hello.s

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
```

```bash
tasm hello.s -o hello.tvm
```

Running a program:
```bash
tvm hello.tvm
```