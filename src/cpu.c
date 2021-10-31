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
#define GET_REG_PAF get_address(self->bus, GET_REG_AF)
#define GET_REG_PBC get_address(self->bus, GET_REG_BC)
#define GET_REG_PDE get_address(self->bus, GET_REG_DE)
#define GET_REG_PHL get_address(self->bus, GET_REG_HL)
#define GET_REG_AF (self->registers[0] << 8 | self->registers[1])
#define GET_REG_BC (self->registers[2] << 8 | self->registers[3])
#define GET_REG_DE (self->registers[4] << 8 | self->registers[5])
#define GET_REG_HL (self->registers[6] << 8 | self->registers[7])
#define GET_REG_SP self->sp
#define GET_REG_PC self->pc

#define SET_REG(r, n)                                                          \
    switch ((r.type)) {                                                        \
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
    case p:                                                                    \
        write_address(self->bus, (r).payload, (n));                            \
        break;                                                                 \
    default:                                                                   \
        fflush(stdout);                                                        \
        fprintf(stderr, "Could not assign to type: %s\n",                      \
                ARGUMENT_NAME[(r.type)]);                                      \
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

#define RESOLVE_PAYLOAD(rhs)                                                   \
    switch (rhs.type) {                                                        \
    case io_offset:                                                            \
        rhs.payload += 0xFF00;                                                 \
        break;                                                                 \
    case sp_offset:                                                            \
        rhs.payload += GET_REG_SP;                                             \
        break;                                                                 \
    default:                                                                   \
        break;                                                                 \
    };

cpu cpu_new(bus *bus) {
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
    return (uint16_t)(b2 << 8) | b1;
}

void noop(cpu *self) { self->clocks++; }

/* Caller is required to set lhs.type and rhs.payload */
void ld(cpu *self, argument_t lhs, argument_t rhs) {
    SET_REG(lhs, rhs.payload);
}

void cpu_clock(cpu *self) {
    const uint8_t opcode = next_instruction(self);
    argument_t lhs;
    argument_t rhs;
    switch (opcode) {
    case 0x00:
        noop(self);
        break;
    case 0x0A:
        lhs.type = a;
        rhs.payload = GET_REG_PBC;
        ld(self, lhs, rhs);
        break;
    case 0x1A:
        lhs.type = a;
        rhs.payload = GET_REG_PDE;
        ld(self, lhs, rhs);
        break;
    case 0x2A:
        lhs.type = a;
        rhs.payload = GET_REG_PHL;
        ld(self, lhs, rhs);
        SET_REG_HL(GET_REG_HL + 1)
        break;
    case 0x3A:
        lhs.type = a;
        rhs.payload = GET_REG_PHL;
        ld(self, lhs, rhs);
        SET_REG_HL(GET_REG_HL - 1)
        break;
    case 0x3E:
        lhs.type = a;
        rhs.payload = get_imm_u8(self);
        ld(self, lhs, rhs);
        break;
    case 0x78:
        lhs.type = a;
        rhs.payload = GET_REG_B;
        ld(self, lhs, rhs);
        break;
    case 0x79:
        lhs.type = a;
        rhs.payload = GET_REG_C;
        ld(self, lhs, rhs);
        break;
    case 0x7A:
        lhs.type = a;
        rhs.payload = GET_REG_D;
        ld(self, lhs, rhs);
        break;
    case 0x7B:
        lhs.type = a;
        rhs.payload = GET_REG_E;
        ld(self, lhs, rhs);
        break;
    case 0x7C:
        lhs.type = a;
        rhs.payload = GET_REG_H;
        ld(self, lhs, rhs);
        break;
    case 0x7D:
        lhs.type = a;
        rhs.payload = GET_REG_L;
        ld(self, lhs, rhs);
        break;
    case 0x7E:
        lhs.type = a;
        rhs.payload = GET_REG_PHL;
        ld(self, lhs, rhs);
        break;
    case 0x7F:
        lhs.type = a;
        rhs.payload = GET_REG_A;
        ld(self, lhs, rhs);
        break;
    case 0xF0:
        lhs.type = a;
        rhs.type = io_offset;
        rhs.payload = get_imm_u8(self);
        RESOLVE_PAYLOAD(rhs);
        ld(self, lhs, rhs);
        break;
    case 0x06:
        lhs.type = b;
        rhs.payload = get_imm_u8(self);
        ld(self, lhs, rhs);
        break;
    case 0x40:
        lhs.type = b;
        rhs.payload = GET_REG_B;
        ld(self, lhs, rhs);
        break;
    case 0x41:
        lhs.type = b;
        rhs.payload = GET_REG_C;
        ld(self, lhs, rhs);
        break;
    case 0x42:
        lhs.type = b;
        rhs.payload = GET_REG_D;
        ld(self, lhs, rhs);
        break;
    case 0x43:
        lhs.type = b;
        rhs.payload = GET_REG_E;
        ld(self, lhs, rhs);
        break;
    case 0x44:
        lhs.type = b;
        rhs.payload = GET_REG_H;
        ld(self, lhs, rhs);
        break;
    case 0x45:
        lhs.type = b;
        rhs.payload = GET_REG_L;
        ld(self, lhs, rhs);
        break;
    case 0x46:
        lhs.type = b;
        rhs.payload = GET_REG_L;
        ld(self, lhs, rhs);
        break;
    case 0x47:
        lhs.type = b;
        rhs.payload = GET_REG_A;
        ld(self, lhs, rhs);
        break;
    case 0x0E:
        lhs.type = c;
        rhs.payload = get_imm_u8(self);
        ld(self, lhs, rhs);
        break;
    case 0x48:
        lhs.type = c;
        rhs.payload = GET_REG_B;
        ld(self, lhs, rhs);
        break;
    case 0x49:
        lhs.type = c;
        rhs.payload = GET_REG_C;
        ld(self, lhs, rhs);
        break;
    case 0x4A:
        lhs.type = c;
        rhs.payload = GET_REG_D;
        ld(self, lhs, rhs);
        break;
    case 0x4B:
        lhs.type = c;
        rhs.payload = GET_REG_E;
        ld(self, lhs, rhs);
        break;
    case 0x4C:
        lhs.type = c;
        rhs.payload = GET_REG_H;
        ld(self, lhs, rhs);
        break;
    case 0x4D:
        lhs.type = c;
        rhs.payload = GET_REG_L;
        ld(self, lhs, rhs);
        break;
    case 0x4E:
        lhs.type = c;
        rhs.payload = GET_REG_L;
        ld(self, lhs, rhs);
        break;
    case 0x4F:
        lhs.type = c;
        rhs.payload = GET_REG_A;
        ld(self, lhs, rhs);
        break;
    case 0x16:
        lhs.type = d;
        rhs.payload = get_imm_u8(self);
        ld(self, lhs, rhs);
        break;
    case 0x50:
        lhs.type = d;
        rhs.payload = GET_REG_B;
        ld(self, lhs, rhs);
        break;
    case 0x51:
        lhs.type = d;
        rhs.payload = GET_REG_C;
        ld(self, lhs, rhs);
        break;
    case 0x52:
        lhs.type = d;
        rhs.payload = GET_REG_D;
        ld(self, lhs, rhs);
        break;
    case 0x53:
        lhs.type = d;
        rhs.payload = GET_REG_E;
        ld(self, lhs, rhs);
        break;
    case 0x54:
        lhs.type = d;
        rhs.payload = GET_REG_H;
        ld(self, lhs, rhs);
        break;
    case 0x55:
        lhs.type = d;
        rhs.payload = GET_REG_L;
        ld(self, lhs, rhs);
        break;
    case 0x56:
        lhs.type = d;
        rhs.payload = GET_REG_L;
        ld(self, lhs, rhs);
        break;
    case 0x57:
        lhs.type = d;
        rhs.payload = GET_REG_A;
        ld(self, lhs, rhs);
        break;
    case 0x1E:
        lhs.type = e;
        rhs.payload = get_imm_u8(self);
        ld(self, lhs, rhs);
        break;
    case 0x58:
        lhs.type = e;
        rhs.payload = GET_REG_B;
        ld(self, lhs, rhs);
        break;
    case 0x59:
        lhs.type = e;
        rhs.payload = GET_REG_C;
        ld(self, lhs, rhs);
        break;
    case 0x5A:
        lhs.type = e;
        rhs.payload = GET_REG_D;
        ld(self, lhs, rhs);
        break;
    case 0x5B:
        lhs.type = e;
        rhs.payload = GET_REG_E;
        ld(self, lhs, rhs);
        break;
    case 0x5C:
        lhs.type = e;
        rhs.payload = GET_REG_H;
        ld(self, lhs, rhs);
        break;
    case 0x5D:
        lhs.type = e;
        rhs.payload = GET_REG_L;
        ld(self, lhs, rhs);
        break;
    case 0x5E:
        lhs.type = e;
        rhs.payload = GET_REG_PHL;
        ld(self, lhs, rhs);
        break;
    case 0x5F:
        lhs.type = e;
        rhs.payload = GET_REG_A;
        ld(self, lhs, rhs);
        break;
    case 0x26:
        lhs.type = h;
        rhs.payload = get_imm_u8(self);
        ld(self, lhs, rhs);
        break;
    case 0x60:
        lhs.type = h;
        rhs.payload = GET_REG_B;
        ld(self, lhs, rhs);
        break;
    case 0x61:
        lhs.type = h;
        rhs.payload = GET_REG_C;
        ld(self, lhs, rhs);
        break;
    case 0x62:
        lhs.type = h;
        rhs.payload = GET_REG_D;
        ld(self, lhs, rhs);
        break;
    case 0x63:
        lhs.type = h;
        rhs.payload = GET_REG_E;
        ld(self, lhs, rhs);
        break;
    case 0x64:
        lhs.type = h;
        rhs.payload = GET_REG_H;
        ld(self, lhs, rhs);
        break;
    case 0x65:
        lhs.type = h;
        rhs.payload = GET_REG_L;
        ld(self, lhs, rhs);
        break;
    case 0x66:
        lhs.type = h;
        rhs.payload = GET_REG_PHL;
        ld(self, lhs, rhs);
        break;
    case 0x67:
        lhs.type = h;
        rhs.payload = GET_REG_A;
        ld(self, lhs, rhs);
        break;
    case 0x2E:
        lhs.type = l;
        rhs.payload = get_imm_u8(self);
        ld(self, lhs, rhs);
        break;
    case 0x68:
        lhs.type = l;
        rhs.payload = GET_REG_B;
        ld(self, lhs, rhs);
        break;
    case 0x69:
        lhs.type = l;
        rhs.payload = GET_REG_C;
        ld(self, lhs, rhs);
        break;
    case 0x6A:
        lhs.type = l;
        rhs.payload = GET_REG_D;
        ld(self, lhs, rhs);
        break;
    case 0x6B:
        lhs.type = l;
        rhs.payload = GET_REG_E;
        ld(self, lhs, rhs);
        break;
    case 0x6C:
        lhs.type = l;
        rhs.payload = GET_REG_H;
        ld(self, lhs, rhs);
        break;
    case 0x6D:
        lhs.type = l;
        rhs.payload = GET_REG_L;
        ld(self, lhs, rhs);
        break;
    case 0x6E:
        lhs.type = l;
        rhs.payload = GET_REG_PHL;
        ld(self, lhs, rhs);
        break;
    case 0x6F:
        lhs.type = l;
        rhs.payload = GET_REG_A;
        ld(self, lhs, rhs);
        break;
    case 0x02:
        lhs.type = pbc;
        rhs.payload = GET_REG_A;
        ld(self, lhs, rhs);
        break;
    case 0x12:
        lhs.type = pde;
        rhs.payload = GET_REG_A;
        ld(self, lhs, rhs);
        break;
    case 0x70:
        lhs.type = phl;
        rhs.payload = GET_REG_B;
        ld(self, lhs, rhs);
        break;
    case 0x71:
        lhs.type = phl;
        rhs.payload = GET_REG_C;
        ld(self, lhs, rhs);
        break;
    case 0x72:
        lhs.type = phl;
        rhs.payload = GET_REG_D;
        ld(self, lhs, rhs);
        break;
    case 0x73:
        lhs.type = phl;
        rhs.payload = GET_REG_E;
        ld(self, lhs, rhs);
        break;
    case 0x74:
        lhs.type = phl;
        rhs.payload = GET_REG_H;
        ld(self, lhs, rhs);
        break;
    case 0x75:
        lhs.type = phl;
        rhs.payload = GET_REG_L;
        ld(self, lhs, rhs);
        break;
    case 0x77:
        lhs.type = phl;
        rhs.payload = GET_REG_A;
        ld(self, lhs, rhs);
        break;
    case 0x36:
        lhs.type = phl;
        rhs.payload = get_imm_u8(self);
        ld(self, lhs, rhs);
        break;
    case 0x22:
        lhs.type = phl;
        rhs.payload = GET_REG_A;
        ld(self, lhs, rhs);
        SET_REG_HL(GET_REG_HL + 1);
        break;
    case 0x32:
        lhs.type = phl;
        rhs.payload = GET_REG_A;
        ld(self, lhs, rhs);
        SET_REG_HL(GET_REG_HL - 1);
        break;
    case 0xEA:
        lhs.type = p;
        lhs.payload = get_imm_u16(self);
        rhs.payload = GET_REG_A;
        ld(self, lhs, rhs);
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
    case 0xF9:
        lhs.type = sp;
        rhs.payload = GET_REG_HL;
        ld(self, lhs, rhs);
        break;
    case 0xE0:
        lhs.type = io_offset;
        lhs.payload = get_imm_u8(self);
        RESOLVE_PAYLOAD(lhs);
        rhs.payload = GET_REG_A;
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
    default:
        PANIC("Unhandled opcode");
        break;
    }
}
