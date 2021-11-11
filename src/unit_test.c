#include "gamegirl.h"
#include <assert.h>

int main(void) {
    gamegirl gg;

    gg = gamegirl_init(NULL);

    while (!gg.cpu.halted) {
        gamegirl_clock(&gg);
    }

    return 0;
}
