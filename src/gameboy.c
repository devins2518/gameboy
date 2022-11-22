#include "gameboy.h"
#include <stdlib.h>

gamegirl *gamegirl_init(char *path) {
    gamegirl *gg = malloc(sizeof(gamegirl));
    cartridge_t cart;
    cart = cartridge_new(path);
    gg->step = true;
    gg->bus = bus_new(cart);
    gg->ppu = ppu_new(&gg->bus);
    gg->cpu = cpu_new(&gg->bus);
    gg->schedule_clocks = 0;
    return gg;
}

void gamegirl_clock(gamegirl *gg) {
    if (gg->schedule_clocks >= 0)
        gg->schedule_clocks -= (cpu_clock(&gg->cpu) * 2); /* Convert m-cycles to t-cycles */
    else
        gg->schedule_clocks += ppu_clock(&gg->ppu);
}
void gamegirl_free(gamegirl gg) { bus_free(gg.bus); }
