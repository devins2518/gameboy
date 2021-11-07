#include "cpu.h"
#include "stdio.h"
#include "utils.h"
#include <stdlib.h>
#include <string.h>

#define GET_REG_A self->registers[0]
#define GET_REG_F self->registers[1]
#define GET_FLAG_Z ((GET_REG_F >> 7) & 0x01)
#define GET_FLAG_N ((GET_REG_F >> 6) & 0x01)
#define GET_FLAG_H ((GET_REG_F >> 5) & 0x01)
#define GET_FLAG_C ((GET_REG_F >> 4) & 0x01)
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
    case paf:                                                                  \
        write_address(self->bus, GET_REG_AF, (n));                             \
        break;                                                                 \
    case pbc:                                                                  \
        write_address(self->bus, GET_REG_BC, (n));                             \
        break;                                                                 \
    case pde:                                                                  \
        write_address(self->bus, GET_REG_DE, (n));                             \
        break;                                                                 \
    case phl:                                                                  \
        write_address(self->bus, GET_REG_HL, (n));                             \
        break;                                                                 \
    default:                                                                   \
        fflush(stdout);                                                        \
        fprintf(stderr, "Could not assign to type: %s\n",                      \
                ARGUMENT_NAME[(r.type)]);                                      \
        abort();                                                               \
    }
#define SET_FLAG_Z(n) SET_REG_F(GET_REG_F &((n) << 7))
#define SET_FLAG_N(n) SET_REG_F(GET_REG_F &((n) << 6))
#define SET_FLAG_H(n) SET_REG_F(GET_REG_F &((n) << 5))
#define ADD_FLAG_H(a, b)                                                       \
    SET_REG_F(GET_REG_F &((((((a)&0xf) + ((b)&0xf)) & 0x10) == 0x10) << 5))
#define SET_FLAG_C(n) SET_REG_F(GET_REG_F &((n) << 4))
#define ADD_FLAG_C(a, b)                                                       \
    SET_REG_F(GET_REG_F &(((((a)&0xf0) + ((b)&0xf0)) & 0x100) == 0x100 << 4))
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
    case imm_u8:                                                               \
        rhs.payload = get_imm_u8(self);                                        \
        break;                                                                 \
    default:                                                                   \
        break;                                                                 \
    };
#define RESOLVE_COND(rhs)                                                      \
    switch (rhs.cond) {                                                        \
    case zero:                                                                 \
        rhs.payload = GET_FLAG_Z == 0;                                         \
        break;                                                                 \
    case nzero:                                                                \
        rhs.payload = GET_FLAG_Z != 0;                                         \
        break;                                                                 \
    case carry:                                                                \
        rhs.payload = GET_FLAG_C == 0;                                         \
        break;                                                                 \
    case ncarry:                                                               \
        rhs.payload = GET_FLAG_C != 0;                                         \
        break;                                                                 \
    case none:                                                                 \
        rhs.payload = TRUE;                                                    \
        break;                                                                 \
    default:                                                                   \
        break;                                                                 \
    };

void handle_cb(cpu *self);

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

void inc(cpu *self, argument_t lhs, argument_t rhs) {
    uint16_t res;
    res = lhs.payload + 1;
    (void)rhs;
    SET_REG(lhs, res);
    switch (lhs.type) {
    case bc:
    case de:
    case hl:
    case sp:
        break;
    default:
        SET_FLAG_Z(res == 0);
        SET_FLAG_N(0);
        ADD_FLAG_H(lhs.payload, 1);
        break;
    }
}

void add(cpu *self, argument_t lhs, argument_t rhs) {
    uint16_t res;
    res = lhs.payload + rhs.payload;
    SET_REG(lhs, res)
    switch (lhs.type) {
    case a:
        SET_FLAG_Z(res == 0)
        break;
    case sp:
        SET_FLAG_Z(0)
        break;
    default:
        break;
    }
    SET_FLAG_N(0);
    ADD_FLAG_H(lhs.payload, rhs.payload);
    ADD_FLAG_C(lhs.payload, rhs.payload);
}

