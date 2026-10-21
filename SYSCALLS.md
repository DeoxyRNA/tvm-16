## TVM-16 Official System Call Table

The system calls supported by the official TVM-16 runtime environment:

### Standard Input/Output
The following system calls are used for reading from and writing to files, as well as managing file descriptors. The system supports files up to 64 KiB in size. 

#### `0x00`: Read
Reads a sequence of bytes from an open file descriptor into a buffer in memory.
| Register | Use |
|---|---|
| `R1` | in: file descriptor |
| `R2` | in: pointer to a buffer |
| `R3` | in: number of bytes to read; out: number of bytes successfully read |
***

#### `0x01`: Write
Writes a sequence of bytes from a buffer in memory to an open file descriptor.
| Register | Use |
|---|---|
| `R1` | in: file descriptor |
| `R2` | in: pointer to a buffer |
| `R3` | in: number of bytes to write; out: number of bytes successfully written |
***

#### `0x02`: Open
Opens a file and returns a file descriptor for subsequent read/write operations.
| Register | Use |
|---|---|
| `R1` | in: pointer to null-terminated string containing the file path |
| `R2` | in: flags |
| `R3` | out: file descriptor |

`R2` is a bitmask that specifies the mode in which the file should be opened. The following flags are supported:
| Flag | Value | Description |
|---|---|---|
| `READ` | `0x01` | Open the file for reading |
| `WRITE` | `0x02` | Open the file for writing |
| `CREATE` | `0x04` | Create the file if it does not already exist |
***

#### `0x03`: Close
Closes an open file descriptor.
| Register | Use |
|---|---|
| `R1` | in: file descriptor; out: error code |
***

#### `0x04`: Seek
Moves the file pointer of an open file descriptor to a specified position.
| Register | Use |
|---|---|
| `R1` | in: file descriptor; out: error code |
| `R2` | in: offset |
| `R3` | in: whence |

`R2` is a signed 16-bit integer representing the number of bytes to move the file pointer. `R3` specifies the reference point for the offset, which can be one of the following values:
| Flag | Value | Description |
|---|---|---|
| `SET` | `0x00` | seek from the beginning of the file |
| `CURRENT` | `0x01` | seek from the current position |
| `END` | `0x02` | seek from the end of the file |
***

#### `0x05`: Tell
Retrieves the current position of the file pointer for an open file descriptor.
| Register | Use |
|---|---|
| `R1` | in: file descriptor; out: current position |
| `R2` | out: error code |
***

### Process Management

#### `0x06`: Exit
Terminates the current process and returns an exit code to the operating system.
| Register | Use |
|---|---|
| `R1` | in: exit code |
***

#### `0x07`: Sleep
Suspends the current process for a specified number of milliseconds (resolution varies by platform).
| Register | Use |
|---|---|
| `R1` | in: low word of number of milliseconds to sleep |
| `R2` | in: high word of number of milliseconds to sleep |
***

### Utility Functions

#### `0x08`: Time
Retrieves the current Unix Timestamp (seconds since January 1, 1970).
| Register | Use |
|---|---|
| `R1` | out: low word of the current Unix Timestamp |
| `R2` | out: high word of the current Unix Timestamp |
***

#### `0x09`: Seed Random
Updates the seed of the system's random number generator.
| Register | Use |
|---|---|
| `R1` | in: the new seed |

#### `0x0A`: Random
Generates a random word using the system's random number generator.
| Register | Use |
|---|---|
| `R1` | out: a random 16-bit value |