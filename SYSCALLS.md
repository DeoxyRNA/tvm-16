## TVM-16 Official System Call Table

The system calls supported by the official TVM-16 runtime environment:

### Input/Output

#### `0x00`: Read
Reads a sequence of bytes to a buffer in memory from the standard input stream.
| Register | Use |
|---|---|
| `R1` | in: pointer to a buffer |
| `R2` | in: number of bytes to read; out: number of bytes successfully read |
***

#### `0x01`: Write
Writes a sequence of bytes from a buffer in memory to the standard output stream.
| Register | Use |
|---|---|
| `R1` | in: pointer to a buffer |
| `R2` | in: number of bytes to write; out: number of bytes successfully written |
***

### Process Management

#### `0x02`: Exit
Terminates the current process and returns an exit code to the operating system.
| Register | Use |
|---|---|
| `R1` | in: exit code |
***

#### `0x03`: Sleep
Suspends the current process for a specified number of milliseconds (resolution varies by platform).
| Register | Use |
|---|---|
| `R1` | in: low word of number of milliseconds to sleep |
| `R2` | in: high word of number of milliseconds to sleep |
***

### Utility Functions

#### `0x04`: Time
Retrieves the current Unix Timestamp (seconds since January 1, 1970).
| Register | Use |
|---|---|
| `R1` | out: low word of the current Unix Timestamp |
| `R2` | out: high word of the current Unix Timestamp |
***

#### `0x05`: Seed Random
Updates the seed of the system's random number generator.
| Register | Use |
|---|---|
| `R1` | in: low word of the new seed |
| `R2` | in: high word of the new seed |
***

#### `0x06`: Random
Generates a random word using the system's random number generator.
| Register | Use |
|---|---|
| `R1` | out: a random 16-bit value |