void adc(cpu *self, argument_t lhs, argument_t rhs) {
    uint16_t res;
    res = lhs.payload + rhs.payload + GET_FLAG_C;
    SET_REG(lhs, res);
    SET_FLAG_Z(res == 0);
    SET_FLAG_N(0);
    ADD_FLAG_H(lhs.payload, rhs.payload + GET_FLAG_C);
    ADD_FLAG_C(lhs.payload, rhs.payload + GET_FLAG_C);
}

void sub(cpu *self, argument_t lhs, argument_t rhs) {
    uint16_t res;
    res = lhs.payload - rhs.payload;
    SET_REG(lhs, res)
    SET_FLAG_Z(res == 0);
    SET_FLAG_N(1);
    /*
     * TODO:
     * I don't know if SET_FLAG_{H,C} properly handle subtraction
     */
    ADD_FLAG_H(lhs.payload, rhs.payload);
    ADD_FLAG_C(lhs.payload, rhs.payload);
}

void sbc(cpu *self, argument_t lhs, argument_t rhs) {
    uint16_t res;
    res = lhs.payload - rhs.payload - GET_FLAG_C;
    SET_REG(lhs, res)
    SET_FLAG_Z(res == 0);
    SET_FLAG_N(1);
    ADD_FLAG_H(lhs.payload, rhs.payload - GET_FLAG_C);
    ADD_FLAG_C(lhs.payload, rhs.payload - GET_FLAG_C);
}

void andReg(cpu *self, argument_t lhs, argument_t rhs) {
    uint16_t res;
    res = lhs.payload & rhs.payload;
    SET_REG(lhs, res)
    SET_FLAG_Z(res == 0);
    SET_FLAG_N(0);
    SET_FLAG_H(1);
    SET_FLAG_C(0);
}

void xorReg(cpu *self, argument_t lhs, argument_t rhs) {
    uint16_t res;
    res = lhs.payload ^ rhs.payload;
    SET_REG(lhs, res)
    SET_FLAG_Z(res == 0);
    SET_FLAG_N(0);
    SET_FLAG_H(0);
    SET_FLAG_C(0);
}

void orReg(cpu *self, argument_t lhs, argument_t rhs) {
    uint16_t res;
    res = lhs.payload | rhs.payload;
    SET_REG(lhs, res)
    SET_FLAG_Z(res == 0);
    SET_FLAG_N(0);
    SET_FLAG_H(0);
    SET_FLAG_C(0);
}

void cp(cpu *self, argument_t lhs, argument_t rhs) {
    uint16_t res;
    res = lhs.payload - rhs.payload;
    SET_FLAG_Z(res == 0);
    SET_FLAG_N(1);
    ADD_FLAG_H(lhs.payload, rhs.payload);
    ADD_FLAG_C(lhs.payload, rhs.payload);
}

void ret(cpu *self, argument_t lhs, argument_t rhs) {
    (void)rhs;
    if (lhs.cond) {
        SET_REG_PC((get_address(self->bus, self->sp--) << 8) |
                   (get_address(self->bus, self->sp--)));
    }
}

void jp(cpu *self, argument_t lhs, argument_t rhs) {
    (void)rhs;
    if (lhs.cond) {
        SET_REG_PC(lhs.payload);
    }
}

