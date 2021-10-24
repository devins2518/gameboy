#include "gamegirl.h"
#include <stdio.h>

int main(int argc, char **argv) {
    gamegirl gg;

    (void)argv;
    gg = gamegirl_init();
    if (argc != TRUE) {
        printf("%d takes no arguments.\n", gg.cpu.sp);
        return 1;
    }

    while (TRUE) {
        gamegirl_clock(&gg);
    }

    return 0;
}
