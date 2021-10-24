#ifndef CPU_H
#define CPU_H

#include "utils.h"
#include <stdbool.h>
#include <stdint.h>

typedef struct cpu {
    uint8_t registers[8];
    uint16_t pc;
    uint16_t sp;
    uint8_t halted;
} cpu;

cpu cpu_init();

void cpu_clock(cpu *self);

#endif
