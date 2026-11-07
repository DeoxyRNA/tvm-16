#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <tvm/env.h>

#include "vm.h"

const char* INVOCATION_NAME;

int main(int argc, char** argv) {
    // get file name from path
    const char* invocationName = strrchr(argv[0], '/');
    if (invocationName == NULL) {
        invocationName = strrchr(argv[0], '\\');
    }
    if (invocationName == NULL) {
        invocationName = argv[0];
    } else {
        invocationName++;
    }

    INVOCATION_NAME = invocationName;

    if (argc < 2) {
        fprintf(stderr, "%s: fatal error: no input file specified\n", INVOCATION_NAME);
        return EXIT_FAILURE;
    }
    
    const char* path = argv[1];
    if (tvmLoadExecutable(path) != EXIT_SUCCESS) {
        return EXIT_FAILURE;
    }

    return tvmRun(argv);
}