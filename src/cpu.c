#include "cpu.h"
#include "stdio.h"
#include "utils.h"
#include <string.h>

#define GET_REG_A self->registers[0]
#define GET_REG_F self->registers[1]
#define GET_REG_B self->registers[2]
#define GET_REG_C self->registers[3]
#define GET_REG_D self->registers[4]
#define GET_REG_E self->registers[5]
#define GET_REG_H self->registers[6]
#define GET_REG_L self->registers[7]
#define GET_REG_AF (self->registers[0] << 8 | self->registers[1])
#define GET_REG_BC (self->registers[2] << 8 | self->registers[3])
#define GET_REG_DE (self->registers[4] << 8 | self->registers[5])
#define GET_REG_HL (self->registers[6] << 8 | self->registers[7])

#define SET_REG_A(n) self->registers[0] = ((n));
#define SET_REG_F(n) self->registers[1] = ((n));
#define SET_REG_B(n) self->registers[2] = ((n));
#define SET_REG_C(n) self->registers[3] = ((n));
#define SET_REG_D(n) self->registers[4] = ((n));
#define SET_REG_E(n) self->registers[5] = ((n));
#define SET_REG_H(n) self->registers[6] = ((n));
#define SET_REG_L(n) self->registers[7] = ((n));
#define SET_REG_AF(n)                                                          \
    GET_REG_A = n >> 8;                                                        \
    GET_REG_F = n & 0x00FF;
#define SET_REG_BC(n)                                                          \
    GET_REG_B = n >> 8;                                                        \
    GET_REG_C = n & 0x00FF;
#define SET_REG_DE(n)                                                          \
    GET_REG_D = n >> 8;                                                        \
    GET_REG_E = n & 0x00FF;
#define SET_REG_HL(n)                                                          \
    GET_REG_H = n >> 8;                                                        \
    GET_REG_L = n & 0x00FF;

void noop(cpu *self);
void ld_u8(cpu *self, register_t reg, uint8_t val);
void ld_u16(cpu *self, register_t reg, uint16_t val);

#define FUNC(a)                                                                \
    switch ((a)) {                                                             \
    case 0x00:                                                                 \
        noop(self);                                                            \
        break;                                                                 \
    }

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
    v = get_address(self->bus, self->pc++);
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
    FUNC(opcode);
    printf("%X", self->pc);
    /* PANIC(); */
}

void noop(cpu *self) { self->clocks++; }
