#include "cpu.h"
#include "stdio.h"
#include "utils.h"
#include <stdlib.h>
#include <string.h>

#define GET_REG_A (self->registers[0])
#define GET_REG_F (self->registers[1])
#define GET_FLAG_Z ((GET_REG_F >> 7) & 0x01)
#define GET_FLAG_N ((GET_REG_F >> 6) & 0x01)
#define GET_FLAG_H ((GET_REG_F >> 5) & 0x01)
#define GET_FLAG_C ((GET_REG_F >> 4) & 0x01)
#define GET_REG_B (self->registers[2])
#define GET_REG_C (self->registers[3])
#define GET_REG_D (self->registers[4])
#define GET_REG_E (self->registers[5])
#define GET_REG_H (self->registers[6])
#define GET_REG_L (self->registers[7])
#define GET_REG_PAF (get_address(self->bus, GET_REG_AF))
#define GET_REG_PBC (get_address(self->bus, GET_REG_BC))
#define GET_REG_PDE (get_address(self->bus, GET_REG_DE))
#define GET_REG_PHL (get_address(self->bus, GET_REG_HL))
#define GET_REG_AF (self->registers[0] << 8 | self->registers[1])
#define GET_REG_BC (self->registers[2] << 8 | self->registers[3])
#define GET_REG_DE (self->registers[4] << 8 | self->registers[5])
#define GET_REG_HL (self->registers[6] << 8 | self->registers[7])
#define GET_REG_SP (self->sp)
#define GET_REG_PC (self->pc)

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
#define SET_FLAG_Z(n) SET_REG_F(GET_REG_F | (((n)&0x01) << 7))
#define SET_FLAG_N(n) SET_REG_F(GET_REG_F | (((n)&0x01) << 6))
#define SET_FLAG_H(n) SET_REG_F(GET_REG_F | (((n)&0x01) << 5))
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

#define SET_REG_A(n) (self->registers[0] = ((n)))
#define SET_REG_F(n) (self->registers[1] = ((n)))
#define SET_REG_B(n) (self->registers[2] = ((n)))
#define SET_REG_C(n) (self->registers[3] = ((n)))
#define SET_REG_D(n) (self->registers[4] = ((n)))
#define SET_REG_E(n) (self->registers[5] = ((n)))
#define SET_REG_H(n) (self->registers[6] = ((n)))
#define SET_REG_L(n) (self->registers[7] = ((n)))
#define SET_REG_SP(n) (self->sp = (n))
#define SET_REG_PC(n) (self->pc = (n))
#define SET_REG_AF(n)                                                          \
    {                                                                          \
        GET_REG_A = (n) >> 8;                                                  \
        GET_REG_F = (n)&0x00FF;                                                \
    }
#define SET_REG_BC(n)                                                          \
    {                                                                          \
        GET_REG_B = (n) >> 8;                                                  \
        GET_REG_C = (n)&0x00FF;                                                \
    }
#define SET_REG_DE(n)                                                          \
    {                                                                          \
        GET_REG_D = (n) >> 8;                                                  \
        GET_REG_E = (n)&0x00FF;                                                \
    }
#define SET_REG_HL(n)                                                          \
    {                                                                          \
        GET_REG_H = (n) >> 8;                                                  \
        GET_REG_L = (n)&0x00FF;                                                \
    }

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
    case imm_u16:                                                              \
        rhs.payload = get_imm_u16(self);                                       \
        break;                                                                 \
    default:                                                                   \
        break;                                                                 \
    };
