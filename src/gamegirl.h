#ifndef GAMEGIRL_H
#define GAMEGIRL_H

#include "cpu.h"
#include <stdint.h>

typedef struct gamegirl {
    cpu cpu;
} gamegirl;

gamegirl gamegirl_init();

void gamegirl_clock(gamegirl *gg);

#endif
