#ifndef GAMEGIRL_H
#define GAMEGIRL_H

#include "cpu.h"
#include <stdint.h>

typedef struct gamegirl {
    cpu cpu;
} gamegirl;

gamegirl gamegirl_init() {
    gamegirl g = {.cpu = cpu_init()};
    return g;
}

#endif