#define RESOLVE_COND(rhs)                                                      \
    switch (rhs.cond) {                                                        \
    case zero:                                                                 \
        rhs.should_continue = GET_FLAG_Z == 0;                                 \
        break;                                                                 \
    case nzero:                                                                \
        rhs.should_continue = GET_FLAG_Z != 0;                                 \
        break;                                                                 \
    case carry:                                                                \
        rhs.should_continue = GET_FLAG_C == 0;                                 \
        break;                                                                 \
    case ncarry:                                                               \
        rhs.should_continue = GET_FLAG_C != 0;                                 \
        break;                                                                 \
    case none:                                                                 \
        rhs.should_continue = TRUE;                                            \
        break;                                                                 \
    default:                                                                   \
        rhs.should_continue = FALSE;                                           \
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
    return (uint16_t)((b2 << 8) | b1);
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
        SET_FLAG_Z(res == 0);
        break;
    case sp:
        SET_FLAG_Z(0);
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

void rlc(cpu *self, argument_t lhs, argument_t rhs) {
    uint16_t res;
    (void)rhs;
    res =
        (lhs.payload << 1) | (lhs.payload >> ((sizeof(lhs.payload) << 3) - 1));
    SET_REG(lhs, res);
    SET_FLAG_Z(res == 0);
    SET_FLAG_N(FALSE);
    SET_FLAG_H(FALSE);
    SET_FLAG_C(lhs.payload >> 7);
}

void rrc(cpu *self, argument_t lhs, argument_t rhs) {
    uint16_t res;
    (void)rhs;
    res =
        (lhs.payload >> 1) | (lhs.payload << ((sizeof(lhs.payload) << 3) - 1));
    SET_REG(lhs, res);
    SET_FLAG_Z(res == 0);
    SET_FLAG_N(FALSE);
    SET_FLAG_H(FALSE);
    SET_FLAG_C(lhs.payload & 0x01);
}

void rl(cpu *self, argument_t lhs, argument_t rhs) {
    uint16_t res;
    (void)rhs;
    res = (lhs.payload << 1) | (GET_FLAG_C);
    SET_REG(lhs, res);
    SET_FLAG_Z(res == 0);
    SET_FLAG_N(FALSE);
    SET_FLAG_H(FALSE);
    SET_FLAG_C(lhs.payload >> 7);
}

void rr(cpu *self, argument_t lhs, argument_t rhs) {
    uint16_t res;
    (void)rhs;
    res = (GET_FLAG_C) | (lhs.payload >> 1);
    SET_REG(lhs, res);
    SET_FLAG_Z(res == 0);
    SET_FLAG_N(FALSE);
    SET_FLAG_H(FALSE);
    SET_FLAG_C(lhs.payload & 0x01);
}

void sla(cpu *self, argument_t lhs, argument_t rhs) {
    uint16_t res;
    (void)rhs;
    res = (lhs.payload << 1);
    SET_REG(lhs, res);
    SET_FLAG_Z(res == 0);
    SET_FLAG_N(FALSE);
    SET_FLAG_H(FALSE);
    SET_FLAG_C(lhs.payload >> 7);
}

void sra(cpu *self, argument_t lhs, argument_t rhs) {
    uint16_t res;
    (void)rhs;
    res = (lhs.payload & 0x80) | (lhs.payload >> 1);
    SET_REG(lhs, res);
    SET_FLAG_Z(res == 0);
    SET_FLAG_N(FALSE);
    SET_FLAG_H(FALSE);
    SET_FLAG_C(lhs.payload & 0x01);
}

void bit(cpu *self, argument_t lhs, argument_t rhs) {
    SET_FLAG_Z(~((lhs.payload >> rhs.payload) & 0x01));
    SET_FLAG_N(FALSE);
    SET_FLAG_H(FALSE);
}

void res(cpu *self, argument_t lhs, argument_t rhs) {
    SET_REG(lhs, lhs.payload | ~(1 << rhs.payload))
}

void set(cpu *self, argument_t lhs, argument_t rhs) {
    SET_REG(lhs, lhs.payload | (1 << rhs.payload))
}

void swap(cpu *self, argument_t lhs, argument_t rhs) {
    uint16_t res;
    (void)rhs;
    res = (lhs.payload & 0x0F) | (lhs.payload >> 4);
    SET_REG(lhs, res);
    SET_FLAG_Z(res == 0);
    SET_FLAG_N(FALSE);
    SET_FLAG_H(FALSE);
    SET_FLAG_C(FALSE);
}

void srl(cpu *self, argument_t lhs, argument_t rhs) {
    uint16_t res;
    (void)rhs;
    res = (lhs.payload >> 1);
    SET_REG(lhs, res);
    SET_FLAG_Z(res == 0);
    SET_FLAG_N(FALSE);
    SET_FLAG_H(FALSE);
    SET_FLAG_C(lhs.payload & 0x01);
}

void jr(cpu *self, argument_t lhs, argument_t rhs) {
    (void)rhs;
    if (lhs.cond) {
        SET_REG_PC(GET_REG_PC + (int8_t)lhs.payload);
    }
}

void cpu_clock(cpu *self) {
    uint8_t opcode;
    argument_t lhs;
    argument_t rhs;

    printf("pc: 0x%X\n", GET_REG_PC);
    opcode = next_instruction(self);
    printf("opcode: 0x%X\n", opcode);
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
        lhs.type = io_offset;
        lhs.payload = get_imm_u8(self);
        RESOLVE_PAYLOAD(lhs);
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
    /* CB */
    case 0xCB:
        handle_cb(self);
        break;
    /* JR */
    case 0x18:
        lhs.cond = none;
        RESOLVE_COND(lhs);
        lhs.type = imm_u8;
        RESOLVE_PAYLOAD(lhs);
        jr(self, lhs, rhs);
        break;
    case 0x20:
        lhs.cond = nzero;
        RESOLVE_COND(lhs);
        lhs.type = imm_u8;
        RESOLVE_PAYLOAD(lhs);
        jr(self, lhs, rhs);
        break;
    case 0x28:
        lhs.cond = zero;
        RESOLVE_COND(lhs);
        lhs.type = imm_u8;
        RESOLVE_PAYLOAD(lhs);
        jr(self, lhs, rhs);
        break;
    case 0x30:
        lhs.cond = ncarry;
        RESOLVE_COND(lhs);
        lhs.type = imm_u8;
        RESOLVE_PAYLOAD(lhs);
        jr(self, lhs, rhs);
        break;
    case 0x38:
        lhs.cond = carry;
        RESOLVE_COND(lhs);
        lhs.type = imm_u8;
        RESOLVE_PAYLOAD(lhs);
        jr(self, lhs, rhs);
        break;
    default:
        PANIC("Unhandled opcode");
        break;
    }

    printf("clocks: %d\n", self->clocks);
}

