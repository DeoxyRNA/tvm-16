#include "shell.h"

#define ERRMSG_BUFFER_SIZE 2048
#define GETLN_BUFFER_SIZE 1024
#define PARSELN_BUFFER_SIZE 64

static char errmsg[ERRMSG_BUFFER_SIZE];
static char bin_dir[2048];
static char current_wd[2048];

static int cmd_cd(int argc, char* argv[]) ;
static int cmd_help(int argc, char* argv[]);
static int cmd_quit(int argc, char* argv[]);
static int cmd_run(int argc, char* argv[]);

const char* const cmd_strings[] = {
    "cd",
    "help",
    "quit",
};

const shell_cmd_t cmd_funcs[] = {
    cmd_cd,
    cmd_help,
    cmd_quit,
    cmd_run
};

static int path_is_within(const char* path, const char* root) {
    size_t root_len = strlen(root);

    if (strcmp(path, root) == 0) {
        return 1;
    }

    if (strncmp(path, root, root_len) == 0 &&
        (path[root_len] == '/' || path[root_len] == '\0')) {
        return 1;
    }

    return 0;
}

static void get_binary_dir(char* out, size_t size) {
#if defined(_WIN32)
    GetModuleFileNameA(NULL, out, (DWORD)size);
    char* last_sls = strrchr(out, '\\');
    if (last_sls != NULL) {
        *last_sls = '\0';
    }
#else
    ssize_t len = readlink("/proc/self/exe", out, size - 1);
    if (len != -1) {
        out[len] = '\0';
        char* last_sls = strrchr(out, '/');
        if (last_sls) {
            *last_sls = '\0';
        }
    }
#endif
}

int shell_loop() {

    char* ln = NULL;
    char** argv = NULL;
    int argc = 0;
    int exit = 0;

    get_binary_dir(bin_dir, sizeof(bin_dir));

    while (!exit) {
        getcwd(current_wd, sizeof(current_wd));
        char* bin = strstr(current_wd, bin_dir);
        if (bin != NULL) {
            memmove(bin, bin + strlen(bin_dir), strlen(bin + strlen(bin_dir)) + 1);
        }

        fprintf(stdout, "tvm16:~%s$ ", current_wd);
        ln = shell_getln();
        if (ln == NULL) {
            break;
        }

        argv = shell_parseln(ln, &argc);
        exit = shell_exec_cmd(argc, argv);
        free(ln);
        free(argv);
    }

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
            bfsz += GETLN_BUFFER_SIZE;
            buffer = (char*)realloc(buffer, sizeof(char) * bfsz);
            if (buffer == NULL) {
                fprintf(stderr, "shell: could not allocate memory for input buffer\n");
                exit(EXIT_FAILURE);
            }
        }
    }
}

char** shell_parseln(char* ln, int* cnt) {
    *cnt = 0;
    size_t bfsz = PARSELN_BUFFER_SIZE;
    size_t pos = 0;
    char** buffer = malloc(sizeof(*buffer) * bfsz);
    char* current_tok;

    if (buffer == NULL) {
        fprintf(stderr, "shell: could not allocate memory for token buffer\n");
        exit(EXIT_FAILURE);
    }

    current_tok = strtok(ln, " \t\r\n");
    while (current_tok != NULL) {
        if (pos >= bfsz) {
            size_t new_size = bfsz + PARSELN_BUFFER_SIZE;
            char** new_buffer = realloc(buffer, sizeof(*new_buffer) * new_size);
            if (new_buffer == NULL) {
                fprintf(stderr, "shell: could not allocate memory for token buffer\n");
                free(buffer);
                exit(EXIT_FAILURE);
            }
            buffer = new_buffer;
            bfsz = new_size;
        }

        buffer[pos++] = current_tok;
        current_tok = strtok(NULL, " \t\r\n");
    }

    buffer[pos] = NULL;
    *cnt = (int)pos;
    return buffer;
}

int shell_exec_cmd(int argc, char* argv[]) {
    if (argv[0] == NULL) {
        return 0;
    }

    for (int i = 0; i < shell_cmd_cnt(); ++i) {
        if (strcmp(argv[0], cmd_strings[i]) == 0) {
            return cmd_funcs[i](argc, argv);
        }
    }

    fprintf(stdout, "'%s': no such command\n", argv[0]);
    return 0;
}

int shell_cmd_cnt() {
    return sizeof(cmd_strings) / sizeof(char*);
}

/* -- cmds -- */

int cmd_cd(int argc, char* argv[]) {
    if (strcmp(bin_dir, current_wd) == 0) {
        return 0;
    }

    if (argc > 2) {
        fprintf(stderr, "error: too many arguments for command 'cd'\n");
        return 0;
    } else if (argc < 2) {
        chdir(bin_dir);
        return 0;
    }

    char resolved[2048];
    if (realpath(argv[1], resolved) == NULL) {
        return 0;
    }

    if (!path_is_within(resolved, bin_dir)) {
        return 0;
    }

    errno = 0;
    if (chdir(argv[1]) == -1) {
        switch (errno) {
            case ENOENT:
                fprintf(stderr, "error: at '%s', no such directory\n", argv[1]);
                return 0;

            case EACCES:
                fprintf(stderr, "error: at '%s', permission denied\n", argv[1]);
                return 0;

            case ENOTDIR:
                fprintf(stderr, "error: '%s' is not a directory\n", argv[1]);
                return 0;
        }
    }

    return 0;
}


int cmd_help(int argc, char* argv[]) {
    if (argc > 1) {
        fprintf(stderr, "error: too many arguments for command 'help'\n");
        return 0;
    }

    fprintf(
        stdout,
"tinyshell v%d.%d.%d\n"
"\n"
"cd [directory]                 move to a directory\n"
"help                           print a list of commands\n"
"quit                           exit tvm-16\n"
"run [filename] [...args]       run a program\n"
"\n",
        SHELL_VERSION_MAJOR,
        SHELL_VERSION_MINOR,
        SHELL_VERSION_PATCH
    );

    return 0;
}

int cmd_quit(int argc, char* argv[]) {
    if (argc > 1) {
        fprintf(stderr, "error: too many arguments for command 'quit'\n");
        return 0;
    }

    return 1;
}

int cmd_run(int argc, char* argv[]) {
    if (argc < 2) {
        fprintf(stderr, "error: too few arguments for command 'run'\n");
        return 0;
    }

    
    return 0;
}