#include <stdlib.h>
#include <stdio.h>

#include "vm.h"

int main(int argc, char** argv) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <executable> [args...]\n", argv[0]);
        return EXIT_FAILURE;
    }

    const char* path = argv[1];
    if (tvmLoadExecutable(path) != EXIT_SUCCESS) {
        return EXIT_FAILURE;
    }

    return tvmRun(argv);
}