void cpu_clock(cpu *self) {
    const uint8_t opcode = next_instruction(self);
    argument_t lhs;
    argument_t rhs;
    switch (opcode) {
    case 0x00:
        noop(self);
        break;
    /* LD */
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
        rhs.type = imm_u8;
        RESOLVE_PAYLOAD(rhs);
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
        rhs.type = imm_u8;
        RESOLVE_PAYLOAD(rhs);
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
        rhs.type = imm_u8;
        RESOLVE_PAYLOAD(rhs);
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
        rhs.type = imm_u8;
        RESOLVE_PAYLOAD(rhs);
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
        rhs.type = imm_u8;
        RESOLVE_PAYLOAD(rhs);
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
        rhs.type = imm_u8;
        RESOLVE_PAYLOAD(rhs);
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
        rhs.type = imm_u8;
        RESOLVE_PAYLOAD(rhs);
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
        /* INC
         * Ignore uninitialized rhs since we don't use it
         */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wuninitialized"
    case 0x03:
        lhs.type = bc;
        SET_ARG_PAYLOAD(lhs);
        inc(self, lhs, rhs);
        break;
    case 0x04:
        lhs.type = b;
        SET_ARG_PAYLOAD(lhs);
        inc(self, lhs, rhs);
        break;
    case 0x0C:
        lhs.type = c;
        SET_ARG_PAYLOAD(lhs);
        inc(self, lhs, rhs);
        break;
    case 0x13:
        lhs.type = de;
        SET_ARG_PAYLOAD(lhs);
        inc(self, lhs, rhs);
        break;
    case 0x14:
        lhs.type = d;
        SET_ARG_PAYLOAD(lhs);
        inc(self, lhs, rhs);
        break;
    case 0x1C:
        lhs.type = e;
        SET_ARG_PAYLOAD(lhs);
        inc(self, lhs, rhs);
        break;
    case 0x23:
        lhs.type = hl;
        SET_ARG_PAYLOAD(lhs);
        inc(self, lhs, rhs);
        break;
    case 0x24:
        lhs.type = h;
        SET_ARG_PAYLOAD(lhs);
        inc(self, lhs, rhs);
        break;
    case 0x2C:
        lhs.type = l;
        SET_ARG_PAYLOAD(lhs);
        inc(self, lhs, rhs);
        break;
    case 0x33:
        lhs.type = sp;
        SET_ARG_PAYLOAD(lhs);
        inc(self, lhs, rhs);
        break;
    case 0x34:
        lhs.type = phl;
        SET_ARG_PAYLOAD(lhs);
        inc(self, lhs, rhs);
        break;
    case 0x3C:
        lhs.type = a;
        SET_ARG_PAYLOAD(lhs);
        inc(self, lhs, rhs);
        break;
    /* DEC */
    case 0x0B:
        lhs.type = bc;
        SET_ARG_PAYLOAD(lhs);
        inc(self, lhs, rhs);
        break;
    case 0x05:
        lhs.type = b;
        SET_ARG_PAYLOAD(lhs);
        inc(self, lhs, rhs);
        break;
    case 0x0D:
        lhs.type = c;
        SET_ARG_PAYLOAD(lhs);
        inc(self, lhs, rhs);
        break;
    case 0x1B:
        lhs.type = de;
        SET_ARG_PAYLOAD(lhs);
        inc(self, lhs, rhs);
        break;
    case 0x15:
        lhs.type = d;
        SET_ARG_PAYLOAD(lhs);
        inc(self, lhs, rhs);
        break;
    case 0x1D:
        lhs.type = e;
        SET_ARG_PAYLOAD(lhs);
        inc(self, lhs, rhs);
        break;
    case 0x2B:
        lhs.type = hl;
        SET_ARG_PAYLOAD(lhs);
        inc(self, lhs, rhs);
        break;
    case 0x25:
        lhs.type = h;
        SET_ARG_PAYLOAD(lhs);
        inc(self, lhs, rhs);
        break;
    case 0x2D:
        lhs.type = l;
        SET_ARG_PAYLOAD(lhs);
        inc(self, lhs, rhs);
        break;
    case 0x3B:
        lhs.type = sp;
        SET_ARG_PAYLOAD(lhs);
        inc(self, lhs, rhs);
        break;
    case 0x35:
        lhs.type = phl;
        SET_ARG_PAYLOAD(lhs);
        inc(self, lhs, rhs);
        break;
    case 0x3D:
        lhs.type = a;
        SET_ARG_PAYLOAD(lhs);
        inc(self, lhs, rhs);
        break;
#pragma GCC diagnostic pop
    /* ADD */
    case 0x09:
        lhs.type = hl;
        rhs.type = bc;
        SET_ARG_PAYLOAD(lhs);
        SET_ARG_PAYLOAD(rhs);
        add(self, lhs, rhs);
        break;
    case 0x19:
        lhs.type = hl;
        rhs.type = de;
        SET_ARG_PAYLOAD(lhs);
        SET_ARG_PAYLOAD(rhs);
        add(self, lhs, rhs);
        break;
    case 0x29:
        lhs.type = hl;
        rhs.type = hl;
        SET_ARG_PAYLOAD(lhs);
        SET_ARG_PAYLOAD(rhs);
        add(self, lhs, rhs);
        break;
    case 0x39:
        lhs.type = hl;
        rhs.type = sp;
        SET_ARG_PAYLOAD(lhs);
        SET_ARG_PAYLOAD(rhs);
        add(self, lhs, rhs);
        break;
    case 0x80:
        lhs.type = a;
        rhs.type = b;
        SET_ARG_PAYLOAD(lhs);
        SET_ARG_PAYLOAD(rhs);
        add(self, lhs, rhs);
        break;
    case 0x81:
        lhs.type = a;
        rhs.type = c;
        SET_ARG_PAYLOAD(lhs);
        SET_ARG_PAYLOAD(rhs);
        add(self, lhs, rhs);
        break;
    case 0x82:
        lhs.type = a;
        rhs.type = d;
        SET_ARG_PAYLOAD(lhs);
        SET_ARG_PAYLOAD(rhs);
        add(self, lhs, rhs);
        break;
    case 0x83:
        lhs.type = a;
        rhs.type = e;
        SET_ARG_PAYLOAD(lhs);
        SET_ARG_PAYLOAD(rhs);
        add(self, lhs, rhs);
        break;
    case 0x84:
        lhs.type = a;
        rhs.type = h;
        SET_ARG_PAYLOAD(lhs);
        SET_ARG_PAYLOAD(rhs);
        add(self, lhs, rhs);
        break;
    case 0x85:
        lhs.type = a;
        rhs.type = l;
        SET_ARG_PAYLOAD(lhs);
        SET_ARG_PAYLOAD(rhs);
        add(self, lhs, rhs);
        break;
    case 0x86:
        lhs.type = a;
        rhs.type = phl;
        SET_ARG_PAYLOAD(lhs);
        SET_ARG_PAYLOAD(rhs);
        add(self, lhs, rhs);
        break;
    case 0x87:
        lhs.type = a;
        rhs.type = a;
        SET_ARG_PAYLOAD(lhs);
        SET_ARG_PAYLOAD(rhs);
        add(self, lhs, rhs);
        break;
    case 0xC6:
        lhs.type = a;
        rhs.type = imm_u8;
        SET_ARG_PAYLOAD(lhs);
        SET_ARG_PAYLOAD(rhs);
        add(self, lhs, rhs);
        break;
    /* ADC */
    case 0x88:
        lhs.type = a;
        rhs.type = b;
        RESOLVE_PAYLOAD(rhs)
        adc(self, lhs, rhs);
        break;
    case 0x89:
        lhs.type = a;
        rhs.type = c;
        adc(self, lhs, rhs);
        break;
    case 0x8A:
        lhs.type = a;
        rhs.type = d;
        adc(self, lhs, rhs);
        break;
    case 0x8B:
        lhs.type = a;
        rhs.type = e;
        adc(self, lhs, rhs);
        break;
    case 0x8C:
        lhs.type = a;
        rhs.type = h;
        adc(self, lhs, rhs);
        break;
    case 0x8D:
        lhs.type = a;
        rhs.type = l;
        adc(self, lhs, rhs);
        break;
    case 0x8E:
        lhs.type = a;
        rhs.type = phl;
        adc(self, lhs, rhs);
        break;
    case 0x8F:
        lhs.type = a;
        rhs.type = a;
        adc(self, lhs, rhs);
        break;
    case 0xCE:
        lhs.type = a;
        rhs.type = imm_u8;
        RESOLVE_PAYLOAD(rhs);
        adc(self, lhs, rhs);
        break;
    /* SUB */
    case 0x90:
        lhs.type = a;
        rhs.type = b;
        RESOLVE_PAYLOAD(rhs);
        sub(self, lhs, rhs);
        break;
    case 0x91:
        lhs.type = a;
        rhs.type = c;
        RESOLVE_PAYLOAD(rhs);
        sub(self, lhs, rhs);
        break;
    case 0x92:
        lhs.type = a;
        rhs.type = d;
        RESOLVE_PAYLOAD(rhs);
        sub(self, lhs, rhs);
        break;
    case 0x93:
        lhs.type = a;
        rhs.type = e;
        RESOLVE_PAYLOAD(rhs);
        sub(self, lhs, rhs);
        break;
    case 0x94:
        lhs.type = a;
        rhs.type = h;
        RESOLVE_PAYLOAD(rhs);
        sub(self, lhs, rhs);
        break;
    case 0x95:
        lhs.type = a;
        rhs.type = l;
        RESOLVE_PAYLOAD(rhs);
        sub(self, lhs, rhs);
        break;
    case 0x96:
        lhs.type = a;
        rhs.type = phl;
        RESOLVE_PAYLOAD(rhs);
        sub(self, lhs, rhs);
        break;
    case 0x97:
        lhs.type = a;
        rhs.type = a;
        RESOLVE_PAYLOAD(rhs);
        sub(self, lhs, rhs);
        break;
    case 0xD6:
        lhs.type = a;
        rhs.type = imm_u8;
        RESOLVE_PAYLOAD(rhs);
        sub(self, lhs, rhs);
        break;
    /* SBC */
    case 0x98:
        lhs.type = a;
        rhs.type = b;
        RESOLVE_PAYLOAD(rhs);
        sbc(self, lhs, rhs);
        break;
    case 0x99:
        lhs.type = a;
        rhs.type = c;
        RESOLVE_PAYLOAD(rhs);
        sbc(self, lhs, rhs);
        break;
    case 0x9A:
        lhs.type = a;
        rhs.type = d;
        RESOLVE_PAYLOAD(rhs);
        sbc(self, lhs, rhs);
        break;
    case 0x9B:
        lhs.type = a;
        rhs.type = e;
        RESOLVE_PAYLOAD(rhs);
        sbc(self, lhs, rhs);
        break;
    case 0x9C:
        lhs.type = a;
        rhs.type = h;
        RESOLVE_PAYLOAD(rhs);
        sbc(self, lhs, rhs);
        break;
    case 0x9D:
        lhs.type = a;
        rhs.type = l;
        RESOLVE_PAYLOAD(rhs);
        sbc(self, lhs, rhs);
        break;
    case 0x9E:
        lhs.type = a;
        rhs.type = phl;
        RESOLVE_PAYLOAD(rhs);
        sbc(self, lhs, rhs);
        break;
    case 0x9F:
        lhs.type = a;
        rhs.type = a;
        RESOLVE_PAYLOAD(rhs);
        sbc(self, lhs, rhs);
        break;
    case 0xDE:
        lhs.type = a;
        rhs.type = imm_u8;
        RESOLVE_PAYLOAD(rhs);
        sbc(self, lhs, rhs);
        break;
    /* AND */
    case 0xA0:
        lhs.type = a;
        rhs.type = b;
        RESOLVE_PAYLOAD(rhs);
        andReg(self, lhs, rhs);
        break;
    case 0xA1:
        lhs.type = a;
        rhs.type = c;
        RESOLVE_PAYLOAD(rhs);
        andReg(self, lhs, rhs);
        break;
    case 0xA2:
        lhs.type = a;
        rhs.type = d;
        RESOLVE_PAYLOAD(rhs);
        andReg(self, lhs, rhs);
        break;
    case 0xA3:
        lhs.type = a;
        rhs.type = e;
        RESOLVE_PAYLOAD(rhs);
        andReg(self, lhs, rhs);
        break;
    case 0xA4:
        lhs.type = a;
        rhs.type = h;
        RESOLVE_PAYLOAD(rhs);
        andReg(self, lhs, rhs);
        break;
    case 0xA5:
        lhs.type = a;
        rhs.type = l;
        RESOLVE_PAYLOAD(rhs);
        andReg(self, lhs, rhs);
        break;
    case 0xA6:
        lhs.type = a;
        rhs.type = phl;
        RESOLVE_PAYLOAD(rhs);
        andReg(self, lhs, rhs);
        break;
    case 0xA7:
        lhs.type = a;
        rhs.type = a;
        RESOLVE_PAYLOAD(rhs);
        andReg(self, lhs, rhs);
        break;
    case 0xE6:
        lhs.type = a;
        rhs.type = imm_u8;
        RESOLVE_PAYLOAD(rhs);
        andReg(self, lhs, rhs);
        break;
    /* XOR */
    case 0xA8:
        lhs.type = a;
        rhs.type = b;
        RESOLVE_PAYLOAD(rhs);
        xorReg(self, lhs, rhs);
        break;
    case 0xA9:
        lhs.type = a;
        rhs.type = c;
        RESOLVE_PAYLOAD(rhs);
        xorReg(self, lhs, rhs);
        break;
    case 0xAA:
        lhs.type = a;
        rhs.type = d;
        RESOLVE_PAYLOAD(rhs);
        xorReg(self, lhs, rhs);
        break;
    case 0xAB:
        lhs.type = a;
        rhs.type = e;
        RESOLVE_PAYLOAD(rhs);
        xorReg(self, lhs, rhs);
        break;
    case 0xAC:
        lhs.type = a;
        rhs.type = h;
        RESOLVE_PAYLOAD(rhs);
        xorReg(self, lhs, rhs);
        break;
    case 0xAD:
        lhs.type = a;
        rhs.type = l;
        RESOLVE_PAYLOAD(rhs);
        xorReg(self, lhs, rhs);
        break;
    case 0xAE:
        lhs.type = a;
        rhs.type = phl;
        RESOLVE_PAYLOAD(rhs);
        xorReg(self, lhs, rhs);
        break;
    case 0xAF:
        lhs.type = a;
        rhs.type = a;
        RESOLVE_PAYLOAD(rhs);
        xorReg(self, lhs, rhs);
        break;
    case 0xEE:
        lhs.type = a;
        rhs.type = imm_u8;
        RESOLVE_PAYLOAD(rhs);
        xorReg(self, lhs, rhs);
        break;
    /* OR */
    case 0xB0:
        lhs.type = a;
        rhs.type = b;
        RESOLVE_PAYLOAD(rhs);
        orReg(self, lhs, rhs);
        break;
    case 0xB1:
        lhs.type = a;
        rhs.type = c;
        RESOLVE_PAYLOAD(rhs);
        orReg(self, lhs, rhs);
        break;
    case 0xB2:
        lhs.type = a;
        rhs.type = d;
        RESOLVE_PAYLOAD(rhs);
        orReg(self, lhs, rhs);
        break;
    case 0xB3:
        lhs.type = a;
        rhs.type = e;
        RESOLVE_PAYLOAD(rhs);
        orReg(self, lhs, rhs);
        break;
    case 0xB4:
        lhs.type = a;
        rhs.type = h;
        RESOLVE_PAYLOAD(rhs);
        orReg(self, lhs, rhs);
        break;
    case 0xB5:
        lhs.type = a;
        rhs.type = l;
        RESOLVE_PAYLOAD(rhs);
        orReg(self, lhs, rhs);
        break;
    case 0xB6:
        lhs.type = a;
        rhs.type = phl;
        RESOLVE_PAYLOAD(rhs);
        orReg(self, lhs, rhs);
        break;
    case 0xB7:
        lhs.type = a;
        rhs.type = a;
        RESOLVE_PAYLOAD(rhs);
        orReg(self, lhs, rhs);
        break;
    case 0xF6:
        lhs.type = a;
        rhs.type = imm_u8;
        RESOLVE_PAYLOAD(rhs);
        orReg(self, lhs, rhs);
        break;
    /* CP */
    case 0xB8:
        lhs.type = a;
        rhs.type = b;
        RESOLVE_PAYLOAD(rhs);
        cp(self, lhs, rhs);
        break;
    case 0xB9:
        lhs.type = a;
        rhs.type = c;
        RESOLVE_PAYLOAD(rhs);
        cp(self, lhs, rhs);
        break;
    case 0xBA:
        lhs.type = a;
        rhs.type = d;
        RESOLVE_PAYLOAD(rhs);
        cp(self, lhs, rhs);
        break;
    case 0xBB:
        lhs.type = a;
        rhs.type = e;
        RESOLVE_PAYLOAD(rhs);
        cp(self, lhs, rhs);
        break;
    case 0xBC:
        lhs.type = a;
        rhs.type = h;
        RESOLVE_PAYLOAD(rhs);
        cp(self, lhs, rhs);
        break;
    case 0xBD:
        lhs.type = a;
        rhs.type = l;
        RESOLVE_PAYLOAD(rhs);
        cp(self, lhs, rhs);
        break;
    case 0xBE:
        lhs.type = a;
        rhs.type = phl;
        RESOLVE_PAYLOAD(rhs);
        cp(self, lhs, rhs);
        break;
    case 0xBF:
        lhs.type = a;
        rhs.type = a;
        RESOLVE_PAYLOAD(rhs);
        cp(self, lhs, rhs);
        break;
    case 0xFE:
        lhs.type = a;
        rhs.type = imm_u8;
        RESOLVE_PAYLOAD(rhs);
        cp(self, lhs, rhs);
        break;
    /* RET */
    case 0xC0:
        lhs.cond = nzero;
        RESOLVE_COND(lhs);
        ret(self, lhs, rhs);
        break;
    case 0xC8:
        lhs.cond = zero;
        RESOLVE_COND(lhs);
        ret(self, lhs, rhs);
        break;
    case 0xC9:
        lhs.cond = none;
        RESOLVE_COND(lhs);
        ret(self, lhs, rhs);
        break;
    case 0xD0:
        lhs.cond = ncarry;
        RESOLVE_COND(lhs);
        ret(self, lhs, rhs);
        break;
    case 0xD8:
        lhs.cond = carry;
        RESOLVE_COND(lhs);
        ret(self, lhs, rhs);
        break;
    /* JP */
    case 0xC2:
        lhs.cond = nzero;
        lhs.type = imm_u16;
        RESOLVE_COND(lhs);
        RESOLVE_PAYLOAD(lhs);
        jp(self, lhs, rhs);
        break;
    case 0xCA:
        lhs.cond = zero;
        lhs.type = imm_u16;
        RESOLVE_COND(lhs);
        RESOLVE_PAYLOAD(lhs);
        jp(self, lhs, rhs);
        break;
    case 0xC3:
        lhs.cond = none;
        lhs.type = imm_u16;
        RESOLVE_COND(lhs);
        RESOLVE_PAYLOAD(lhs);
        jp(self, lhs, rhs);
        break;
    case 0xD2:
        lhs.cond = ncarry;
        lhs.type = imm_u16;
        RESOLVE_COND(lhs);
        RESOLVE_PAYLOAD(lhs);
        jp(self, lhs, rhs);
        break;
    case 0xDA:
        lhs.cond = carry;
        lhs.type = imm_u16;
        RESOLVE_COND(lhs);
        RESOLVE_PAYLOAD(lhs);
        jp(self, lhs, rhs);
        break;
    case 0xCB:
        handle_cb(self);
        break;
    default:
        PANIC("Unhandled opcode");
        break;
    }
}

