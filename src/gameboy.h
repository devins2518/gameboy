#ifndef GAMEGIRL_H
#define GAMEGIRL_H

#include "bus.h"
#include "cpu.h"
#include "ppu.h"

typedef struct gamegirl {
    bool step;
    cpu cpu;
    ppu ppu;
    bus bus;
    int32_t schedule_clocks;
} gamegirl;

gamegirl *gamegirl_init();

void gamegirl_clock(gamegirl *gg);
void gamegirl_free(gamegirl gg);

#endif
