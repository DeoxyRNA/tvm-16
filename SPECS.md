# TVM-16 Implementation Specifications
This document serves as a guide for any users wanting to implement or use TVM-16. The official implementation can be found [here](https://github.com/DeoxyRNA/tvm-16).

## Contents
1. [Contents (you're already here)](#contents)
2. [Architecture](#architecture)
    * [Overview](#overview)
    * [Registers](#registers)
    * [Instruction Layout](#instruction-layout)
    * [Calling Convention](#calling-convention)
3. [Assembler](#assembler)
    * [Syntax](#syntax)
    * [Literals](#literals)
    * [Segments](#segments)
4. [Instruction Set](#instruction-set)
    * [System Instructions](#system-instructions)
    * [Branch Instructions](#branch-instructions)
    * [Arithmetic and Logic Instructions](#arithmetic-and-logic-instructions)
5. [System Calls](#system-calls)
    * [I/O Calls](#i/o-calls)
    * [Utility Calls](#utility-calls)

## Architecture
### Overview
TVM-16 has 64 KiB of random-access memory, addressable from 0x0000 to 0xFFFF. Each memory location can store a byte (8-bit number). Words are encoded using 2 bytes, and thus occupy 2 memory locations.

Addresses 0x0000 to 0x9FFF are usable for general data, and adresses 0xA000 to 0xFFFF are reserved for the program's code.\
The stack grows downwards, starting at address 0x9FFF. Pushing decreases the stack pointer, and popping increases it.

The CPU reads and writes numbers to and from memory in little-endian format. This means that the most significant byte is stored at the highest memory address.
| Address | Byte value |
| :- | :- |
| `0x2000` | `0x39` |
| `0x2001` | `0x30` |


### Registers
The CPU has 8 registers, each of which are 2 bytes (16-bits) wide. Since data cannot be directly written from one address to another, each register acts as a buffer between the CPU and memory.
| Register | ID | Purpose |
| :- | :- | :- |
| R0 | `0` | general purpose |
| R1 | `1` | general purpose |
| R2 | `2` | general purpose |
| R3 | `3` | general purpose |
| RS | `4` | "stack pointer;" holds the address of the top of the stack |
| RB | `5` | "base pointer;" holds the address of the bottom of the current stack frame |
| RI | `6` | "instruction pointer;" holds the address of the current instruction |
| RF | `7` | "flag mask;" a bitmask containing each status flag |

RS and RF are initialized at 0x9FFF. When the program runs, the command line arguments are pushed onto the stack.\
RI is initialized at 0xA000, and unless it branches, increases by 5 after each instruction is executed, until the program halts goes out of memory bounds.\
R0, R1, R2, R3, and RF are initialized at 0x0000.

The status flags provide additional information about the result of the last arithmetic operation. They are stored as a bit mask in RF.
| Flag | Bit mask value | Name |
| :- | :- | :- |
| Z | `1` | Zero Flag |
| C | `2` | Carry Flag |
| S | `4` | Sign Flag |
| O | `8` | Overflow Flag |

The upper 12 bits of RF are unused. The are never cleared or set by the arithmetic operations.

### Instruction Layout
Each instruction is encoded as a 5-byte string. The highest byte, called the Instruction Header, stores the instruction's opcode and variant. The highest 2 bits of the Instruction Header are occupied by the variant, while the lowest 6 are occupied by the opcode. The lowest 4 bytes of the instruction are the two operands, which are stored as words in their respective order.

For example, consider the following:
```
ADD R2, 0xAFC8
```

The machine code for this instruction would look like:
```
[01][010110] [00000010 00000000] [11001000 10101111]

variant: 0x1
opcode: ADD (0x16)
operand 1: R2 (0x0002)
operand 2: 0xAFC8
```
Note: The variants are opcode-specific, and not all opcodes use all (or any) of them. Instructions that use fewer than 2 operands will have unused bytes in the operand fields. These are ignored on decode, but should be set to 0.

### Calling Convention
When a function is called, a new stack frame is created. All of the functions arguments are stored on the stack frame, the return value is passed back in a register, and cleanup after the call is the caller's responsibility.

#### Register Roles
| Register | Role |
| :- | :- |
| R0 | lower word of return value, caller-saved |
| R1 | upper word of return value, caller-saved |
| R2 | scratch register, caller-saved |
| R3 | used for base-relative addressing, caller-saved |
| RS | stack pointer; callee-saved |
| RB | base pointer; callee-saved |
| RI | not touched directly |
| RF | not preserved across calls |

Nothing is expected to be preserved across a call except RB and RS. If a caller needs a value in R0–R3 after a call, they must save it (e.g. push it or) beforehand and load it afterward.

#### Argument Passing
All arguments are words, pushed onto the stack right-to-left (the last argument is pushed first), then the call is made:
```
PSW arg1
PSW arg2
CLS my_func
ADD RS, 4       # caller cleans up: 2 args * 2 bytes
```
It is the caller's responsibility to clean up; RET only pops the return address.\
If the return value is a word, its value is held in R0. If it is 32-bit, its lower word is held in R0 and its upper word is held in R1.

#### Stack Frame
Given a call to a two-argument function, immediately preceded by pushing its second argument, then its first, the frame looks like this once the prologue has run:

| Address (relative to RB) | Contents |
| :- | :- |
| RB + 8 | arg2 |
| RB + 6 | arg1 |
| RB + 4 | return address |
| RB + 2 | saved caller RB |
| RB + 0 | frame base |
| RB - 2 | local 1 |
| RB - 4 | local 2 |
<br>

This is the conventional function prologue (used to set up the stack frame):
```
my_func:
    PSW RB          # save caller's rb
    LDW RB, RS      # make new frame: RB = RS
    SUB RS, N       # reserve N bytes for locals (if there are any)
```
After this, RB + 2 holds the saved caller's RB, RB + 4 holds the return address, RB + 6, 8, 10, and so on are arg1, arg2, arg3, and so on, and RB - 2, 4, 6, and so on are local variables.

This is the conventional function epilogue (used to destroy the stack frame):
```
    LDW RS, RB      # drop locals: RS = RB
    PPW RB          # restore caller's RB
    RET             # pop return address and branch back to call site
```

#### Example
```
# int add2(int a, int b) { return a + b; }
add2:
    PSW RB
    LDW RB, RS

    LDW R3, RB
    ADD R3, 6
    LDW R0, @R3          # R0 = a   (arg1, at RB+6)

    LDW R3, RB
    ADD R3, 8
    LDW R1, @R3          # R1 = b   (arg2, at RB+8)

    ADD R0, R1           # R0 = a + b   (return value)

    LDW RS, RB
    PPW RB
    RET

# add2(3, 4)
main:
    PSW 4                # push b (rightmost first)
    PSW 3                # push a
    CLS add2
    ADD RS, 4            # caller cleanup: 2 args * 2 bytes

    HLT
```

## Assembler
In case you didn't feel like typing out each individual byte of machine code, TVM-16 features its very own assembly language, TAsm (TVM-16 Assembly), which maps the system's binary machine instructions to human-readable ones.

### Syntax
TAsm's syntax is simple. Each instrucion is written like this:
```
[mnemonic] [operand 1 (if used)], [operand 2 (if used)]
```

Comments, denoted by the hashtag (#) are ignored by the assembler.
```
# this is a comment
LDW R0, 0x4392              # this comment is at the end of the line
```

### Literals
Literals are constant values stored in the program's data. They appear in multiple forms, including number literals, which can be written in decimal, hexadecimal, octal, or binary:
```
PSW 13920                   # this is a decimal literal
PSW 0x3660                  # this is a hexadecimal literal
PSW 0o33140                 # this is an octal literal
PSW 0b0011011001100000      # this is a binary literal
```

Character literals, wrapped in single quotes
```
PSB 'a'                     # this is a character literal
```

And string literals, wrapped in double quotes
```
PSB "Hello"                 # this is a string literal
```

#### Labels
Instead of manually calculating the addresses of all your subroutines, you can use labels instead. Each label is automatically resolved with its address at assemble-time.

```
ADD R0, 5                   # this instruction is at address 0x0000
BNC not_carry

if_carry:                   # a label's address is that of the instruction that follows it, so this label is at address 0x000A
    SUB R2, 10
    PSW R2
    HLT

not_carry:                  # this label is at address 0x0019
   PSW 4                    # if the operation on line 1 produces a carry, this code won't be reached
   HLT
```

You can also use local labels to help organize your code. These are denoted with a dot (.) at the beginning, and are relative to the most recent global label.
```
label:
    PSB 8
    ADS R3, 2
.local:                 
    ADD R0, 3

CMP R1, 7
BCA label.local             # branches to address 0x000A
```

#### Dereferencing
Values can be dereferenced to access the value at the address they point to. This is done using the "at" symbol (@).
```
LDW R0, @0x2000             # load the value at address 0x2000 into R0
```

### Segments
TAsm has 3 segments, each of which are used to provide the assembler information about the program.

#### The STATIC Segment
The STATIC segment is used for declarations of variables that are zero-initialized.

The RESERVE directive is used to reserve memory for zero-initialized variables. It is then followed by a type, a name, and an number. The variable names are resolved with memory addresses at assemble time.
```
SEGMENT STATIC:
    a RESERVE WORD 1        # reserve one word for variable a
    b RESERVE BYTE 10       # reserve an array of 10 bytes for variable b
```

These variable names can be used in code as though they are memory addresses.
```
LDW R0, @a                  # load the value of a into R0
STW b, R0                   # store the value of R0 into b
```

#### The DATA Segment
The DATA segment is used to provide declarations of variables that are initialized with non-zero values.

The DEFINE directive is used to define variables with initial values. It is then followed by a type, a name, and an initializer. The initial values of these variables are stored in the program's code region, and take up space on the executable. The variable names are resolved with memory addresses at assemble time.\
The ASSIGN directive is used to define symbols that are expanded to immediate values at assemble time.\
```
SEGMENT DATA:
    a DEFINE WORD 0x1234    # reserve a word for the variable a, and store the value 0x1234 at its address
    b ASSIGN 0x56           # assign the immediate value 0x56 to the symbol b
```

Variables can be used in code as if they are memory addresses. Symbols defined with the ASSIGN directive can be used as immediate values.
```
LDW R0, &a                  # load the value of a into R0
LDW R0, b                   # load the value of b into R0
STW 0x2000, b               # store the value of b into address 0x2000
```

#### The TEXT Segment
The TEXT segment is used to provide the program's code.
```
SEGMENT TEXT:
main:
    PSW 0x2000
    HLT
```

## Instruction Set
### System Instructions
#### 0x00: NOP (No Operation)
Does nothing.
```
NOP
```
<br>

#### 0x01: LDW (Load Word)
Loads a word onto a register.
```
LDW [Rn], [Rn]
LDW [Rn], [immediate]
LDW [Rn], [@Rn]
LDW [Rn], [@immediate]
```
<br>

#### 0x02: LDB (Load Byte)
Loads a byte onto a register. Only overwrites the lower byte.\
The value of the second operand is truncated to its lower byte.
```
LDW [Rn], [Rn]
LDW [Rn], [immediate]
LDW [Rn], [@Rn]
LDW [Rn], [@immediate]
```
<br>

#### 0x03: STW (Store Word)
Stores a word at a memory location.\
Operand 1 is the destination, Operand 2 is the source.
```
STW [Rn], [Rn]
STW [Rn], [immediate]
STW [immediate], [Rn]
STW [immediate], [immediate]
```
<br>

#### 0x04: STB (Store Byte)
Stores a byte at a memory location.\
Operand 1 is the destination, Operand 2 is the source.\
The value of the second operand is truncated to its lower byte.
```
STB [Rn], [Rn]
STB [Rn], [immediate]
STB [immediate], [Rn]
STB [immediate], [immediate]
```
Storing a string literal gets unrolled into multiple byte storage operations by the assembler.
```
STB 0x3320, "hello"

# is the same as:

STB 0x3320, 'h'
STB 0x3321, 'e'
STB 0x3322, 'l'
STB 0x3323, 'l'
STB 0x3324, 'o'
```
<br>

#### 0x05: PSW (Push Word)
Pushes a word onto the stack.\
Decreases RS by 2.
```
PSW [Rn]
PSW [immediate]
```
<br>

#### 0x06: PSB (Push Byte)
Pushes a byte onto the stack.\
Decreases RS by 1.
```
PSB [Rn]
PSB [immediate]
```
Pushing a string literal gets unrolled into multiple byte push operations by the assembler.
```
PSB "hello"

# is the same as:

PSB 'h'
PSB 'e'
PSB 'l'
PSB 'l'
PSB 'o'
```
<br>

#### 0x07: PPW (Pop Word)
Pops a word off of the stack.\
Increases RS by 1.
```
PPW [Rn]
```
<br>

#### 0x08: PPB (Pop Byte)
Pops a word off of the stack.\
Only overwrites the lower byte.\
Increases RS by 1.
```
PPB [Rn]
```
<br>

#### 0x09: HLT (Halt)
Halts the program.
```
HLT
```
<br>

#### 0x0A: SYS (System Call)
Invokes a system call with the id in R0.
```
SYS
```
<br>

### Branch Instructions
#### 0x0B: BRC (Branch)
Branches to the specified address.
```
BRC [Rn]
BRC [immediate/label]
BRC [&Rn]
BRC [&immediate]
```
<br>

#### 0x0C: BEQ (Branch if Equal)
Branches to the specified address if the Zero Flag is set.
```
BEQ [Rn]
BEQ [immediate/label]
BEQ [&Rn]
BEQ [&immediate]
```
<br>

#### 0x0D: BNE (Branch if Not Equal)
Branches to the specified address if the Zero Flag is clear.
```
BNE [Rn]
BNE [immediate/label]
BNE [&Rn]
BNE [&immediate]
```
<br>

#### 0x0E: BMI (Branch if Minus)
Branches to the specified address if the Sign Flag is set.
```
BMI [Rn]
BMI [immediate/label]
BMI [&Rn]
BMI [&immediate]
```
<br>

#### 0x0F: BPL (Branch if Plus)
Branches to the specified address if the Sign Flag is clear.
```
BPL [Rn]
BPL [immediate/label]
BPL [&Rn]
BPL [&immediate]
```
<br>

#### 0x10: BCA (Branch if Carry)
Branches to the specified address if the Carry Flag is set.
```
BCA [Rn]
BCA [immediate/label]
BCA [&Rn]
BCA [&immediate]
```
<br>

#### 0x11: BCA (Branch if Not Carry)
Branches to the specified address if the Carry Flag is clear.
```
BNC [Rn]
BNC [immediate/label]
BNC [&Rn]
BNC [&immediate]
```
<br>

#### 0x12: BOV (Branch if Overflow)
Branches to the specified address if the Overflow Flag is set.
```
BOV [Rn]
BOV [immediate/label]
BOV [&Rn]
BOV [&immediate]
```
<br>

#### 0x13: BNO (Branch if Not Overflow)
Branches to the specified address if the Overflow Flag is clear.
```
BNO [Rn]
BNO [immediate/label]
BNO [&Rn]
BNO [&immediate]
```
<br>

#### 0x14: CLS (Call Subroutine)
Calls a subroutine at the specified address, pushing the return address onto the stack.
```
CLS [Rn]
CLS [immediate/label]
CLS [&Rn]
CLS [&immediate]
```
<br>

#### 0x15: RET (Return From Subroutine)
Returns from a subroutine, popping the return address from the stack.
```
RET
```
<br>

### Arithmetic and Logic Instructions
#### 0x16: ADD (Add)
Adds two values together, storing the result in the first operand.\
Sets the Zero Flag if the result is zero; clears it otherwise.\
Sets the Carry Flag if there is a carry; clears it otherwise.\
Clears the Sign Flag and the Overflow Flag.
```
ADD [Rn], [Rn]
ADD [Rn], [immediate]
ADD [Rn], [&Rn]
ADD [Rn], [&immediate]
```
<br>

#### 0x17: ADS (Add Signed)
Adds two values together using two's complement arithmetic, storing the result in the first operand.\
Sets the Zero Flag if the result is zero; clears it otherwise.\
Sets the Carry Flag if there is a carry; clears it otherwise.\
Sets the Sign Flag if the result is negative; clears it otherwise.\
Sets the Overflow Flag if there is an overflow; clears it otherwise.
```
ADS [Rn], [Rn]
ADS [Rn], [immediate]
ADS [Rn], [&Rn]
ADS [Rn], [&immediate]
```
<br>

#### 0x18: ADC (Add with Carry)
Adds two values together, including the carry flag, storing the result in the first operand.\
Sets the Zero Flag if the result is zero; clears it otherwise.\
Sets the Carry Flag if there is a carry; clears it otherwise.\
Clears the Sign Flag and the Overflow Flag.
```
ADC [Rn], [Rn]
ADC [Rn], [immediate]
ADC [Rn], [&Rn]
ADC [Rn], [&immediate]
```
<br>

#### 0x19: SUB (Subtract)
Subtracts the second operand from the first operand, storing the result in the first operand.\
Sets the Zero Flag if the result is zero; clears it otherwise.\
Sets the Carry Flag if there is a borrow; clears it otherwise.\
Clears the Sign Flag and the Overflow Flag.
```
SUB [Rn], [Rn]
SUB [Rn], [immediate]
SUB [Rn], [&Rn]
SUB [Rn], [&immediate]
```
<br>

#### 0x1A: SBS (Subtract Signed)
Subtracts the second operand from the first operand using two's complement arithmetic, storing the result in the first operand.\
Sets the Zero Flag if the result is zero; clears it otherwise.\
Sets the Carry Flag if there is a borrow; clears it otherwise.\
Sets the Sign Flag if the result is negative; clears it otherwise.\
Sets the Overflow Flag if there is an underflow; clears it otherwise.
```
SBS [Rn], [Rn]
SBS [Rn], [immediate]
SBS [Rn], [&Rn]
SBS [Rn], [&immediate]
```
<br>

#### 0x1B: SBB (Subtract with Borrow)
Subtracts the second operand from the first operand, including the borrow flag, storing the result in the first operand.
Sets the Zero Flag if the result is zero; clears it otherwise.
Sets the Carry Flag if there is a borrow; clears it otherwise.
Sets the Sign Flag if the result is negative; clears it otherwise.
Sets the Overflow Flag if there is an underflow; clears it otherwise.
```
SBB [Rn], [Rn]
SBB [Rn], [immediate]
SBB [Rn], [&Rn]
SBB [Rn], [&immediate]
```
<br>

#### 0x1C: AND (Bitwise AND)
Performs a bitwise AND operation on the two operands, storing the result in the first operand.\
Sets the Zero Flag if the result is zero; clears it otherwise.\
Clears the Carry Flag, the Sign Flag, and the Overflow Flag.
```
AND [Rn], [Rn]
AND [Rn], [immediate]
AND [Rn], [&Rn]
AND [Rn], [&immediate]
```
<br>

#### 0x1D: IOR (Bitwise Inclusive OR)
Performs a bitwise inclusive OR operation on the two operands, storing the result in the first operand.\
Sets the Zero Flag if the result is zero; clears it otherwise.\
Clears the Carry Flag, the Sign Flag, and the Overflow Flag.
```
IOR [Rn], [Rn]
IOR [Rn], [immediate]
IOR [Rn], [&Rn]
IOR [Rn], [&immediate]
```
<br>

#### 0x1E: XOR (Bitwise Exclusive OR)
Performs a bitwise exclusive OR operation on the two operands, storing the result in the first operand.\
Sets the Zero Flag if the result is zero; clears it otherwise.\
Clears the Carry Flag, the Sign Flag, and the Overflow Flag.
```
XOR [Rn], [Rn]
XOR [Rn], [immediate]
XOR [Rn], [&Rn]
XOR [Rn], [&immediate]
```
<br>

#### 0x1F: NOT (Bitwise NOT)
Performs a bitwise NOT operation on the operand, storing the result in the operand.\
Sets the Zero Flag if the result is zero; clears it otherwise.\
Clears the Carry Flag, the Sign Flag, and the Overflow Flag.
```
NOT [Rn]
```
<br>

#### 0x20: SHL (Bitwise Shift Left)
Shifts the bits of the first operand to the left by the second operand, storing the result in the first operand.\
Sets the Zero Flag if the result is zero; clears it otherwise.\
Sets the Carry Flag if the leftmost set bit was shifted out; clears it otherwise.\
Clears the Sign Flag and the Overflow Flag.
```
SHL [Rn], [Rn]
SHL [Rn], [immediate]
SHL [Rn], [&Rn]
SHL [Rn], [&immediate]
```
<br>

#### 0x21: SHR (Bitwise Shift Right)
Shifts the bits of the first operand to the right by the second operand, storing the result in the first operand.\
Sets the Zero Flag if the result is zero; clears it otherwise.\
Sets the Carry Flag if the rightmost set bit was shifted out; clears it otherwise.\
Clears the Sign Flag and the Overflow Flag.
```
SHR [Rn], [Rn]
SHR [Rn], [immediate]
SHR [Rn], [&Rn]
SHR [Rn], [&immediate]
```
<br>

#### 0x22: CMP (Compare)
Compares the first operand to the second.\
Sets the Zero Flag if the operands are equal; clears it otherwise.\
Sets the Carry Flag if the first operand is less than the second; clears it otherwise.\
Clears the Sign Flag and the Overflow Flag.
```
CMP [Rn], [Rn]
CMP [Rn], [immediate]
CMP [Rn], [&Rn]
CMP [Rn], [&immediate]
```
<br>

#### 0x23: CPS (Compare Signed)
Compares the first operand to the second using two's complement arithmetic.\
Sets the Zero Flag if the operands are equal; clears it otherwise.\
Sets the Carry Flag if the first operand is less than the second; clears it otherwise.\
Sets the Sign Flag if the first operand is less than the second; clears it otherwise.\
Sets the Overflow Flag if operand 1 - operand 2 would result in an underflow; clears it otherwise.
```
CPS [Rn], [Rn]
CPS [Rn], [immediate]
CPS [Rn], [&Rn]
CPS [Rn], [&immediate]
```
<br>

## System Calls
### I/O Calls
#### 0x00: Read
Reads a number of bytes from a file into a buffer.\
R1: number of bytes to read [in], number of bytes read [out]\
R2: address of the buffer [in]\
R3: file descriptor [in]
```
# read 5 bytes from stdin into a buffer at &0x2450
LDW R0, 0
LDW R1, 5
LDW R2, 0x2450
LDW R3, 0
SYS
```
<br>

#### 0x01: Write
Writes a number of bytes from a buffer to a file.\
R1: number of bytes to write [in], number of bytes written [out]\
R2: address of the buffer [in]\
R3: file descriptor [in]
```
# write 5 bytes from a buffer at &0x2450 to stdout
LDW R0, 1
LDW R1, 5
LDW R2, 0x2450
LDW R3, 0
SYS
```
<br>

#### 0x02: Get Character
Reads a byte from a file.
R1: file descriptor [in], the read byte [out]
```
# read the next byte from stdin
LDW R0, 2
LDW R1, 0
SYS
```
<br>

#### 0x03: Put Character
Writes a byte to a file.
R1: file descriptor [in]\
R2: the byte to write [in]
```
# write 'a' to stdout
LDW R0, 3
LDW R1, 1
LDW R2, 'a'
SYS
```
<br>

#### 0x04: Seek
Repositions the offset of a file.
R1: file descriptor [in]\
R2: offset [in]\
R3: 0 for SET (offset relative to beginning), 1 for CUR (offset relative to current), 2 for END (offset relative to end) [in]
```
# move to end of file whose descriptor is held in R3
LDW R0, 4
LDW R1, R3
LDW R2, 0
LDW R3, 2
SYS
```
<br>

#### 0x05: Tell
Retrieves the offset of a file.
R1: file descriptor [in], offset [out]
```
# get the offset of file whose descriptor is held in R3
LDW R0, 5
LDW R1, R3
SYS
```
<br>

#### 0x06: Open
Opens the file at the specified path.\
R1: address of a buffer containing the null-terminated path [in], file descriptor [out]\
R2: 0 for O_CREAT (create new file if it doesn't exist), any other value for O_OPEN (return invalid descriptor if it doesn't exist) [in]
```
# open file at './resources/script.r'
STB 0x2000, "resources/script.r\0"
LDW R0, 6
LDW R1, 0x2000
LDW R2, 1
SYS
```
<br>

#### 0x07: Close
Closes a file.
R1: file descriptor [in]
```
# close file whose descriptor is held in R3
LDW R0, 7
LDW R1, R3
SYS
```
<br>

### Utility Calls
#### 0x08: Seed Random
Seeds the system's random number generator with a specified seed.
R1: seed [in]
```
# seed the random number generator with the value in R1
LDW R0, 8
LDW R1, 38292
SYS
```
<br>

#### 0x09: Random
Generates a random word using the system's random number generator.
R1: generated random word [out]
```
# generate a random number between 0 and 65535
LDW R0, 9
SYS
```
<br>

#### 0x0A: Time
Retrieves the number of seconds since the Unix Epoch (a 32-bit number).\
R1: upper word of the time [out]\
R2: lower word of the time [out]
```
# get the current time in seconds since the Unix Epoch
LDW R0, 10
SYS
```
<br>

#### 0x0B: Sleep
Sleeps for a specified number of milliseconds.\
R1: number of milliseconds to sleep [in]
```
# sleep for 1000 milliseconds
LDW R0, 11
LDW R1, 1000
SYS
```
<br>

#### 0x0C: Exit
Exits the program with the specified exit code.\
R1: exit code [in]
```
# exit the program with the code 0
LDW R0, 12
LDW R1, 0
SYS
```