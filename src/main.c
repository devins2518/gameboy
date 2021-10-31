#include "gamegirl.h"
#include "utils.h"
#include <libgen.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main(int argc, char **argv) {
    gamegirl gg;
    char *path;

    signal(SIGSEGV, panic_handler);
    signal(SIGABRT, panic_handler);

    if (argc > 1) {
        path = argv[1];
    } else {
        path = NULL;
    }
    gg = gamegirl_init(path);
    free(path);

    while (TRUE) {
        gamegirl_clock(&gg);
        printf("gg.clocks %d\n", gg.cpu.clocks);
    }

    return 0;
}
