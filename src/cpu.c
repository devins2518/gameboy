#include "cpu.h"
#include "stdio.h"
#include "utils.h"
#include <stdlib.h>
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
#define GET_REG_SP self->sp
#define GET_REG_PC self->pc

#define SET_REG(r, n)                                                          \
    switch ((r)) {                                                             \
    case a:                                                                    \
        SET_REG_A((n));                                                        \
        break;                                                                 \
    case f:                                                                    \
        SET_REG_F((n));                                                        \
        break;                                                                 \
    case b:                                                                    \
        SET_REG_B((n));                                                        \
        break;                                                                 \
    case c:                                                                    \
        SET_REG_C((n));                                                        \
        break;                                                                 \
    case d:                                                                    \
        SET_REG_D((n));                                                        \
        break;                                                                 \
    case e:                                                                    \
        SET_REG_E((n));                                                        \
        break;                                                                 \
    case h:                                                                    \
        SET_REG_H((n));                                                        \
        break;                                                                 \
    case l:                                                                    \
        SET_REG_L((n));                                                        \
        break;                                                                 \
    case af:                                                                   \
        SET_REG_AF((n));                                                       \
        break;                                                                 \
    case bc:                                                                   \
        SET_REG_BC((n));                                                       \
        break;                                                                 \
    case de:                                                                   \
        SET_REG_DE((n));                                                       \
        break;                                                                 \
    case hl:                                                                   \
        SET_REG_HL((n));                                                       \
        break;                                                                 \
    case sp:                                                                   \
        SET_REG_SP((n));                                                       \
        break;                                                                 \
    case pc:                                                                   \
        SET_REG_PC((n));                                                       \
        break;                                                                 \
    default:                                                                   \
        fflush(stdout);                                                        \
        fprintf(stderr, "Could not assign to type: %s\n", ARGUMENT_NAME[(r)]); \
        abort();                                                               \
    }
#define SET_ARG_PAYLOAD(arg)                                                   \
    switch ((arg.type)) {                                                      \
    case a:                                                                    \
        arg.payload = (uint16_t)GET_REG_A;                                     \
        break;                                                                 \
    case f:                                                                    \
        arg.payload = (uint16_t)GET_REG_F;                                     \
        break;                                                                 \
    case b:                                                                    \
        arg.payload = (uint16_t)GET_REG_B;                                     \
        break;                                                                 \
    case c:                                                                    \
        arg.payload = (uint16_t)GET_REG_C;                                     \
        break;                                                                 \
    case d:                                                                    \
        arg.payload = (uint16_t)GET_REG_D;                                     \
        break;                                                                 \
    case e:                                                                    \
        arg.payload = (uint16_t)GET_REG_E;                                     \
        break;                                                                 \
    case h:                                                                    \
        arg.payload = (uint16_t)GET_REG_H;                                     \
        break;                                                                 \
    case l:                                                                    \
        arg.payload = (uint16_t)GET_REG_L;                                     \
        break;                                                                 \
    case af:                                                                   \
        arg.payload = GET_REG_AF;                                              \
        break;                                                                 \
    case bc:                                                                   \
        arg.payload = GET_REG_BC;                                              \
        break;                                                                 \
    case de:                                                                   \
        arg.payload = GET_REG_DE;                                              \
        break;                                                                 \
    case hl:                                                                   \
        arg.payload = GET_REG_HL;                                              \
        break;                                                                 \
    case sp:                                                                   \
        arg.payload = GET_REG_SP;                                              \
        break;                                                                 \
    case pc:                                                                   \
        arg.payload = GET_REG_PC;                                              \
        break;                                                                 \
    default:                                                                   \
        break;                                                                 \
    }

#define SET_REG_A(n) self->registers[0] = ((n));
#define SET_REG_F(n) self->registers[1] = ((n));
#define SET_REG_B(n) self->registers[2] = ((n));
#define SET_REG_C(n) self->registers[3] = ((n));
#define SET_REG_D(n) self->registers[4] = ((n));
#define SET_REG_E(n) self->registers[5] = ((n));
#define SET_REG_H(n) self->registers[6] = ((n));
#define SET_REG_L(n) self->registers[7] = ((n));
#define SET_REG_SP(n) self->sp = (n);
#define SET_REG_PC(n) self->pc = (n);
#define SET_REG_AF(n)                                                          \
    GET_REG_A = (n) >> 8;                                                      \
    GET_REG_F = (n)&0x00FF;
#define SET_REG_BC(n)                                                          \
    GET_REG_B = (n) >> 8;                                                      \
    GET_REG_C = (n)&0x00FF;
#define SET_REG_DE(n)                                                          \
    GET_REG_D = (n) >> 8;                                                      \
    GET_REG_E = (n)&0x00FF;
#define SET_REG_HL(n)                                                          \
    GET_REG_H = (n) >> 8;                                                      \
    GET_REG_L = (n)&0x00FF;

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

uint8_t get_imm_u8(cpu *self) { return next_instruction(self); }

uint16_t get_imm_u16(cpu *self) {
    uint8_t b1 = next_instruction(self);
    uint8_t b2 = next_instruction(self);
    return (uint16_t)(b1 << 8) | b2;
}

void noop(cpu *self) { self->clocks++; }

// Caller is required to set lhs.type and rhs.payload
void ld(cpu *self, argument_t lhs, argument_t rhs) {
    SET_REG(lhs.type, rhs.payload);
}

void cpu_clock(cpu *self) {
    const uint8_t opcode = next_instruction(self);
    argument_t lhs;
    argument_t rhs;
    switch (opcode) {
    case 0x00:
        noop(self);
        break;
    case 0x01:
        lhs.type = bc;
        rhs.payload = get_imm_u16(self);
        ld(self, lhs, rhs);
        break;
    case 0x11:
        lhs.type = de;
        rhs.payload = get_imm_u16(self);
        ld(self, lhs, rhs);
        break;
    case 0x21:
        lhs.type = hl;
        rhs.payload = get_imm_u16(self);
        ld(self, lhs, rhs);
        break;
    case 0x31:
        lhs.type = sp;
        rhs.payload = get_imm_u16(self);
        ld(self, lhs, rhs);
        break;
    }
    /* PANIC(); */
}
