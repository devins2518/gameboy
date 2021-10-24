#include "gamegirl.h"
#include <stdio.h>

int main(int argc, char **argv) {
    gamegirl gg;

    (void)argv;
    gg = gamegirl_init();
    gamegirl_clock(&gg);
    if (argc != TRUE) {
        printf("%X takes no arguments.\n", gg.cpu.pc);
        return 1;
    }

    while (TRUE) {
        gamegirl_clock(&gg);
    }

    return 0;
}
