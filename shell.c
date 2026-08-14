#include "shell.h"

#define ERRMSG_BUFFER_SIZE 2048
#define GETLN_BUFFER_SIZE 1024

static char errmsg[ERRMSG_BUFFER_SIZE];

const char* const cmd_strings[] = {
    "help",
    "moveto",
    "list",
    "newfile",
    "newdir",
    "dltdir",
    "dltdir",
    "run",
    "quit"
};

const shell_cmd_t cmds[] = {
    shell_help,
    shell_moveto,
    shell_list,
    shell_newfile,
    shell_newdir,
    shell_dltfile,
    shell_dltdir,
    shell_run,
    shell_quit
};

static int cmd_cnt() {
    return sizeof(cmd_strings) / sizeof(char*);
}

int shell_loop() {
    fprintf(stdout, "%s>\t", SHELL_CURRENT_WORKING_DIRECTORY);

    char* ln = shell_getln();
    int argc;
    char** cmd = shell_parseln(ln, &argc);

    free(ln);
    free(cmd);

    return 1;
}

char* shell_getln() {
    size_t bfsz = GETLN_BUFFER_SIZE;
    size_t pos = 0;
    char* buffer = (char*)malloc(sizeof(char) * bfsz);
    char c;

    if (buffer == NULL) {
        fprintf(stderr, "shell: could not allocate memory for input buffer\n");
        return NULL;
    }

    while (1) {
        c = fgetc(stdin);

        if (c == EOF || c == '\n') {
            buffer[pos] = 0;
            return buffer;
        } else {
            buffer[pos] = c;
        }
        ++pos;

        if (pos >= bfsz) {
            bfsz <<= 2;
            buffer = (char*)realloc(buffer, sizeof(char) * bfsz);
            if (buffer == NULL) {
                fprintf(stderr, "shell: could not allocate memory for input buffer\n");
                return NULL;
            }
        }
    }
}

extern int shell_help(int argc, char** argv) {
    if (argc > 1) {
        fprintf(stderr, "error: too many arguments for command 'help'\n");
        return 1;
    }

    fprintf(
        stdout,
        "dltdir - delete directory\n"
        "dltfile - delete file\n"
        "help - print a list of commands\n"
        "moveto - move to a directory\n"
        "newdir - create new directory\n"
        "newfile - create new file\n"
        "quit - quit TVM-16\n"
        "run - run a program\n"
    );
}

extern int shell_moveto(int argc, char** argv) {
    if (argc > 2) {
        fprintf(stderr, "error: too many arguments for command 'moveto'\n");
        return 1;
    }

    if (!change_wd(argv[1])) {
        fprintf(stderr, "error: at '%s'; no such directory\n");
        return 1;
    }

    return 1;
}

extern int shell_newfile(int argc, char** argv) {
    if (argc > 2) {
        fprintf(stderr, "error: too many arguments for command 'newfile'\n");
        return 1;
    }

    FILE* file = fopen(argv[1], "w");
    if (file == NULL) {
        fprintf(stderr, "error: failed to create file '%s'\n", argv[1]);
        return 1;
    }

    fclose(file);
    return 1;
}

extern int shell_newdir(int argc, char** argv) {
    if (argc > 2) {
        fprintf(stderr, "error: too many arguments for command 'newdir'\n");
        return 1;
    }

    if (!make_dir(argv[1])) {
        fprintf(stderr, "error: failed to create directory '%s'\n", argv[1]);
        return 1;
    }

    return 1;
}

extern int shell_delete(int argc, char** argv) {
    if (argc > 2) {
        fprintf(stderr, "error: too many arguments for command 'newdir'\n");
        return 1;
    }

    if (!make_dir(argv[1])) {
        fprintf(stderr, "error: failed to create directory '%s'\n", argv[1]);
        return 1;
    }

    return 1;
}