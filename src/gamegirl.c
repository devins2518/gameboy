#include "gamegirl.h"
#include <stdlib.h>

gamegirl *gamegirl_init(char *path) {
    gamegirl *g = malloc(sizeof(gamegirl));
    cartridge_t cart;
    cart = cartridge_new(path);
    g->step = TRUE;
    g->bus = bus_new(cart);
    g->ppu = ppu_new(&g->bus);
    g->cpu = cpu_new(&g->bus);
    return g;
}

void gamegirl_clock(gamegirl *gg) { cpu_clock(&gg->cpu); }
void gamegirl_free(gamegirl gg) { bus_free(gg.bus); }
