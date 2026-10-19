# TVM-16 Implementation Specifications

## Contents
1. [Architecture](#architecture)
   - [Overview](#overview)
   - [Registers](#registers)
   - [Status Flags](#status-flags)
   - [Instruction Layout](#instruction-layout)
   - [Calling Convention](#calling-convention)
2. [Assembler](#assembler)
   - [Syntax](#syntax)
   - [Literals](#literals)
   - [Labels](#labels)
   - [Dereferencing](#dereferencing)
   - [Segments](#segments)
3. [Instruction Set](#instruction-set)
   - [State Instructions](#state-instructions)
   - [Branch Instructions](#branch-instructions)
   - [Arithmetic and Logic Instructions](#arithmetic-and-logic-instructions)
4. [System Calls](#system-calls)
   - [I/O Calls](#io-calls)
   - [Utility Calls](#utility-calls)

## Architecture

### Overview

TVM-16 has 64 KiB of random-access memory, addressable from `0x0000` to `0xFFFF`. Each memory location stores one byte. A word is exactly 16 bits and occupies two consecutive memory locations.

Memory uses little-endian byte order: the low byte is stored at the lower address and the high byte at the higher address.

| Address | Byte value |
| :- | :- |
| `0x2000` | `0x39` |
| `0x2001` | `0x30` |

The example above encodes the word `0x3039`.

The stack grows downward (toward lower addresses) from `0x9FFF`. The code segment begins at `0xA000` and grows upward (toward higher addresses). The stack and code segments are adjacent, and because the stack grows away from the code segment, ordinary stack growth should not overwrite program code; a stack that grows too far can only run into the DATA or STATIC segments (see [Segments](#segments)).

### Registers

The CPU has eight 16-bit registers. Registers provide the intermediary between the CPU and memory; values cannot be copied directly from one memory address to another.

| Register | ID | Purpose |
| :- | :- | :- |
| `R0` | `0` | general purpose; syscall ID and low return word |
| `R1` | `1` | general purpose; high return word |
| `R2` | `2` | general purpose |
| `R3` | `3` | general purpose |
| `SP` | `4` | stack pointer |
| `BP` | `5` | base pointer |
| `PC` | `6` | program counter |
| `FL` | `7` | status flags |

At program start:

- `PC` is initialized to `0xA000`.
- `BP` is initialized to `0x9FFF`.
- Command-line arguments are pushed onto the stack before execution begins.
- `SP` is initialized to the top of the stack **after** the command-line arguments have been pushed. With no arguments, `SP` equals `BP` (`0x9FFF`).
- `R0`, `R1`, `R2`, `R3`, and `FL` are initialized to `0x0000`.

`SP` always points at the most recently pushed value. A push first decrements `SP` by the size of the value and then writes the value at the new `SP`; a pop reads the value at `SP` and then increments `SP`. The stack-frame offsets in [Calling Convention](#calling-convention) depend on this behavior.

Unless control flow changes `PC`, it advances by 6 bytes after each instruction. Execution stops when the program halts.

### Status Flags

The lower four bits of `FL` contain the status flags:

| Flag | Bit mask | Meaning |
| :- | :- | :- |
| `ZF` | `0x0001` | result is zero |
| `CF` | `0x0002` | carry or borrow occurred, as defined by the instruction |
| `SF` | `0x0004` | signed result is negative |
| `OF` | `0x0008` | signed overflow or underflow occurred |

The upper 12 bits of `FL` are reserved. Arithmetic and logic instructions update the flags listed in their descriptions and never modify the reserved bits. `FL` is not preserved across subroutine calls.

### Instruction Layout

Each instruction occupies 6 bytes:

| Byte(s) | Contents |
| :- | :- |
| `0` | variant |
| `1` | opcode |
| `2–3` | first operand, often called the "destination" or "destination operand" |
| `4–5` | second operand, often called the "source" or "source operand" |

For example:

```
ADD R2, @0xAFC8
```

is encoded as:

```
03 16 02 00 C8 AF
```

This decodes as variant `0x03`, opcode `ADD` (`0x16`), operand 1 `R2` (`0x0002`), and operand 2 `0xAFC8`. The assembler encodes unused operand fields as zero, and the decoder ignores them.

### Calling Convention

A call creates a stack frame. The caller pushes word arguments from right to left, and the callee establishes and destroys its own frame. The caller removes the arguments after the call; `RET` removes only the return address. `CLL` only pushes the return address; it does not modify `BP`.

#### Register Roles

| Register | Role |
| :- | :- |
| `R0` | low word of return value; caller-saved |
| `R1` | high word of return value; caller-saved |
| `R2` | scratch; caller-saved |
| `R3` | addressing scratch; caller-saved |
| `SP` | stack pointer; callee-saved |
| `BP` | base pointer; callee-saved |
| `PC` | instruction pointer; not modified directly by ordinary code |
| `FL` | status flags; not preserved across calls |

Only `SP` and `BP` are guaranteed to survive a call. A caller that needs `R0`–`R3` after a call must save them explicitly, for example by pushing them onto the stack.

#### Argument Passing

Arguments are pushed right-to-left:

```
PSW arg2
PSW arg1
CLL my_func
ADD SP, 4       # caller cleanup for two word arguments
```

The return value is placed in `R0` for a 16-bit result. For a 32-bit result, the low word is in `R0` and the high word is in `R1`.

#### Stack Frame

After the conventional prologue for a subroutine with two word-sized arguments, offsets relative to `BP` are:

| Offset | Contents |
| :- | :- |
| `+6` | `arg2` |
| `+4` | `arg1` |
| `+2` | return address |
| `+0` | saved caller `BP` |
| `-2` | local 1 |
| `-4` | local 2 |

The conventional prologue is:

```
my_func:
    PSW BP          # save caller's BP
    LDW BP, SP      # BP points to the saved caller BP
    SUB SP, N       # reserve N bytes for locals, if needed
```

After the prologue, `BP + 2` is the return address; `BP + 4`, `BP + 6`, and so on hold `arg1`, `arg2`, and so on; and negative offsets hold locals.

The conventional epilogue is:

```
    LDW SP, BP      # discard locals
    PPW BP          # restore caller's BP
    RET             # pop return address and return
```

`R1` holds the multiplicand, and `R2` is the number of additions remaining. The initial `SUB` checks for a zero multiplier because loads do not update the status flags. Repeated addition naturally discards any bits above the low word, so the returned result is truncated to 16 bits.

## Assembler

TAsm (TVM-16 Assembly) maps machine instructions to human-readable mnemonics.

### Syntax

```
[mnemonic] [destination], [source]
```

Comments begin with `#` and continue to the end of the line.

```
# comment
LDW R0, 0x4392       # trailing comment
```

### Literals

Number literals may be written in decimal, hexadecimal, octal, or binary:

```
PSW 13920
PSW 0x3660
PSW 033140
PSW 0b0011011001100000
```

Character literals use single quotes:

```
PSB 'a'
```

String literals use double quotes. The assembler expands them into individual byte operations. Only `STB` and `PSB` accept string literals. Strings are **not** implicitly null-terminated; include `\0` explicitly when a terminator is required.

```
STB 0x3320, "hello"

# equivalent expansion
STB 0x3320, 'h'
STB 0x3321, 'e'
STB 0x3322, 'l'
STB 0x3323, 'l'
STB 0x3324, 'o'

# null-terminated string
STB 0x3320, "hello\0"
```

### Labels

A label resolves to the address of the instruction that follows it.

```
start:
    ADD R0, 5
    BNC done

done:
    HLT
```

Local labels begin with `.` and are scoped to the most recent global label. The full name of a local label is the global label followed by the local name, so `.loop` under `countdown` is `countdown.loop`. Either form may be used as a branch target:

```
countdown:
    LDW R0, 10
.loop:
    SUB R0, 1
    BNE .loop           # same as BNE countdown.loop
```

### Dereferencing

The `@` prefix treats its operand as a memory address and loads or stores through that address.

```
LDW R0, @0x2000    # R0 = the word stored at address 0x2000
LDW R0, @label     # R0 = the word stored at the address of label
LDW R3, 0x2000     # R3 = the value 0x2000 (no @, so no memory access)
LDW R0, @R3        # R0 = the word stored at the address held in R3
```

Store instructions (`STW`, `STB`) are the exception: their destination operand is always interpreted as an address, so it is written without `@` (for example `STW 0x2000, R0` or `STW R3, R0`).

### Segments

Programs contain four segments. STATIC starts at `0x0000` and DATA follows it. The stack occupies the region ending at `0x9FFF`, and CODE starts at `0xA000`. The region between DATA and the stack is unreserved; if the stack grows past it, it overwrites DATA or STATIC.

```
0x0000 ┌───────────────────────────────┐
       │ STATIC: zero-initialized      │
       │ grows toward higher addresses │
       ├───────────────────────────────┤
       │ DATA: initialized values      │
       │ grows toward higher addresses │
       ├───────────────────────────────┤
       │ unreserved                    │
       ├───────────────────────────────┤
       │ STACK: frames and arguments   │
       │ grows toward lower addresses  │
0x9FFF ├───────────────────────────────┤
0xA000 │ CODE: executable instructions │
       │ grows toward higher addresses │
0xFFFF └───────────────────────────────┘
```

Examples in this document use literal addresses such as `0x2000` for brevity. In a real program, use STATIC/DATA symbols, or make sure literal addresses do not overlap those segments.

#### STATIC

`RES` (reserve) reserves zero-initialized bytes and assigns the symbol an address. The syntax is `RES name, size`, with the size in bytes:

```
SEGMENT STATIC:
    RES a, 2
    RES b, 10
```

```
LDW R0, @a     # load the word stored in a
STW b, R0      # store R0 at the address of b
```

#### DATA

`DFW` (define word) and `DFB` (define byte) define initialized variables. `ASN` (assign) defines a constant immediate value:

```
SEGMENT DATA:
    DFW a, 0x1234
    DFB b, 'x'
    ASN size, 10
```

```
LDW R0, @a            # R0 = 0x1234 (the value stored in a)
LDW R0, size          # R0 = 10 (the constant)
STW 0x2000, size      # store 10 at address 0x2000
```

#### CODE

```
SEGMENT CODE:
main:
    PSW 0x2000
    HLT
```

#### Executable Space
The variables of the DATA segment are initialized at program start, so their initial values are stored in the program's executable file. The sizes of the both the STATIC and DATA segments must also be stored in the program's executable file, so that the runtime environment knows where to start reading the instructions from when it loads the program into memory.

The executable file format is as follows:
| Section | Size (bytes) | Description |
| :- | :- | :- |
| Magic Bytes | 4 | ASCII "pTVM" |
| STATIC size | 2 | The size of the STATIC segment in bytes |
| DATA size | 2 | The size of the DATA segment in bytes |
| DATA contents | varies | The initial values of the DATA segment, in order of definition |
| CODE contents | varies | The instructions of the CODE segment, in order of definition |

For example, given the following program:
```
SEGMENT STATIC:
    RES a, 2
    RES b, 10

SEGMENT DATA:
    DFW a, 0x1234
    DFB b, 'x'

SEGMENT CODE:
main:
    LDW R0, @a
    STW b, R0
    HLT
```

The executable file would look like this (translated to hexadecimal for readability):
```
70 54 56 4D 0C 00 03 00
34 12 78 03 01 00 00 00
00 01 03 02 00 00 00
```

When the program is loaded into memory, the CPU reads the first 4 bytes to verify that the file is a valid TVM-16 executable. It then reads the next 2 bytes to determine the size of the STATIC segment, and the following 2 bytes to determine the size of the DATA segment. The CPU then initializes the DATA segment with the values provided in the executable file, and finally loads the CODE segment into memory starting at address `0xA000`. The CPU then sets `PC` to `0xA000` and begins executing instructions.

## Instruction Set

Each instruction's **variant** (byte 0 of the encoding) selects which operand forms it takes. Operand notation used below:

| Notation | Meaning |
| :- | :- |
| `[Rn]` | a register |
| `[immediate]` | a literal value, constant, or label |
| `[@Rn]` | the memory at the address held in a register |
| `[@immediate]` | the memory at a literal address |

Most two-operand instructions (`LDW`, `LDB`, and the arithmetic, logic, and compare instructions) share one variant pattern: `0x0` = register, register; `0x1` = register, immediate; `0x2` = register, `@`register; `0x3` = register, `@`immediate. Store instructions and single-operand instructions differ and are listed individually.

### State Instructions
These instructions move data between registers, memory, and the stack, and control the program and its host environment (`HLT`, `SYS`).

#### `0x00`: NOP (No Operation)
Does nothing.

| Variant | Usage |
| :- | :- |
| `0x0` | `NOP` |
***
#### `0x01`: LDW (Load Word)
Loads a word into a register.

| Variant | Usage |
| :- | :- |
| `0x0` | `LDW [Rn], [Rn]` |
| `0x1` | `LDW [Rn], [immediate]` |
| `0x2` | `LDW [Rn], [@Rn]` |
| `0x3` | `LDW [Rn], [@immediate]` |
***
#### `0x02`: LDB (Load Byte)
Loads a byte into the low byte of a register.\
Preserves the high byte.

| Variant | Usage |
| :- | :- |
| `0x0` | `LDB [Rn], [Rn]` |
| `0x1` | `LDB [Rn], [immediate]` |
| `0x2` | `LDB [Rn], [@Rn]` |
| `0x3` | `LDB [Rn], [@immediate]` |
***
#### `0x03`: STW (Store Word)
Stores a word at a memory location. Operand 1 is the destination address; operand 2 is the value to store.

| Variant | Usage |
| :- | :- |
| `0x0` | `STW [Rn], [Rn]` |
| `0x1` | `STW [Rn], [immediate]` |
| `0x2` | `STW [immediate], [Rn]` |
| `0x3` | `STW [immediate], [immediate]` |
***
#### `0x04`: STB (Store Byte)
Stores a byte at a memory location. Operand 1 is the destination address; operand 2 is the value to store.

| Variant | Usage |
| :- | :- |
| `0x0` | `STB [Rn], [Rn]` |
| `0x1` | `STB [Rn], [immediate]` |
| `0x2` | `STB [immediate], [Rn]` |
| `0x3` | `STB [immediate], [immediate/string literal]` |
***
#### `0x05`: PSW (Push Word)
Decreases the stack pointer by `2`, then stores the word at the new stack pointer.

| Variant | Usage |
| :- | :- |
| `0x0` | `PSW [Rn]` |
| `0x1` | `PSW [immediate]` |
***
#### `0x06`: PSB (Push Byte)
Decreases the stack pointer by `1`, then stores the byte at the new stack pointer.

| Variant | Usage |
| :- | :- |
| `0x0` | `PSB [Rn]` |
| `0x1` | `PSB [immediate/string literal]` |
***
#### `0x07`: PPW (Pop Word)
Reads the word at the stack pointer into a register, then increases the stack pointer by `2`.

| Variant | Usage |
| :- | :- |
| `0x0` | `PPW [Rn]` |
***
#### `0x08`: PPB (Pop Byte)
Reads the byte at the stack pointer into the low byte of a register, then increases the stack pointer by `1`.\
Preserves the high byte.

| Variant | Usage |
| :- | :- |
| `0x0` | `PPB [Rn]` |
***
#### `0x09`: HLT (Halt)
Halts the program.

| Variant | Usage |
| :- | :- |
| `0x0` | `HLT` |
***
#### `0x0A`: SYS (System Call)
Invokes a [system call](#system-calls).

| Variant | Usage |
| :- | :- |
| `0x0` | `SYS` |

#### String Literal Operands

`STB` (`0x04`, variant `0x3`) and `PSB` (`0x06`) accept string literals, which the assembler expands into one single-byte instruction per character. For `STB`, successive characters go to successive addresses (see [Literals](#literals)). For `PSB`, the bytes are pushed in reverse, so:

```
PSB "Hello"
```

is the same as:

```
PSB 'o'
PSB 'l'
PSB 'l'
PSB 'e'
PSB 'H'
```

The stack grows toward lower addresses, so pushing the string in the original order would cause the bytes to end up reversed in memory; that is to say, `SP` would point at `'o'`, and reading upward from `SP` would yield `olleH`.

### Branch Instructions
These instructions manipulate the sequence of the program's execution.

#### `0x0B`: BRC (Branch)
Branches to an address.
`
| Variant | Usage |
| :- | :- |
| `0x0` | `BRC [Rn]` |
| `0x1` | `BRC [immediate/label]` |
| `0x2` | `BRC [@Rn]` |
| `0x3` | `BRC [@immediate]` |
***
#### `0x0C`: BEQ (Branch if Equal)
Branches to an address if `ZF` is set.

| Variant | Usage |
| :- | :- |
| `0x0` | `BEQ [Rn]` |
| `0x1` | `BEQ [immediate/label]` |
| `0x2` | `BEQ [@Rn]` |
| `0x3` | `BEQ [@immediate]` |
***
#### `0x0D`: BNE (Branch if Not Equal)
Branches to an address if `ZF` is clear.

| Variant | Usage |
| :- | :- |
| `0x0` | `BNE [Rn]` |
| `0x1` | `BNE [immediate/label]` |
| `0x2` | `BNE [@Rn]` |
| `0x3` | `BNE [@immediate]` |
***
#### `0x0E`: BCA (Branch if Carry)
Branches to an address if `CF` is set.

| Variant | Usage |
| :- | :- |
| `0x0` | `BCA [Rn]` |
| `0x1` | `BCA [immediate/label]` |
| `0x2` | `BCA [@Rn]` |
| `0x3` | `BCA [@immediate]` |
***
#### `0x0F`: BNC (Branch if Not Carry)
Branches to an address if `CF` is clear.

| Variant | Usage |
| :- | :- |
| `0x0` | `BNC [Rn]` |
| `0x1` | `BNC [immediate/label]` |
| `0x2` | `BNC [@Rn]` |
| `0x3` | `BNC [@immediate]` |
***
#### `0x10`: BMI (Branch if Minus)
Branches to an address if `SF` is set.

| Variant | Usage |
| :- | :- |
| `0x0` | `BMI [Rn]` |
| `0x1` | `BMI [immediate/label]` |
| `0x2` | `BMI [@Rn]` |
| `0x3` | `BMI [@immediate]` |
***
#### `0x11`: BPL (Branch if Plus)
Branches to an address if `SF` is clear.

| Variant | Usage |
| :- | :- |
| `0x0` | `BPL [Rn]` |
| `0x1` | `BPL [immediate/label]` |
| `0x2` | `BPL [@Rn]` |
| `0x3` | `BPL [@immediate]` |
***
#### `0x12`: BOV (Branch if Overflow)
Branches to an address if `OF` is set.

| Variant | Usage |
| :- | :- |
| `0x0` | `BOV [Rn]` |
| `0x1` | `BOV [immediate/label]` |
| `0x2` | `BOV [@Rn]` |
| `0x3` | `BOV [@immediate]` |
***
#### `0x13`: BNO (Branch if Not Overflow)
Branches to an address if `OF` is clear.

| Variant | Usage |
| :- | :- |
| `0x0` | `BNO [Rn]` |
| `0x1` | `BNO [immediate/label]` |
| `0x2` | `BNO [@Rn]` |
| `0x3` | `BNO [@immediate]` |
***
#### `0x14`: CLL (Call)
Pushes the return address (the address of the instruction after the `CLL`) onto the stack, then branches to the target address.\
Does not modify `BP`; the callee establishes its own frame (see [Calling Convention](#calling-convention)).

| Variant | Usage |
| :- | :- |
| `0x0` | `CLL [Rn]` |
| `0x1` | `CLL [immediate/label]` |
| `0x2` | `CLL [@Rn]` |
| `0x3` | `CLL [@immediate]` |
***
#### `0x15`: RET (Return)
Pops the return address off of the stack and branches to it. subroutine arguments are not removed; the caller does that.

| Variant | Usage |
| :- | :- |
| `0x0` | `RET` |

### Arithmetic and Logic Instructions
These instructions perform mathematical operations on data.

#### `0x16`: ADD (Add)
Performs an unsigned addition on the two operands.\
Stores the result in the destination.

| Flag | Condition |
| :- | :- |
| `ZF` | Result = `0` |
| `CF` | Operation produced a carry |

| Variant | Usage |
| :- | :- |
| `0x0` | `ADD [Rn], [Rn]` |
| `0x1` | `ADD [Rn], [immediate]` |
| `0x2` | `ADD [Rn], [@Rn]` |
| `0x3` | `ADD [Rn], [@immediate]` |
***
#### `0x17`: ADS (Add Signed)
Performs a signed addition on the two operands.\
Stores the result in the destination.

| Flag | Condition |
| :- | :- |
| `ZF` | Result = `0` |
| `CF` | Operation produced a carry |
| `SF` | Result < `0` |
| `OF` | Signed overflow occurred |

| Variant | Usage |
| :- | :- |
| `0x0` | `ADS [Rn], [Rn]` |
| `0x1` | `ADS [Rn], [immediate]` |
| `0x2` | `ADS [Rn], [@Rn]` |
| `0x3` | `ADS [Rn], [@immediate]` |
***
#### `0x18`: ADC (Add With Carry)
Performs an unsigned addition of the two operands and `CF` (operand 1 + operand 2 + `CF`).\
Stores the result in the destination.

| Flag | Condition |
| :- | :- |
| `ZF` | Result = `0` |
| `CF` | Operation produced a carry |

| Variant | Usage |
| :- | :- |
| `0x0` | `ADC [Rn], [Rn]` |
| `0x1` | `ADC [Rn], [immediate]` |
| `0x2` | `ADC [Rn], [@Rn]` |
| `0x3` | `ADC [Rn], [@immediate]` |
***
#### `0x19`: SUB (Subtract)
Performs an unsigned subtraction on the two operands.\
Stores the result in the destination.

| Flag | Condition |
| :- | :- |
| `ZF` | Result = `0` |
| `CF` | Operation produced a borrow |

| Variant | Usage |
| :- | :- |
| `0x0` | `SUB [Rn], [Rn]` |
| `0x1` | `SUB [Rn], [immediate]` |
| `0x2` | `SUB [Rn], [@Rn]` |
| `0x3` | `SUB [Rn], [@immediate]` |
***
#### `0x1A`: SBS (Subtract Signed)
Performs a signed subtraction on the two operands.\
Stores the result in the destination.

| Flag | Condition |
| :- | :- |
| `ZF` | Result = `0` |
| `CF` | Operation produced a borrow |
| `SF` | Result < `0`|
| `OF` | Signed overflow or underflow occurred |

| Variant | Usage |
| :- | :- |
| `0x0` | `SBS [Rn], [Rn]` |
| `0x1` | `SBS [Rn], [immediate]` |
| `0x2` | `SBS [Rn], [@Rn]` |
| `0x3` | `SBS [Rn], [@immediate]` |
***
#### `0x1B`: SBB (Subtract With Borrow)
Performs an unsigned subtraction of the two operands and `CF` (operand 1 - operand 2 - `CF`).\
Stores the result in the destination.

| Flag | Condition |
| :- | :- |
| `ZF` | Result = `0` |
| `CF` | Operation produced a borrow |

| Variant | Usage |
| :- | :- |
| `0x0` | `SBB [Rn], [Rn]` |
| `0x1` | `SBB [Rn], [immediate]` |
| `0x2` | `SBB [Rn], [@Rn]` |
| `0x3` | `SBB [Rn], [@immediate]` |
***
#### `0x1C`: AND (Bitwise AND)
Performs a bitwise AND on the two operands.\
Stores the result in the destination.

| Flag | Condition |
| :- | :- |
| `ZF` | Result = `0` |

| Variant | Usage |
| :- | :- |
| `0x0` | `AND [Rn], [Rn]` |
| `0x1` | `AND [Rn], [immediate]` |
| `0x2` | `AND [Rn], [@Rn]` |
| `0x3` | `AND [Rn], [@immediate]` |
***
#### `0x1D`: IOR (Bitwise Inclusive OR)
Performs a bitwise inclusive OR on the two operands.\
Stores the result in the destination.

| Flag | Condition |
| :- | :- |
| `ZF` | Result = `0` |

| Variant | Usage |
| :- | :- |
| `0x0` | `IOR [Rn], [Rn]` |
| `0x1` | `IOR [Rn], [immediate]` |
| `0x2` | `IOR [Rn], [@Rn]` |
| `0x3` | `IOR [Rn], [@immediate]` |
***
#### `0x1E`: XOR (Bitwise Exclusive OR)
Performs a bitwise exclusive OR on the two operands.\
Stores the result in the destination.

| Flag | Condition |
| :- | :- |
| `ZF` | Result = `0` |

| Variant | Usage |
| :- | :- |
| `0x0` | `XOR [Rn], [Rn]` |
| `0x1` | `XOR [Rn], [immediate]` |
| `0x2` | `XOR [Rn], [@Rn]` |
| `0x3` | `XOR [Rn], [@immediate]` |
***
#### `0x1F`: NOT (Bitwise NOT)
Performs a bitwise NOT on the operand and stores the result in the operand.

| Flag | Condition |
| :- | :- |
| `ZF` | Result = `0` |

| Variant | Usage |
| :- | :- |
| `0x0` | `NOT [Rn]` |
***
#### `0x20`: SHL (Bitwise Shift Left)
Shifts the bits of the operand one position to the left and stores the result in the operand.\
A clear bit is shifted in from the right.

| Flag | Condition |
| :- | :- |
| `ZF` | Result = `0` |
| `CF` | The bit shifted out (the leftmost bit of the original value) was set |

| Variant | Usage |
| :- | :- |
| `0x0` | `SHL [Rn]` |
***
#### `0x21`: SHR (Bitwise Shift Right)
Shifts the bits of the operand one position to the right and stores the result in the operand.\
A clear bit is shifted in from the left.

| Flag | Condition |
| :- | :- |
| `ZF` | Result = `0` |
| `CF` | The bit shifted out (the rightmost bit of the original value) was set |

| Variant | Usage |
| :- | :- |
| `0x0` | `SHR [Rn]` |
***
#### `0x22`: CMP (Compare)
Performs an unsigned comparison on the two operands without modifying either one.

| Flag | Condition |
| :- | :- |
| `ZF` | Operand 1 = Operand 2 |
| `CF` | Operand 1 < Operand 2 |

| Variant | Usage |
| :- | :- |
| `0x0` | `CMP [Rn], [Rn]` |
| `0x1` | `CMP [Rn], [immediate]` |
| `0x2` | `CMP [Rn], [@Rn]` |
| `0x3` | `CMP [Rn], [@immediate]` |
***
#### `0x23`: CPS (Compare Signed)
Performs a signed comparison on the two operands without modifying either one.

| Flag | Condition |
| :- | :- |
| `ZF` | Operand 1 = Operand 2 |
| `CF` | Operand 1 < Operand 2 |
| `SF` | The signed difference (operand 1 - operand 2) is negative |
| `OF` | The signed subtraction overflowed or underflowed |

| Variant | Usage |
| :- | :- |
| `0x0` | `CPS [Rn], [Rn]` |
| `0x1` | `CPS [Rn], [immediate]` |
| `0x2` | `CPS [Rn], [@Rn]` |
| `0x3` | `CPS [Rn], [@immediate]` |

## System Calls
System calls (abbreviated as "syscalls"), are operations that hand execution of the program over to the runtime environment. They can be used for input/output operations, like printing to the console or reading from a file, or miscellaneous utilities beyond the scope of the base instruction set. System calls are implementation-defined, and will differ depending on the runtime environment you use.

The system call ID is passed to the runtime in `R0`. It may also read in or write back additional data in any of the general-purpose registers (`R0`-`R3`). The exact usage of the registers is defined by the syscall ID. After the arguments are loaded, the program must invoke the syscall by executing the `SYS` instruction.
For example, a syscall that reads a file might take the file descriptor in `R1`, the buffer address in `R2`, and the number of bytes to read in `R3`. The syscall might then return the number of bytes actually read in `R0`.
```
# Example: syscall 0x00 reads from a file descriptor
LDW R0, 0
LDW R1, fd
LDW R2, buffer
LDW R3, 0x100
SYS
```

Passing an unrecognized syscall ID to the runtime environment results in undefined behavior. The runtime may ignore the syscall, return an error code, or terminate the program.