#include "cpu.h"
#include "gamegirl.h"

gamegirl gamegirl_init() {
    gamegirl g;
    g.cpu = cpu_init();
    return g;
}

void gamegirl_clock(gamegirl *gg) { cpu_clock(&gg->cpu); }