void handle_cb(cpu *self) {
    uint8_t opcode;
    argument_t lhs;
    argument_t rhs;

    printf("pc: 0x%X\n", GET_REG_PC);
    opcode = next_instruction(self);
    printf("opcode: 0x%X\n", opcode);
    switch (opcode) {
    /* RLC */
    case 0x00:
        lhs.type = b;
        RESOLVE_PAYLOAD(lhs);
        rlc(self, lhs, rhs);
        break;
    case 0x01:
        lhs.type = c;
        RESOLVE_PAYLOAD(lhs);
        rlc(self, lhs, rhs);
        break;
    case 0x02:
        lhs.type = d;
        RESOLVE_PAYLOAD(lhs);
        rlc(self, lhs, rhs);
        break;
    case 0x03:
        lhs.type = e;
        RESOLVE_PAYLOAD(lhs);
        rlc(self, lhs, rhs);
        break;
    case 0x04:
        lhs.type = h;
        RESOLVE_PAYLOAD(lhs);
        rlc(self, lhs, rhs);
        break;
    case 0x05:
        lhs.type = l;
        RESOLVE_PAYLOAD(lhs);
        rlc(self, lhs, rhs);
        break;
    case 0x06:
        lhs.type = phl;
        RESOLVE_PAYLOAD(lhs);
        rlc(self, lhs, rhs);
        break;
    case 0x07:
        lhs.type = a;
        RESOLVE_PAYLOAD(lhs);
        rlc(self, lhs, rhs);
        break;
    /* RRC */
    case 0x08:
        lhs.type = b;
        RESOLVE_PAYLOAD(lhs);
        rrc(self, lhs, rhs);
        break;
    case 0x09:
        lhs.type = c;
        RESOLVE_PAYLOAD(lhs);
        rrc(self, lhs, rhs);
        break;
    case 0x0A:
        lhs.type = d;
        RESOLVE_PAYLOAD(lhs);
        rrc(self, lhs, rhs);
        break;
    case 0x0B:
        lhs.type = e;
        RESOLVE_PAYLOAD(lhs);
        rrc(self, lhs, rhs);
        break;
    case 0x0C:
        lhs.type = h;
        RESOLVE_PAYLOAD(lhs);
        rrc(self, lhs, rhs);
        break;
    case 0x0D:
        lhs.type = l;
        RESOLVE_PAYLOAD(lhs);
        rrc(self, lhs, rhs);
        break;
    case 0x0E:
        lhs.type = phl;
        RESOLVE_PAYLOAD(lhs);
        rrc(self, lhs, rhs);
        break;
    case 0x0F:
        lhs.type = a;
        RESOLVE_PAYLOAD(lhs);
        rrc(self, lhs, rhs);
        break;
    /* RL */
    case 0x10:
        lhs.type = b;
        RESOLVE_PAYLOAD(lhs);
        rl(self, lhs, rhs);
        break;
    case 0x11:
        lhs.type = c;
        RESOLVE_PAYLOAD(lhs);
        rl(self, lhs, rhs);
        break;
    case 0x12:
        lhs.type = d;
        RESOLVE_PAYLOAD(lhs);
        rl(self, lhs, rhs);
        break;
    case 0x13:
        lhs.type = e;
        RESOLVE_PAYLOAD(lhs);
        rl(self, lhs, rhs);
        break;
    case 0x14:
        lhs.type = h;
        RESOLVE_PAYLOAD(lhs);
        rl(self, lhs, rhs);
        break;
    case 0x15:
        lhs.type = l;
        RESOLVE_PAYLOAD(lhs);
        rl(self, lhs, rhs);
        break;
    case 0x16:
        lhs.type = phl;
        RESOLVE_PAYLOAD(lhs);
        rl(self, lhs, rhs);
        break;
    case 0x17:
        lhs.type = a;
        RESOLVE_PAYLOAD(lhs);
        rl(self, lhs, rhs);
        break;
    /* RR */
    case 0x18:
        lhs.type = b;
        RESOLVE_PAYLOAD(lhs);
        rr(self, lhs, rhs);
        break;
    case 0x19:
        lhs.type = c;
        RESOLVE_PAYLOAD(lhs);
        rr(self, lhs, rhs);
        break;
    case 0x1A:
        lhs.type = d;
        RESOLVE_PAYLOAD(lhs);
        rr(self, lhs, rhs);
        break;
    case 0x1B:
        lhs.type = e;
        RESOLVE_PAYLOAD(lhs);
        rr(self, lhs, rhs);
        break;
    case 0x1C:
        lhs.type = h;
        RESOLVE_PAYLOAD(lhs);
        rr(self, lhs, rhs);
        break;
    case 0x1D:
        lhs.type = l;
        RESOLVE_PAYLOAD(lhs);
        rr(self, lhs, rhs);
        break;
    case 0x1E:
        lhs.type = phl;
        RESOLVE_PAYLOAD(lhs);
        rr(self, lhs, rhs);
        break;
    case 0x1F:
        lhs.type = a;
        RESOLVE_PAYLOAD(lhs);
        rr(self, lhs, rhs);
        break;
    /* SLA */
    case 0x20:
        lhs.type = b;
        RESOLVE_PAYLOAD(lhs);
        sla(self, lhs, rhs);
        break;
    case 0x21:
        lhs.type = c;
        RESOLVE_PAYLOAD(lhs);
        sla(self, lhs, rhs);
        break;
    case 0x22:
        lhs.type = d;
        RESOLVE_PAYLOAD(lhs);
        sla(self, lhs, rhs);
        break;
    case 0x23:
        lhs.type = e;
        RESOLVE_PAYLOAD(lhs);
        sla(self, lhs, rhs);
        break;
    case 0x24:
        lhs.type = h;
        RESOLVE_PAYLOAD(lhs);
        sla(self, lhs, rhs);
        break;
    case 0x25:
        lhs.type = l;
        RESOLVE_PAYLOAD(lhs);
        sla(self, lhs, rhs);
        break;
    case 0x26:
        lhs.type = phl;
        RESOLVE_PAYLOAD(lhs);
        sla(self, lhs, rhs);
        break;
    case 0x27:
        lhs.type = a;
        RESOLVE_PAYLOAD(lhs);
        sla(self, lhs, rhs);
        break;
    /* SRA */
    case 0x28:
        lhs.type = b;
        RESOLVE_PAYLOAD(lhs);
        sra(self, lhs, rhs);
        break;
    case 0x29:
        lhs.type = c;
        RESOLVE_PAYLOAD(lhs);
        sra(self, lhs, rhs);
        break;
    case 0x2A:
        lhs.type = d;
        RESOLVE_PAYLOAD(lhs);
        sra(self, lhs, rhs);
        break;
    case 0x2B:
        lhs.type = e;
        RESOLVE_PAYLOAD(lhs);
        sra(self, lhs, rhs);
        break;
    case 0x2C:
        lhs.type = h;
        RESOLVE_PAYLOAD(lhs);
        sra(self, lhs, rhs);
        break;
    case 0x2D:
        lhs.type = l;
        RESOLVE_PAYLOAD(lhs);
        sra(self, lhs, rhs);
        break;
    case 0x2E:
        lhs.type = phl;
        RESOLVE_PAYLOAD(lhs);
        sra(self, lhs, rhs);
        break;
    case 0x2F:
        lhs.type = a;
        RESOLVE_PAYLOAD(lhs);
        sra(self, lhs, rhs);
        break;
    /* SWAP */
    case 0x30:
        lhs.type = b;
        RESOLVE_PAYLOAD(lhs);
        swap(self, lhs, rhs);
        break;
    case 0x31:
        lhs.type = c;
        RESOLVE_PAYLOAD(lhs);
        swap(self, lhs, rhs);
        break;
    case 0x32:
        lhs.type = d;
        RESOLVE_PAYLOAD(lhs);
        swap(self, lhs, rhs);
        break;
    case 0x33:
        lhs.type = e;
        RESOLVE_PAYLOAD(lhs);
        swap(self, lhs, rhs);
        break;
    case 0x34:
        lhs.type = h;
        RESOLVE_PAYLOAD(lhs);
        swap(self, lhs, rhs);
        break;
    case 0x35:
        lhs.type = l;
        RESOLVE_PAYLOAD(lhs);
        swap(self, lhs, rhs);
        break;
    case 0x36:
        lhs.type = phl;
        RESOLVE_PAYLOAD(lhs);
        swap(self, lhs, rhs);
        break;
    case 0x37:
        lhs.type = a;
        RESOLVE_PAYLOAD(lhs);
        swap(self, lhs, rhs);
        break;
    /* SRL */
    case 0x38:
        lhs.type = b;
        RESOLVE_PAYLOAD(lhs);
        srl(self, lhs, rhs);
        break;
    case 0x39:
        lhs.type = c;
        RESOLVE_PAYLOAD(lhs);
        srl(self, lhs, rhs);
        break;
    case 0x3A:
        lhs.type = d;
        RESOLVE_PAYLOAD(lhs);
        srl(self, lhs, rhs);
        break;
    case 0x3B:
        lhs.type = e;
        RESOLVE_PAYLOAD(lhs);
        srl(self, lhs, rhs);
        break;
    case 0x3C:
        lhs.type = h;
        RESOLVE_PAYLOAD(lhs);
        srl(self, lhs, rhs);
        break;
    case 0x3D:
        lhs.type = l;
        RESOLVE_PAYLOAD(lhs);
        srl(self, lhs, rhs);
        break;
    case 0x3E:
        lhs.type = phl;
        RESOLVE_PAYLOAD(lhs);
        srl(self, lhs, rhs);
        break;
    case 0x3F:
        lhs.type = a;
        RESOLVE_PAYLOAD(lhs);
        srl(self, lhs, rhs);
        break;
    /* BIT */
    case 0x40:
        lhs.type = b;
        RESOLVE_PAYLOAD(lhs);
        rhs.payload = 0;
        bit(self, lhs, rhs);
        break;
    case 0x41:
        lhs.type = c;
        RESOLVE_PAYLOAD(lhs);
        rhs.payload = 0;
        bit(self, lhs, rhs);
        break;
    case 0x42:
        lhs.type = d;
        RESOLVE_PAYLOAD(lhs);
        rhs.payload = 0;
        bit(self, lhs, rhs);
        break;
    case 0x43:
        lhs.type = e;
        RESOLVE_PAYLOAD(lhs);
        rhs.payload = 0;
        bit(self, lhs, rhs);
        break;
    case 0x44:
        lhs.type = h;
        RESOLVE_PAYLOAD(lhs);
        rhs.payload = 0;
        bit(self, lhs, rhs);
        break;
    case 0x45:
        lhs.type = l;
        RESOLVE_PAYLOAD(lhs);
        rhs.payload = 0;
        bit(self, lhs, rhs);
        break;
    case 0x46:
        lhs.type = phl;
        RESOLVE_PAYLOAD(lhs);
        rhs.payload = 0;
        bit(self, lhs, rhs);
        break;
    case 0x47:
        lhs.type = a;
        RESOLVE_PAYLOAD(lhs);
        rhs.payload = 0;
        bit(self, lhs, rhs);
        break;
    case 0x48:
        lhs.type = b;
        RESOLVE_PAYLOAD(lhs);
        rhs.payload = 1;
        bit(self, lhs, rhs);
        break;
    case 0x49:
        lhs.type = c;
        RESOLVE_PAYLOAD(lhs);
        rhs.payload = 1;
        bit(self, lhs, rhs);
        break;
    case 0x4A:
        lhs.type = d;
        RESOLVE_PAYLOAD(lhs);
        rhs.payload = 1;
        bit(self, lhs, rhs);
        break;
    case 0x4B:
        lhs.type = e;
        RESOLVE_PAYLOAD(lhs);
        rhs.payload = 1;
        bit(self, lhs, rhs);
        break;
    case 0x4C:
        lhs.type = h;
        RESOLVE_PAYLOAD(lhs);
        rhs.payload = 1;
        bit(self, lhs, rhs);
        break;
    case 0x4D:
        lhs.type = l;
        RESOLVE_PAYLOAD(lhs);
        rhs.payload = 1;
        bit(self, lhs, rhs);
        break;
    case 0x4E:
        lhs.type = phl;
        RESOLVE_PAYLOAD(lhs);
        rhs.payload = 1;
        bit(self, lhs, rhs);
        break;
    case 0x4F:
        lhs.type = a;
        RESOLVE_PAYLOAD(lhs);
        rhs.payload = 1;
        bit(self, lhs, rhs);
        break;
    case 0x50:
        lhs.type = b;
        RESOLVE_PAYLOAD(lhs);
        rhs.payload = 2;
        bit(self, lhs, rhs);
        break;
    case 0x51:
        lhs.type = c;
        RESOLVE_PAYLOAD(lhs);
        rhs.payload = 2;
        bit(self, lhs, rhs);
        break;
    case 0x52:
        lhs.type = d;
        RESOLVE_PAYLOAD(lhs);
        rhs.payload = 2;
        bit(self, lhs, rhs);
        break;
    case 0x53:
        lhs.type = e;
        RESOLVE_PAYLOAD(lhs);
        rhs.payload = 2;
        bit(self, lhs, rhs);
        break;
    case 0x54:
        lhs.type = h;
        RESOLVE_PAYLOAD(lhs);
        rhs.payload = 2;
        bit(self, lhs, rhs);
        break;
    case 0x55:
        lhs.type = l;
        RESOLVE_PAYLOAD(lhs);
        rhs.payload = 2;
        bit(self, lhs, rhs);
        break;
    case 0x56:
        lhs.type = phl;
        RESOLVE_PAYLOAD(lhs);
        rhs.payload = 2;
        bit(self, lhs, rhs);
        break;
    case 0x57:
        lhs.type = a;
        RESOLVE_PAYLOAD(lhs);
        rhs.payload = 2;
        bit(self, lhs, rhs);
        break;
    case 0x58:
        lhs.type = b;
        RESOLVE_PAYLOAD(lhs);
        rhs.payload = 3;
        bit(self, lhs, rhs);
        break;
    case 0x59:
        lhs.type = c;
        RESOLVE_PAYLOAD(lhs);
        rhs.payload = 3;
        bit(self, lhs, rhs);
        break;
    case 0x5A:
        lhs.type = d;
        RESOLVE_PAYLOAD(lhs);
        rhs.payload = 3;
        bit(self, lhs, rhs);
        break;
    case 0x5B:
        lhs.type = e;
        RESOLVE_PAYLOAD(lhs);
        rhs.payload = 3;
        bit(self, lhs, rhs);
        break;
    case 0x5C:
        lhs.type = h;
        RESOLVE_PAYLOAD(lhs);
        rhs.payload = 3;
        bit(self, lhs, rhs);
        break;
    case 0x5D:
        lhs.type = l;
        RESOLVE_PAYLOAD(lhs);
        rhs.payload = 3;
        bit(self, lhs, rhs);
        break;
    case 0x5E:
        lhs.type = phl;
        RESOLVE_PAYLOAD(lhs);
        rhs.payload = 3;
        bit(self, lhs, rhs);
        break;
    case 0x5F:
        lhs.type = a;
        RESOLVE_PAYLOAD(lhs);
        rhs.payload = 3;
        bit(self, lhs, rhs);
        break;
    case 0x60:
        lhs.type = b;
        RESOLVE_PAYLOAD(lhs);
        rhs.payload = 4;
        bit(self, lhs, rhs);
        break;
    case 0x61:
        lhs.type = c;
        RESOLVE_PAYLOAD(lhs);
        rhs.payload = 4;
        bit(self, lhs, rhs);
        break;
    case 0x62:
        lhs.type = d;
        RESOLVE_PAYLOAD(lhs);
        rhs.payload = 4;
        bit(self, lhs, rhs);
        break;
    case 0x63:
        lhs.type = e;
        RESOLVE_PAYLOAD(lhs);
        rhs.payload = 4;
        bit(self, lhs, rhs);
        break;
    case 0x64:
        lhs.type = h;
        RESOLVE_PAYLOAD(lhs);
        rhs.payload = 4;
        bit(self, lhs, rhs);
        break;
    case 0x65:
        lhs.type = l;
        RESOLVE_PAYLOAD(lhs);
        rhs.payload = 4;
        bit(self, lhs, rhs);
        break;
    case 0x66:
        lhs.type = phl;
        RESOLVE_PAYLOAD(lhs);
        rhs.payload = 4;
        bit(self, lhs, rhs);
        break;
    case 0x67:
        lhs.type = a;
        RESOLVE_PAYLOAD(lhs);
        rhs.payload = 4;
        bit(self, lhs, rhs);
        break;
    case 0x68:
        lhs.type = b;
        RESOLVE_PAYLOAD(lhs);
        rhs.payload = 5;
        bit(self, lhs, rhs);
        break;
    case 0x69:
        lhs.type = c;
        RESOLVE_PAYLOAD(lhs);
        rhs.payload = 5;
        bit(self, lhs, rhs);
        break;
    case 0x6A:
        lhs.type = d;
        RESOLVE_PAYLOAD(lhs);
        rhs.payload = 5;
        bit(self, lhs, rhs);
        break;
    case 0x6B:
        lhs.type = e;
        RESOLVE_PAYLOAD(lhs);
        rhs.payload = 5;
        bit(self, lhs, rhs);
        break;
    case 0x6C:
        lhs.type = h;
        RESOLVE_PAYLOAD(lhs);
        rhs.payload = 5;
        bit(self, lhs, rhs);
        break;
    case 0x6D:
        lhs.type = l;
        RESOLVE_PAYLOAD(lhs);
        rhs.payload = 5;
        bit(self, lhs, rhs);
        break;
    case 0x6E:
        lhs.type = phl;
        RESOLVE_PAYLOAD(lhs);
        rhs.payload = 5;
        bit(self, lhs, rhs);
        break;
    case 0x6F:
        lhs.type = a;
        RESOLVE_PAYLOAD(lhs);
        rhs.payload = 5;
        bit(self, lhs, rhs);
        break;
    case 0x70:
        lhs.type = b;
        RESOLVE_PAYLOAD(lhs);
        rhs.payload = 6;
        bit(self, lhs, rhs);
        break;
    case 0x71:
        lhs.type = c;
        RESOLVE_PAYLOAD(lhs);
        rhs.payload = 6;
        bit(self, lhs, rhs);
        break;
    case 0x72:
        lhs.type = d;
        RESOLVE_PAYLOAD(lhs);
        rhs.payload = 6;
        bit(self, lhs, rhs);
        break;
    case 0x73:
        lhs.type = e;
        RESOLVE_PAYLOAD(lhs);
        rhs.payload = 6;
        bit(self, lhs, rhs);
        break;
    case 0x74:
        lhs.type = h;
        RESOLVE_PAYLOAD(lhs);
        rhs.payload = 6;
        bit(self, lhs, rhs);
        break;
    case 0x75:
        lhs.type = l;
        RESOLVE_PAYLOAD(lhs);
        rhs.payload = 6;
        bit(self, lhs, rhs);
        break;
    case 0x76:
        lhs.type = phl;
        RESOLVE_PAYLOAD(lhs);
        rhs.payload = 6;
        bit(self, lhs, rhs);
        break;
    case 0x77:
        lhs.type = a;
        RESOLVE_PAYLOAD(lhs);
        rhs.payload = 6;
        bit(self, lhs, rhs);
        break;
    case 0x78:
        lhs.type = b;
        RESOLVE_PAYLOAD(lhs);
        rhs.payload = 7;
        bit(self, lhs, rhs);
        break;
    case 0x79:
        lhs.type = c;
        RESOLVE_PAYLOAD(lhs);
        rhs.payload = 7;
        bit(self, lhs, rhs);
        break;
    case 0x7A:
        lhs.type = d;
        RESOLVE_PAYLOAD(lhs);
        rhs.payload = 7;
        bit(self, lhs, rhs);
        break;
    case 0x7B:
        lhs.type = e;
        RESOLVE_PAYLOAD(lhs);
        rhs.payload = 7;
        bit(self, lhs, rhs);
        break;
    case 0x7C:
        lhs.type = h;
        RESOLVE_PAYLOAD(lhs);
        rhs.payload = 7;
        bit(self, lhs, rhs);
        break;
    case 0x7D:
        lhs.type = l;
        RESOLVE_PAYLOAD(lhs);
        rhs.payload = 7;
        bit(self, lhs, rhs);
        break;
    case 0x7E:
        lhs.type = phl;
        RESOLVE_PAYLOAD(lhs);
        rhs.payload = 7;
        bit(self, lhs, rhs);
        break;
    case 0x7F:
        lhs.type = a;
        RESOLVE_PAYLOAD(lhs);
        rhs.payload = 7;
        bit(self, lhs, rhs);
        break;
    /* RES */
    case 0x80:
        lhs.type = b;
        RESOLVE_PAYLOAD(lhs);
        rhs.payload = 0;
        res(self, lhs, rhs);
        break;
    case 0x81:
        lhs.type = c;
        RESOLVE_PAYLOAD(lhs);
        rhs.payload = 0;
        res(self, lhs, rhs);
        break;
    case 0x82:
        lhs.type = d;
        RESOLVE_PAYLOAD(lhs);
        rhs.payload = 0;
        res(self, lhs, rhs);
        break;
    case 0x83:
        lhs.type = e;
        RESOLVE_PAYLOAD(lhs);
        rhs.payload = 0;
        res(self, lhs, rhs);
        break;
    case 0x84:
        lhs.type = h;
        RESOLVE_PAYLOAD(lhs);
        rhs.payload = 0;
        res(self, lhs, rhs);
        break;
    case 0x85:
        lhs.type = l;
        RESOLVE_PAYLOAD(lhs);
        rhs.payload = 0;
        res(self, lhs, rhs);
        break;
    case 0x86:
        lhs.type = phl;
        RESOLVE_PAYLOAD(lhs);
        rhs.payload = 0;
        res(self, lhs, rhs);
        break;
    case 0x87:
        lhs.type = a;
        RESOLVE_PAYLOAD(lhs);
        rhs.payload = 0;
        res(self, lhs, rhs);
        break;
    case 0x88:
        lhs.type = b;
        RESOLVE_PAYLOAD(lhs);
        rhs.payload = 1;
        res(self, lhs, rhs);
        break;
    case 0x89:
        lhs.type = c;
        RESOLVE_PAYLOAD(lhs);
        rhs.payload = 1;
        res(self, lhs, rhs);
        break;
    case 0x8A:
        lhs.type = d;
        RESOLVE_PAYLOAD(lhs);
        rhs.payload = 1;
        res(self, lhs, rhs);
        break;
    case 0x8B:
        lhs.type = e;
        RESOLVE_PAYLOAD(lhs);
        rhs.payload = 1;
        res(self, lhs, rhs);
        break;
    case 0x8C:
        lhs.type = h;
        RESOLVE_PAYLOAD(lhs);
        rhs.payload = 1;
        res(self, lhs, rhs);
        break;
    case 0x8D:
        lhs.type = l;
        RESOLVE_PAYLOAD(lhs);
        rhs.payload = 1;
        res(self, lhs, rhs);
        break;
    case 0x8E:
        lhs.type = phl;
        RESOLVE_PAYLOAD(lhs);
        rhs.payload = 1;
        res(self, lhs, rhs);
        break;
    case 0x8F:
        lhs.type = a;
        RESOLVE_PAYLOAD(lhs);
        rhs.payload = 1;
        res(self, lhs, rhs);
        break;
    case 0x90:
        lhs.type = b;
        RESOLVE_PAYLOAD(lhs);
        rhs.payload = 2;
        res(self, lhs, rhs);
        break;
    case 0x91:
        lhs.type = c;
        RESOLVE_PAYLOAD(lhs);
        rhs.payload = 2;
        res(self, lhs, rhs);
        break;
    case 0x92:
        lhs.type = d;
        RESOLVE_PAYLOAD(lhs);
        rhs.payload = 2;
        res(self, lhs, rhs);
        break;
    case 0x93:
        lhs.type = e;
        RESOLVE_PAYLOAD(lhs);
        rhs.payload = 2;
        res(self, lhs, rhs);
        break;
    case 0x94:
        lhs.type = h;
        RESOLVE_PAYLOAD(lhs);
        rhs.payload = 2;
        res(self, lhs, rhs);
        break;
    case 0x95:
        lhs.type = l;
        RESOLVE_PAYLOAD(lhs);
        rhs.payload = 2;
        res(self, lhs, rhs);
        break;
    case 0x96:
        lhs.type = phl;
        RESOLVE_PAYLOAD(lhs) rhs.payload = 2;
        res(self, lhs, rhs);
        ;
        break;
    case 0x97:
        lhs.type = a;
        RESOLVE_PAYLOAD(lhs);
        rhs.payload = 2;
        res(self, lhs, rhs);
        break;
    case 0x98:
        lhs.type = b;
        RESOLVE_PAYLOAD(lhs);
        rhs.payload = 3;
        res(self, lhs, rhs);
        break;
    case 0x99:
        lhs.type = c;
        RESOLVE_PAYLOAD(lhs);
        rhs.payload = 3;
        res(self, lhs, rhs);
        break;
    case 0x9A:
        lhs.type = d;
        RESOLVE_PAYLOAD(lhs);
        rhs.payload = 3;
        res(self, lhs, rhs);
        break;
    case 0x9B:
        lhs.type = e;
        RESOLVE_PAYLOAD(lhs);
        rhs.payload = 3;
        res(self, lhs, rhs);
        break;
    case 0x9C:
        lhs.type = h;
        RESOLVE_PAYLOAD(lhs);
        rhs.payload = 3;
        res(self, lhs, rhs);
        break;
    case 0x9D:
        lhs.type = l;
        RESOLVE_PAYLOAD(lhs);
        rhs.payload = 3;
        res(self, lhs, rhs);
        break;
    case 0x9E:
        lhs.type = phl;
        RESOLVE_PAYLOAD(lhs);
        rhs.payload = 3;
        res(self, lhs, rhs);
        break;
    case 0x9F:
        lhs.type = a;
        RESOLVE_PAYLOAD(lhs);
        rhs.payload = 3;
        res(self, lhs, rhs);
        break;
    case 0xA0:
        lhs.type = b;
        RESOLVE_PAYLOAD(lhs);
        rhs.payload = 4;
        res(self, lhs, rhs);
        break;
    case 0xA1:
        lhs.type = c;
        RESOLVE_PAYLOAD(lhs);
        rhs.payload = 4;
        res(self, lhs, rhs);
        break;
    case 0xA2:
        lhs.type = d;
        RESOLVE_PAYLOAD(lhs);
        rhs.payload = 4;
        res(self, lhs, rhs);
        break;
    case 0xA3:
        lhs.type = e;
        RESOLVE_PAYLOAD(lhs);
        rhs.payload = 4;
        res(self, lhs, rhs);
        break;
    case 0xA4:
        lhs.type = h;
        RESOLVE_PAYLOAD(lhs);
        rhs.payload = 4;
        res(self, lhs, rhs);
        break;
    case 0xA5:
        lhs.type = l;
        RESOLVE_PAYLOAD(lhs);
        rhs.payload = 4;
        res(self, lhs, rhs);
        break;
    case 0xA6:
        lhs.type = phl;
        RESOLVE_PAYLOAD(lhs);
        rhs.payload = 4;
        res(self, lhs, rhs);
        break;
    case 0xA7:
        lhs.type = a;
        RESOLVE_PAYLOAD(lhs);
        rhs.payload = 4;
        res(self, lhs, rhs);
        break;
    case 0xA8:
        lhs.type = b;
        RESOLVE_PAYLOAD(lhs);
        rhs.payload = 5;
        res(self, lhs, rhs);
        break;
    case 0xA9:
        lhs.type = c;
        RESOLVE_PAYLOAD(lhs);
        rhs.payload = 5;
        res(self, lhs, rhs);
        break;
    case 0xAA:
        lhs.type = d;
        RESOLVE_PAYLOAD(lhs);
        rhs.payload = 5;
        res(self, lhs, rhs);
        break;
    case 0xAB:
        lhs.type = e;
        RESOLVE_PAYLOAD(lhs);
        rhs.payload = 5;
        res(self, lhs, rhs);
        break;
    case 0xAC:
        lhs.type = h;
        RESOLVE_PAYLOAD(lhs);
        rhs.payload = 5;
        res(self, lhs, rhs);
        break;
    case 0xAD:
        lhs.type = l;
        RESOLVE_PAYLOAD(lhs);
        rhs.payload = 5;
        res(self, lhs, rhs);
        break;
    case 0xAE:
        lhs.type = phl;
        RESOLVE_PAYLOAD(lhs);
        rhs.payload = 5;
        res(self, lhs, rhs);
        break;
    case 0xAF:
        lhs.type = a;
        RESOLVE_PAYLOAD(lhs);
        rhs.payload = 5;
        res(self, lhs, rhs);
        break;
    case 0xB0:
        lhs.type = b;
        RESOLVE_PAYLOAD(lhs);
        rhs.payload = 6;
        res(self, lhs, rhs);
        break;
    case 0xB1:
        lhs.type = c;
        RESOLVE_PAYLOAD(lhs);
        rhs.payload = 6;
        res(self, lhs, rhs);
        break;
    case 0xB2:
        lhs.type = d;
        RESOLVE_PAYLOAD(lhs);
        rhs.payload = 6;
        res(self, lhs, rhs);
        break;
    case 0xB3:
        lhs.type = e;
        RESOLVE_PAYLOAD(lhs);
        rhs.payload = 6;
        res(self, lhs, rhs);
        break;
    case 0xB4:
        lhs.type = h;
        RESOLVE_PAYLOAD(lhs);
        rhs.payload = 6;
        res(self, lhs, rhs);
        break;
    case 0xB5:
        lhs.type = l;
        RESOLVE_PAYLOAD(lhs);
        rhs.payload = 6;
        res(self, lhs, rhs);
        break;
    case 0xB6:
        lhs.type = phl;
        RESOLVE_PAYLOAD(lhs);
        rhs.payload = 6;
        res(self, lhs, rhs);
        break;
    case 0xB7:
        lhs.type = a;
        RESOLVE_PAYLOAD(lhs);
        rhs.payload = 6;
        res(self, lhs, rhs);
        break;
    case 0xB8:
        lhs.type = b;
        RESOLVE_PAYLOAD(lhs);
        rhs.payload = 7;
        res(self, lhs, rhs);
        break;
    case 0xB9:
        lhs.type = c;
        RESOLVE_PAYLOAD(lhs);
        rhs.payload = 7;
        res(self, lhs, rhs);
        break;
    case 0xBA:
        lhs.type = d;
        RESOLVE_PAYLOAD(lhs);
        rhs.payload = 7;
        res(self, lhs, rhs);
        break;
    case 0xBB:
        lhs.type = e;
        RESOLVE_PAYLOAD(lhs);
        rhs.payload = 7;
        res(self, lhs, rhs);
        break;
    case 0xBC:
        lhs.type = h;
        RESOLVE_PAYLOAD(lhs);
        rhs.payload = 7;
        res(self, lhs, rhs);
        break;
    case 0xBD:
        lhs.type = l;
        RESOLVE_PAYLOAD(lhs);
        rhs.payload = 7;
        res(self, lhs, rhs);
        break;
    case 0xBE:
        lhs.type = phl;
        RESOLVE_PAYLOAD(lhs);
        rhs.payload = 7;
        res(self, lhs, rhs);
        break;
    case 0xBF:
        lhs.type = a;
        RESOLVE_PAYLOAD(lhs);
        rhs.payload = 7;
        res(self, lhs, rhs);
        break;
    /* SET */
    case 0xC0:
        lhs.type = b;
        RESOLVE_PAYLOAD(lhs);
        rhs.payload = 0;
        set(self, lhs, rhs);
        break;
    case 0xC1:
        lhs.type = c;
        RESOLVE_PAYLOAD(lhs);
        rhs.payload = 0;
        set(self, lhs, rhs);
        break;
    case 0xC2:
        lhs.type = d;
        RESOLVE_PAYLOAD(lhs);
        rhs.payload = 0;
        set(self, lhs, rhs);
        break;
    case 0xC3:
        lhs.type = e;
        RESOLVE_PAYLOAD(lhs);
        rhs.payload = 0;
        set(self, lhs, rhs);
        break;
    case 0xC4:
        lhs.type = h;
        RESOLVE_PAYLOAD(lhs);
        rhs.payload = 0;
        set(self, lhs, rhs);
        break;
    case 0xC5:
        lhs.type = l;
        RESOLVE_PAYLOAD(lhs);
        rhs.payload = 0;
        set(self, lhs, rhs);
        break;
    case 0xC6:
        lhs.type = phl;
        RESOLVE_PAYLOAD(lhs);
        rhs.payload = 0;
        set(self, lhs, rhs);
        break;
    case 0xC7:
        lhs.type = a;
        RESOLVE_PAYLOAD(lhs);
        rhs.payload = 0;
        set(self, lhs, rhs);
        break;
    case 0xC8:
        lhs.type = b;
        RESOLVE_PAYLOAD(lhs);
        rhs.payload = 1;
        set(self, lhs, rhs);
        break;
    case 0xC9:
        lhs.type = c;
        RESOLVE_PAYLOAD(lhs);
        rhs.payload = 1;
        set(self, lhs, rhs);
        break;
    case 0xCA:
        lhs.type = d;
        RESOLVE_PAYLOAD(lhs);
        rhs.payload = 1;
        set(self, lhs, rhs);
        break;
    case 0xCB:
        lhs.type = e;
        RESOLVE_PAYLOAD(lhs);
        rhs.payload = 1;
        set(self, lhs, rhs);
        break;
    case 0xCC:
        lhs.type = h;
        RESOLVE_PAYLOAD(lhs);
        rhs.payload = 1;
        set(self, lhs, rhs);
        break;
    case 0xCD:
        lhs.type = l;
        RESOLVE_PAYLOAD(lhs);
        rhs.payload = 1;
        set(self, lhs, rhs);
        break;
    case 0xCE:
        lhs.type = phl;
        RESOLVE_PAYLOAD(lhs);
        rhs.payload = 1;
        set(self, lhs, rhs);
        break;
    case 0xCF:
        lhs.type = a;
        RESOLVE_PAYLOAD(lhs);
        rhs.payload = 1;
        set(self, lhs, rhs);
        break;
    case 0xD0:
        lhs.type = b;
        RESOLVE_PAYLOAD(lhs);
        rhs.payload = 2;
        set(self, lhs, rhs);
        break;
    case 0xD1:
        lhs.type = c;
        RESOLVE_PAYLOAD(lhs);
        rhs.payload = 2;
        set(self, lhs, rhs);
        break;
    case 0xD2:
        lhs.type = d;
        RESOLVE_PAYLOAD(lhs);
        rhs.payload = 2;
        set(self, lhs, rhs);
        break;
    case 0xD3:
        lhs.type = e;
        RESOLVE_PAYLOAD(lhs);
        rhs.payload = 2;
        set(self, lhs, rhs);
        break;
    case 0xD4:
        lhs.type = h;
        RESOLVE_PAYLOAD(lhs);
        rhs.payload = 2;
        set(self, lhs, rhs);
        break;
    case 0xD5:
        lhs.type = l;
        RESOLVE_PAYLOAD(lhs);
        rhs.payload = 2;
        set(self, lhs, rhs);
        break;
    case 0xD6:
        lhs.type = phl;
        RESOLVE_PAYLOAD(lhs);
        rhs.payload = 2;
        set(self, lhs, rhs);
        break;
    case 0xD7:
        lhs.type = a;
        RESOLVE_PAYLOAD(lhs);
        rhs.payload = 2;
        set(self, lhs, rhs);
        break;
    case 0xD8:
        lhs.type = b;
        RESOLVE_PAYLOAD(lhs);
        rhs.payload = 3;
        set(self, lhs, rhs);
        break;
    case 0xD9:
        lhs.type = c;
        RESOLVE_PAYLOAD(lhs);
        rhs.payload = 3;
        set(self, lhs, rhs);
        break;
    case 0xDA:
        lhs.type = d;
        RESOLVE_PAYLOAD(lhs);
        rhs.payload = 3;
        set(self, lhs, rhs);
        break;
    case 0xDB:
        lhs.type = e;
        RESOLVE_PAYLOAD(lhs);
        rhs.payload = 3;
        set(self, lhs, rhs);
        break;
    case 0xDC:
        lhs.type = h;
        RESOLVE_PAYLOAD(lhs);
        rhs.payload = 3;
        set(self, lhs, rhs);
        break;
    case 0xDD:
        lhs.type = l;
        RESOLVE_PAYLOAD(lhs);
        rhs.payload = 3;
        set(self, lhs, rhs);
        break;
    case 0xDE:
        lhs.type = phl;
        RESOLVE_PAYLOAD(lhs);
        rhs.payload = 3;
        set(self, lhs, rhs);
        break;
    case 0xDF:
        lhs.type = a;
        RESOLVE_PAYLOAD(lhs);
        rhs.payload = 3;
        set(self, lhs, rhs);
        break;
    case 0xE0:
        lhs.type = b;
        RESOLVE_PAYLOAD(lhs);
        rhs.payload = 4;
        set(self, lhs, rhs);
        break;
    case 0xE1:
        lhs.type = c;
        RESOLVE_PAYLOAD(lhs);
        rhs.payload = 4;
        set(self, lhs, rhs);
        break;
    case 0xE2:
        lhs.type = d;
        RESOLVE_PAYLOAD(lhs);
        rhs.payload = 4;
        set(self, lhs, rhs);
        break;
    case 0xE3:
        lhs.type = e;
        RESOLVE_PAYLOAD(lhs);
        rhs.payload = 4;
        set(self, lhs, rhs);
        break;
    case 0xE4:
        lhs.type = h;
        RESOLVE_PAYLOAD(lhs);
        rhs.payload = 4;
        set(self, lhs, rhs);
        break;
    case 0xE5:
        lhs.type = l;
        RESOLVE_PAYLOAD(lhs);
        rhs.payload = 4;
        set(self, lhs, rhs);
        break;
    case 0xE6:
        lhs.type = phl;
        RESOLVE_PAYLOAD(lhs);
        rhs.payload = 4;
        set(self, lhs, rhs);
        break;
    case 0xE7:
        lhs.type = a;
        RESOLVE_PAYLOAD(lhs);
        rhs.payload = 4;
        set(self, lhs, rhs);
        break;
    case 0xE8:
        lhs.type = b;
        RESOLVE_PAYLOAD(lhs);
        rhs.payload = 5;
        set(self, lhs, rhs);
        break;
    case 0xE9:
        lhs.type = c;
        RESOLVE_PAYLOAD(lhs);
        rhs.payload = 5;
        set(self, lhs, rhs);
        break;
    case 0xEA:
        lhs.type = d;
        RESOLVE_PAYLOAD(lhs);
        rhs.payload = 5;
        set(self, lhs, rhs);
        break;
    case 0xEB:
        lhs.type = e;
        RESOLVE_PAYLOAD(lhs);
        rhs.payload = 5;
        set(self, lhs, rhs);
        break;
    case 0xEC:
        lhs.type = h;
        RESOLVE_PAYLOAD(lhs);
        rhs.payload = 5;
        set(self, lhs, rhs);
        break;
    case 0xED:
        lhs.type = l;
        RESOLVE_PAYLOAD(lhs);
        rhs.payload = 5;
        set(self, lhs, rhs);
        break;
    case 0xEE:
        lhs.type = phl;
        RESOLVE_PAYLOAD(lhs);
        rhs.payload = 5;
        set(self, lhs, rhs);
        break;
    case 0xEF:
        lhs.type = a;
        RESOLVE_PAYLOAD(lhs);
        rhs.payload = 5;
        set(self, lhs, rhs);
        break;
    case 0xF0:
        lhs.type = b;
        RESOLVE_PAYLOAD(lhs);
        rhs.payload = 6;
        set(self, lhs, rhs);
        break;
    case 0xF1:
        lhs.type = c;
        RESOLVE_PAYLOAD(lhs);
        rhs.payload = 6;
        set(self, lhs, rhs);
        break;
    case 0xF2:
        lhs.type = d;
        RESOLVE_PAYLOAD(lhs);
        rhs.payload = 6;
        set(self, lhs, rhs);
        break;
    case 0xF3:
        lhs.type = e;
        RESOLVE_PAYLOAD(lhs);
        rhs.payload = 6;
        set(self, lhs, rhs);
        break;
    case 0xF4:
        lhs.type = h;
        RESOLVE_PAYLOAD(lhs);
        rhs.payload = 6;
        set(self, lhs, rhs);
        break;
    case 0xF5:
        lhs.type = l;
        RESOLVE_PAYLOAD(lhs);
        rhs.payload = 6;
        set(self, lhs, rhs);
        break;
    case 0xF6:
        lhs.type = phl;
        RESOLVE_PAYLOAD(lhs);
        rhs.payload = 6;
        set(self, lhs, rhs);
        break;
    case 0xF7:
        lhs.type = a;
        RESOLVE_PAYLOAD(lhs);
        rhs.payload = 6;
        set(self, lhs, rhs);
        break;
    case 0xF8:
        lhs.type = b;
        RESOLVE_PAYLOAD(lhs);
        rhs.payload = 7;
        set(self, lhs, rhs);
        break;
    case 0xF9:
        lhs.type = c;
        RESOLVE_PAYLOAD(lhs);
        rhs.payload = 7;
        set(self, lhs, rhs);
        break;
    case 0xFA:
        lhs.type = d;
        RESOLVE_PAYLOAD(lhs);
        rhs.payload = 7;
        set(self, lhs, rhs);
        break;
    case 0xFB:
        lhs.type = e;
        RESOLVE_PAYLOAD(lhs);
        rhs.payload = 7;
        set(self, lhs, rhs);
        break;
    case 0xFC:
        lhs.type = h;
        RESOLVE_PAYLOAD(lhs);
        rhs.payload = 7;
        set(self, lhs, rhs);
        break;
    case 0xFD:
        lhs.type = l;
        RESOLVE_PAYLOAD(lhs);
        rhs.payload = 7;
        set(self, lhs, rhs);
        break;
    case 0xFE:
        lhs.type = phl;
        RESOLVE_PAYLOAD(lhs);
        rhs.payload = 7;
        set(self, lhs, rhs);
        break;
    case 0xFF:
        lhs.type = a;
        RESOLVE_PAYLOAD(lhs);
        rhs.payload = 7;
        set(self, lhs, rhs);
        break;
    default:
        PANIC("Unhandled opcode");
        break;
    }
}
