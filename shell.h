#pragma once

#if defined(_WIN32)
    #include <direct.h>
    #include <sys/stat.h>

    #define getcwd _getcwd
    #define chdir _chdir
    #define mkdir(path) _mkdir(path)
    #define rmdir _rmdir
    
    #if defined(_WIN64)
        #define stat _stat64
    #else
        #define stat _stat32
    #endif
#else
    #include <unistd.h>
    #include <sys/stat.h>

    #define getcwd getcwd
    #define chdir chdir
    #define mkdir(path) mkdir(path, 0777) // code 0777 grants universal perms
    #define rmdir rmdir
    #define stat stat
#endif

#define SHELL_VERSION_MAJOR 0
#define SHELL_VERSION_MINOR 0
#define SHELL_VERSION_PATCH 1

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef int (*shell_cmd_t)(int, char**);

/// gets a command from the user and executes it
/// returns 1 on success, 0 on failure
extern int shell_loop(void);

/// gets a line of input from the user
/// the returned string is allocated on the heap and must be freed by the caller
/// this is called within shell_loop(), avoid calling it on its own
extern char* shell_getln(void);

/// parses a line into tokens
/// the returned array is allocated on the heap and must be freed by the caller
/// returns the argument count in cnt if it is not NULL
/// this is called within shell_loop(), avoid calling it on its own
extern char** shell_parseln(char* ln, int* cnt);

/// print a list of commands
extern int shell_help(int argc, char* argv[]);

/// quit tvm-16
extern int shell_quit(int argc, char* argv[]);

// attempts to execute a command from a list of arguments
extern int shell_exec_cmd(int argc, char* argv[]);

/// returns the number of built-in commands
extern int shell_cmd_cnt();