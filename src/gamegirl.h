#ifndef GAMEGIRL_H
#define GAMEGIRL_H

#include "bus.h"
#include "cpu.h"
#include <stdint.h>

typedef struct gamegirl {
    cpu cpu;
    bus bus;
} gamegirl;

gamegirl gamegirl_init();

void gamegirl_clock(gamegirl *gg);
void gamegirl_free(gamegirl gg);

#endif
