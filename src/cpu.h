#ifndef CPU_H
#define CPU_H

#include "bus.h"
#include "utils.h"
#include <stdbool.h>
#include <stdint.h>

typedef struct cpu {
    uint8_t registers[8];
    uint16_t pc;
    uint16_t sp;
    uint8_t halted;
    bus *bus;
    uint16_t clocks;
} cpu;

cpu cpu_init(bus *bus);

void cpu_clock(cpu *self);

#endif
