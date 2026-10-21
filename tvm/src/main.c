#include <stdlib.h>

#include "vm.h"

int main(int argc, char** argv) {
    (void)argc;

    const char* path = "file.tvm";
    if (tvmLoadExecutable(path) != EXIT_SUCCESS) {
        return EXIT_FAILURE;
    }

    return tvmRun(argv);
}