void handle_cb(cpu *self) {
    const uint8_t opcode = next_instruction(self);
    argument_t lhs;
    argument_t rhs;
    switch (opcode) {
    case 0x00:
    case 0x01:
    case 0x02:
    case 0x03:
    case 0x04:
    case 0x05:
    case 0x06:
    case 0x07:
    case 0x08:
    case 0x09:
    case 0x0A:
    case 0x0B:
    case 0x0C:
    case 0x0D:
    case 0x0E:
    case 0x0F:
    case 0x10:
    case 0x11:
    case 0x12:
    case 0x13:
    case 0x14:
    case 0x15:
    case 0x16:
    case 0x17:
    case 0x18:
    case 0x19:
    case 0x1A:
    case 0x1B:
    case 0x1C:
    case 0x1D:
    case 0x1E:
    case 0x1F:
    case 0x20:
    case 0x21:
    case 0x22:
    case 0x23:
    case 0x24:
    case 0x25:
    case 0x26:
    case 0x27:
    case 0x28:
    case 0x29:
    case 0x2A:
    case 0x2B:
    case 0x2C:
    case 0x2D:
    case 0x2E:
    case 0x2F:
    case 0x30:
    case 0x31:
    case 0x32:
    case 0x33:
    case 0x34:
    case 0x35:
    case 0x36:
    case 0x37:
    case 0x38:
    case 0x39:
    case 0x3A:
    case 0x3B:
    case 0x3C:
    case 0x3D:
    case 0x3E:
    case 0x3F:
    case 0x40:
    case 0x41:
    case 0x42:
    case 0x43:
    case 0x44:
    case 0x45:
    case 0x46:
    case 0x47:
    case 0x48:
    case 0x49:
    case 0x4A:
    case 0x4B:
    case 0x4C:
    case 0x4D:
    case 0x4E:
    case 0x4F:
    case 0x50:
    case 0x51:
    case 0x52:
    case 0x53:
    case 0x54:
    case 0x55:
    case 0x56:
    case 0x57:
    case 0x58:
    case 0x59:
    case 0x5A:
    case 0x5B:
    case 0x5C:
    case 0x5D:
    case 0x5E:
    case 0x5F:
    case 0x60:
    case 0x61:
    case 0x62:
    case 0x63:
    case 0x64:
    case 0x65:
    case 0x66:
    case 0x67:
    case 0x68:
    case 0x69:
    case 0x6A:
    case 0x6B:
    case 0x6C:
    case 0x6D:
    case 0x6E:
    case 0x6F:
    case 0x70:
    case 0x71:
    case 0x72:
    case 0x73:
    case 0x74:
    case 0x75:
    case 0x76:
    case 0x77:
    case 0x78:
    case 0x79:
    case 0x7A:
    case 0x7B:
    case 0x7C:
    case 0x7D:
    case 0x7E:
    case 0x7F:
    case 0x80:
    case 0x81:
    case 0x82:
    case 0x83:
    case 0x84:
    case 0x85:
    case 0x86:
    case 0x87:
    case 0x88:
    case 0x89:
    case 0x8A:
    case 0x8B:
    case 0x8C:
    case 0x8D:
    case 0x8E:
    case 0x8F:
    case 0x90:
    case 0x91:
    case 0x92:
    case 0x93:
    case 0x94:
    case 0x95:
    case 0x96:
    case 0x97:
    case 0x98:
    case 0x99:
    case 0x9A:
    case 0x9B:
    case 0x9C:
    case 0x9D:
    case 0x9E:
    case 0x9F:
    case 0xA0:
    case 0xA1:
    case 0xA2:
    case 0xA3:
    case 0xA4:
    case 0xA5:
    case 0xA6:
    case 0xA7:
    case 0xA8:
    case 0xA9:
    case 0xAA:
    case 0xAB:
    case 0xAC:
    case 0xAD:
    case 0xAE:
    case 0xAF:
    case 0xB0:
    case 0xB1:
    case 0xB2:
    case 0xB3:
    case 0xB4:
    case 0xB5:
    case 0xB6:
    case 0xB7:
    case 0xB8:
    case 0xB9:
    case 0xBA:
    case 0xBB:
    case 0xBC:
    case 0xBD:
    case 0xBE:
    case 0xBF:
    case 0xC0:
    case 0xC1:
    case 0xC2:
    case 0xC3:
    case 0xC4:
    case 0xC5:
    case 0xC6:
    case 0xC7:
    case 0xC8:
    case 0xC9:
    case 0xCA:
    case 0xCB:
    case 0xCC:
    case 0xCD:
    case 0xCE:
    case 0xCF:
    case 0xD0:
    case 0xD1:
    case 0xD2:
    case 0xD3:
    case 0xD4:
    case 0xD5:
    case 0xD6:
    case 0xD7:
    case 0xD8:
    case 0xD9:
    case 0xDA:
    case 0xDB:
    case 0xDC:
    case 0xDD:
    case 0xDE:
    case 0xDF:
    case 0xE0:
    case 0xE1:
    case 0xE2:
    case 0xE3:
    case 0xE4:
    case 0xE5:
    case 0xE6:
    case 0xE7:
    case 0xE8:
    case 0xE9:
    case 0xEA:
    case 0xEB:
    case 0xEC:
    case 0xED:
    case 0xEE:
    case 0xEF:
    case 0xF0:
    case 0xF1:
    case 0xF2:
    case 0xF3:
    case 0xF4:
    case 0xF5:
    case 0xF6:
    case 0xF7:
    case 0xF8:
    case 0xF9:
    case 0xFA:
    case 0xFB:
    case 0xFC:
    case 0xFD:
    case 0xFE:
    case 0xFF:
    default:
        PANIC("Unhandled opcode");
        break;
    }
}
