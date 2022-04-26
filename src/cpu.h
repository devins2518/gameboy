#ifndef CPU_H
#define CPU_H

#include "bus.h"
#include "utils.h"
#include <stdint.h>

typedef struct cpu {
    uint8_t registers[8];
    uint16_t pc;
    uint16_t sp;
    uint8_t halted;
    bus *bus;
    uintptr_t clocks;
} cpu;

cpu cpu_new(bus *bus);

uintptr_t cpu_clock(cpu *self);

uint16_t get_sp(cpu *self);
uint8_t get_imm_u8(cpu *self);
uint16_t get_imm_u16(cpu *self);
uint8_t get_flag_z(cpu *self);
uint8_t get_flag_n(cpu *self);
uint8_t get_flag_h(cpu *self);
uint8_t get_flag_c(cpu *self);

#endif
