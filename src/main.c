#include "gamegirl.h"
#include <stdio.h>

int main(int argc, char **argv) {
    gamegirl gb = gamegirl_init();
    if (argc != 1) {
        printf("%d takes no arguments.\n", gb.cpu.bc.lo);
        return 1;
    }
    return 0;
}
