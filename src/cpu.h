#ifndef CPU_H
#define CPU_H

#include "utils.h"
#include <stdint.h>

typedef struct cpu {
    afreg af;
    reg bc;
    reg de;
    reg hl;
    uint16_t pc;
    uint16_t sp;
} cpu;

cpu cpu_init();

#endif
