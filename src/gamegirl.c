#include "bus.h"
#include "cpu.h"
#include "gamegirl.h"

gamegirl gamegirl_init() {
    gamegirl g;
    g.bus = bus_init();
    g.cpu = cpu_init(&g.bus);
    return g;
}

void gamegirl_clock(gamegirl *gg) { cpu_clock(&gg->cpu); }
