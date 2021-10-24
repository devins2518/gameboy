#include "cpu.h"
#include "stdio.h"
#include "utils.h"
#include <string.h>

#define REG_A cpu->registers[0]
#define REG_F cpu->registers[1]
#define REG_B cpu->registers[2]
#define REG_C cpu->registers[3]
#define REG_D cpu->registers[4]
#define REG_E cpu->registers[5]
#define REG_H cpu->registers[6]
#define REG_L cpu->registers[7]
#define REG_AF (cpu->registers[0] << 8 | cpu->registers[1])
#define REG_BC (cpu->registers[2] << 8 | cpu->registers[3])
#define REG_DE (cpu->registers[4] << 8 | cpu->registers[5])
#define REG_HL (cpu->registers[6] << 8 | cpu->registers[7])

const uint8_t arg_table[512];

cpu cpu_init(bus *bus) {
    cpu c;
    memset(c.registers, 0, 8 * sizeof(uint8_t));
    c.pc = 0x0000;
    c.sp = 0xFFFE;
    c.halted = FALSE;
    c.bus = bus;
    c.clocks = 0x0000;
    return c;
}

uint8_t next_instruction(cpu *self) {
    uint8_t v;
    v = get_address(self->bus, self->pc);
    self->clocks++;
    return v;
}

uint8_t imm_u8(cpu *self) {
    (void)self;
    return 0;
}

uint16_t imm_u16(cpu *self) {
    (void)self;
    return 0;
}

void cpu_clock(cpu *self) {
    const uint8_t opcode = next_instruction(self);
    printf("%d", arg_table[opcode]);
    /* PANIC(); */
}
