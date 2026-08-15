#include "shell.h"
#include "vm.h"

int main() {
    fprintf(
        stdout,
" _______     ____  __       _  __   \n"
"|_   _\\ \\   / /  \\/  |     / |/ /_  \n"
"  | |  \\ \\ / /| |\\/| |_____| |  _ \\ \n"
"  | |   \\ V / | |  | |_____| | (_) |\n"
"  |_|    \\_/  |_|  |_|     |_|\\___/ \n"
"\n"
"type 'help' for a list of commands.\n"
"\n"
    );

    shell_loop();
    return 0;
}