#include "gamegirl.h"
#include "bus.h"
#include "cartridge.h"
#include "cpu.h"

gamegirl gamegirl_init(char *path) {
    gamegirl g;
    cartridge_t cart;
    cart = cartridge_new(path);
    g.bus = bus_new(cart);
    g.cpu = cpu_new(&g.bus);
    return g;
}

void gamegirl_clock(gamegirl *gg) { cpu_clock(&gg->cpu); }
void gamegirl_free(gamegirl gg) { bus_free(gg.bus); }