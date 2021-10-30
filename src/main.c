#include "gamegirl.h"
#include "utils.h"
#include <signal.h>
#include <stdio.h>

int main(int argc, char **argv) {
    signal(SIGSEGV, panic_handler);
    signal(SIGABRT, panic_handler);
    gamegirl gg;

    (void)argc;
    gg = gamegirl_init(argv[1]);
    gamegirl_clock(&gg);

    while (TRUE) {
        gamegirl_clock(&gg);
    }

    return 0;
}
