#pragma once

#if defined(_WIN32)
#include <direct.h>

#define get_wd _getcwd
#define change_wd _chdir
#define make_dir _mkdir(path)
#else
#include <unistd.h>
#include <sys/stat.h>

#define get_wd getcwd
#define change_wd chdir
#define make_dir(path) mkdir(path, 0777) // code 0777 grants universal perms
#endif

#include <stdio.h>
#include <stdlib.h>

typedef int (*shell_cmd_t)(int, char**);

/// current working directory relative to the tvm16 executable
const char* SHELL_CURRENT_WORKING_DIRECTORY;

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

/// prints a list of commands
extern int shell_help(int argc, char** argv);

/// move to directory
extern int shell_moveto(int argc, char** argv);

/// list contents of current directory
extern int shell_list(int argc, char** argv);

/// create new file
extern int shell_newfile(int argc, char** argv);

/// create new directory
extern int shell_newdir(int argc, char** argv);

/// delete file
extern int shell_dltfile(int argc, char** argv);

/// delete directory
extern int shell_dltdir(int argc, char** argv);

// run a tvm-16 program
extern int shell_run(int argc, char** argv);

// quit tvm-16
extern int shell_quit(int argc, char** argv);