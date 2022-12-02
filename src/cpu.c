#include "cpu.h"
#include "decoder.h"
#include "instruction.h"
#include "stdio.h"
#include "utils.h"
#include <stdlib.h>
#include <string.h>

void cpu_write_bus(cpu *self, uint16_t addr, uint8_t n);
uint8_t cpu_read_bus(cpu *self, uint16_t addr);

__attribute((always_inline)) uint8_t get_reg_a(cpu *self) {
    return self->af.u8.a;
}
__attribute((always_inline)) uint8_t get_reg_f(cpu *self) {
    return self->af.u8.f.u8;
}
__attribute((always_inline)) uint8_t get_reg_b(cpu *self) {
    return self->bc.u8.b;
}
__attribute((always_inline)) uint8_t get_reg_c(cpu *self) {
    return self->bc.u8.c;
}
__attribute((always_inline)) uint8_t get_reg_d(cpu *self) {
    return self->de.u8.d;
}
__attribute((always_inline)) uint8_t get_reg_e(cpu *self) {
    return self->de.u8.e;
}
__attribute((always_inline)) uint8_t get_reg_h(cpu *self) {
    return self->hl.u8.h;
}
__attribute((always_inline)) uint8_t get_reg_l(cpu *self) {
    return self->hl.u8.l;
}
uint16_t get_reg_af(cpu *self) {
    return self->af.u16;
}
uint16_t get_reg_bc(cpu *self) {
    return self->bc.u16;
}
uint16_t get_reg_de(cpu *self) {
    return self->de.u16;
}
uint16_t get_reg_hl(cpu *self) {
    return self->hl.u16;
}
uint16_t get_reg_sp(cpu *self) {
    return self->sp;
}
uint8_t get_sp_u8(cpu *self) {
    return cpu_read_bus(self, self->sp++);
}
uint16_t get_sp_u16(cpu *self) {
    uint8_t hi = get_sp_u8(self);
    uint8_t lo = get_sp_u8(self);
    return hi << 8 | lo;
}
uint16_t get_pc(cpu *self) {
    return self->decoder.idx;
}
uint8_t get_flag_z(cpu *self) {
    return self->af.u8.f.bits.z;
}
uint8_t get_flag_n(cpu *self) {
    return self->af.u8.f.bits.n;
}
uint8_t get_flag_h(cpu *self) {
    return self->af.u8.f.bits.h;
}
uint8_t get_flag_c(cpu *self) {
    return self->af.u8.f.bits.c;
}
void set_reg_a(cpu *self, uint8_t n) {
    self->af.u8.a = n;
}
void set_reg_f(cpu *self, uint8_t n) {
    self->af.u8.f.u8 = n;
}
void set_reg_b(cpu *self, uint8_t n) {
    self->bc.u8.b = n;
}
void set_reg_c(cpu *self, uint8_t n) {
    self->bc.u8.c = n;
}
void set_reg_d(cpu *self, uint8_t n) {
    self->de.u8.d = n;
}
void set_reg_e(cpu *self, uint8_t n) {
    self->de.u8.e = n;
}
void set_reg_h(cpu *self, uint8_t n) {
    self->hl.u8.h = n;
}
void set_reg_l(cpu *self, uint8_t n) {
    self->hl.u8.l = n;
}
void set_reg_af(cpu *self, uint16_t n) {
    self->af.u16 = n;
}
void set_reg_bc(cpu *self, uint16_t n) {
    self->bc.u16 = n;
}
void set_reg_de(cpu *self, uint16_t n) {
    self->de.u16 = n;
}
void set_reg_hl(cpu *self, uint16_t n) {
    self->hl.u16 = n;
}
void set_reg_sp(cpu *self, uint16_t n) {
    self->sp = n;
}
void set_flag_z(cpu *self, bool z) {
    self->af.u8.f.bits.z = z;
}
void set_flag_n(cpu *self, bool z) {
    self->af.u8.f.bits.n = z;
}
void set_flag_h(cpu *self, bool z) {
    self->af.u8.f.bits.h = z;
}
void set_flag_c(cpu *self, bool z) {
    self->af.u8.f.bits.c = z;
}
void set_flag_h_add(cpu *self, uint16_t a, uint16_t b) {
    set_flag_h(self, (get_reg_f(self) & ((((a & 0xf0) + (b & 0xf0)) & 0x100) == 0x100 << 4)) == 1);
}
void set_flag_c_add(cpu *self, uint16_t a, uint16_t b) {
    set_flag_c(self, (get_reg_f(self) & ((((a & 0xf) + (b & 0xf)) & 0x10) == 0x10)) == 1);
}
void set_sp(cpu *self, uint16_t n) {
    self->sp = n;
}
void set_sp_u8(cpu *self, uint8_t n) {
    cpu_write_bus(self, --self->sp, n);
}
void set_sp_u16(cpu *self, uint16_t n) {
    cpu_write_bus(self, --self->sp, n & 0x00FF);
    cpu_write_bus(self, --self->sp, (n >> 8) & 0x00FF);
}
void set_pc(cpu *self, uint16_t n) {
    self->decoder.idx = n;
}

cpu cpu_new(bus *bus) {
    cpu c;
    c.af.u16 = 0x0000;
    c.bc.u16 = 0x0000;
    c.de.u16 = 0x0000;
    c.hl.u16 = 0x0000;
    c.sp = 0xFFFE;
    c.mode = cpu_running_mode_e;
    c.bus = bus;
    c.clocks = 0x0000;
    c.decoder = decoder_new(bus->bootrom, BOOTROM_SIZE);
    return c;
}

void cpu_write_bus(cpu *self, uint16_t addr, uint8_t n) {
    bus_write(self->bus, addr, n);
}

uint8_t cpu_read_bus(cpu *self, uint16_t addr) {
    return bus_read(self->bus, addr);
}

uint16_t get_rhs(cpu *self, argument_t *rhs) {
    switch (rhs->e) {
    case none_e:
        PANIC("Attempted to read from none_e rhs!");
        return 0;
    case condition_e:
        PANIC("Attempted to read from condition_e rhs!");
        return 0;
    case register_e:
        switch (rhs->p.register_p) {
        case a_register_p:
            return get_reg_a(self);
        case f_register_p:
            return get_reg_f(self);
        case b_register_p:
            return get_reg_b(self);
        case c_register_p:
            return get_reg_c(self);
        case d_register_p:
            return get_reg_d(self);
        case e_register_p:
            return get_reg_e(self);
        case h_register_p:
            return get_reg_h(self);
        case l_register_p:
            return get_reg_l(self);
        case af_register_p:
            return get_reg_af(self);
        case bc_register_p:
            return get_reg_bc(self);
        case de_register_p:
            return get_reg_de(self);
        case hl_register_p:
            return get_reg_hl(self);
        case sp_register_p:
            return get_reg_sp(self);
        }
    case io_offset_u8_e:
        return bus_read(self->bus, 0xff00 + rhs->p.io_offset_u8_p);
    case io_offset_c_e:
        return bus_read(self->bus, 0xff00 + get_reg_c(self));
    case hl_ptr_e:
        switch (rhs->p.hl_ptr_p) {
        case hl_ptr_dec_e: {
            uint16_t hl = get_reg_hl(self);
            uint8_t res = bus_read(self->bus, hl);
            set_reg_hl(self, hl - 1);
            return res;
        }
        case hl_ptr_inc_e: {
            uint16_t hl = get_reg_hl(self);
            uint8_t res = bus_read(self->bus, hl);
            set_reg_hl(self, hl + 1);
            return res;
        }
        }
    case register_ptr_e:
        switch (rhs->p.register_ptr_p) {
        case bc_register_ptr_e:
            return bus_read(self->bus, get_reg_bc(self));
        case de_register_ptr_e:
            return bus_read(self->bus, get_reg_de(self));
        case hl_register_ptr_e:
            return bus_read(self->bus, get_reg_hl(self));
        }
    case imm_u8_e:
        return rhs->p.imm_u8_p;
    case imm_i8_e:
        return (int8_t)rhs->p.imm_u8_p;
    case imm_u16_e:
        return rhs->p.imm_u16_p;
    case imm_u16_ptr_e:
        return bus_read(self->bus, rhs->p.imm_u16_ptr_p);
    case fixed_payload_e:
        return rhs->p.fixed_payload_p;
    case sp_offset_e:
        return get_reg_sp(self) + rhs->p.sp_offset_p;
    }
}

void set_lhs(cpu *self, argument_t *lhs, uint16_t n) {
    switch (lhs->e) {
    case none_e:
        PANIC("Attempted to write to none_e lhs!");
        break;
    case condition_e:
        PANIC("Attempted to write to condition_e lhs!");
        break;
    case register_e:
        switch (lhs->p.register_p) {
        case a_register_p:
            set_reg_a(self, (uint8_t)n);
            break;
        case f_register_p:
            set_reg_f(self, (uint8_t)n);
            break;
        case b_register_p:
            set_reg_b(self, (uint8_t)n);
            break;
        case c_register_p:
            set_reg_c(self, (uint8_t)n);
            break;
        case d_register_p:
            set_reg_d(self, (uint8_t)n);
            break;
        case e_register_p:
            set_reg_e(self, (uint8_t)n);
            break;
        case h_register_p:
            set_reg_h(self, (uint8_t)n);
            break;
        case l_register_p:
            set_reg_l(self, (uint8_t)n);
            break;
        case af_register_p:
            set_reg_af(self, n);
            break;
        case bc_register_p:
            set_reg_bc(self, n);
            break;
        case de_register_p:
            set_reg_de(self, n);
            break;
        case hl_register_p:
            set_reg_hl(self, n);
            break;
        case sp_register_p:
            set_reg_sp(self, n);
            break;
        }
        break;
    case io_offset_u8_e:
        switch (get_raw_size_argument_t(lhs)) {
        case 0:
            PANIC("Attempted to write ZST!");
            break;
        case 1:
            bus_write(self->bus, 0xff00 + lhs->p.io_offset_u8_p, (uint8_t)(n & 0xFF));
            break;
        case 2:
            bus_write(self->bus, 0xff00 + lhs->p.io_offset_u8_p, (uint8_t)(n & 0xFF));
            bus_write(self->bus, 0xff00 + lhs->p.io_offset_u8_p + 1, (uint8_t)((n >> 8) & 0xFF));
            break;
        }
        break;
    case io_offset_c_e: {
        uint8_t c = get_reg_c(self);
        switch (get_raw_size_argument_t(lhs)) {
        case 0:
            PANIC("Attempted to write ZST!");
            break;
        case 1:
            bus_write(self->bus, 0xff00 + c, (uint8_t)(n & 0xFF));
            break;
        case 2:
            bus_write(self->bus, 0xff00 + c, (uint8_t)(n & 0xFF));
            bus_write(self->bus, 0xff00 + c + 1, (uint8_t)((n >> 8) & 0xFF));
            break;
        }
        break;
    }
    case hl_ptr_e: {
        uint16_t hl = get_reg_hl(self);
        switch (get_raw_size_argument_t(lhs)) {
        case 0:
            PANIC("Attempted to write ZST!");
            break;
        case 1:
            bus_write(self->bus, hl, (uint8_t)n);
            break;
        case 2:
            bus_write(self->bus, hl, (uint8_t)(n & 0xFF));
            bus_write(self->bus, hl + 1, (uint8_t)((n >> 8) & 0xFF));
            break;
        }
        switch (lhs->p.hl_ptr_p) {
        case hl_ptr_inc_e:
            set_reg_hl(self, hl + 1);
            break;
        case hl_ptr_dec_e:
            set_reg_hl(self, hl - 1);
            break;
        }
        break;
    }
    case register_ptr_e: {
        uint16_t addr;
        switch (lhs->p.register_ptr_p) {
        case bc_register_ptr_e:
            addr = get_reg_bc(self);
            break;
        case de_register_ptr_e:
            addr = get_reg_de(self);
            break;
        case hl_register_ptr_e:
            addr = get_reg_hl(self);
            break;
        }
        switch (get_raw_size_argument_t(lhs)) {
        case 0:
            PANIC("Attempted to write ZST!");
            break;
        case 1:
            bus_write(self->bus, addr, (uint8_t)n);
            break;
        case 2:
            bus_write(self->bus, addr, (uint8_t)(n & 0xFF));
            bus_write(self->bus, addr + 1, (uint8_t)((n >> 8) & 0xFF));
            break;
        }
        break;
    }
    case imm_u8_e:
        PANIC("Attempted to write to imm_u8_e lhs!");
        break;
    case imm_i8_e:
        PANIC("Attempted to write to imm_i8_e lhs!");
        break;
    case imm_u16_e:
        PANIC("Attempted to write to imm_u16_e lhs!");
        break;
    case imm_u16_ptr_e:
        switch (get_raw_size_argument_t(lhs)) {
        case 0:
            PANIC("Attempted to write ZST!");
            break;
        case 1:
            bus_write(self->bus, lhs->p.imm_u16_ptr_p, (uint8_t)(n & 0xFF));
            break;
        case 2:
            bus_write(self->bus, lhs->p.imm_u16_ptr_p, (uint8_t)(n & 0xFF));
            bus_write(self->bus, lhs->p.imm_u16_ptr_p + 1, (uint8_t)((n >> 8) & 0xFF));
            break;
        }
        break;
    case fixed_payload_e:
        PANIC("Attempted to write to fixed_payload_e lhs!");
        break;
    case sp_offset_e:
        PANIC("Attempted to write to sp_offset_e lhs!");
        break;
    }
}

bool resolve_cond(cpu *self, argument_t *cond) {
#ifdef TESTING
    if (cond->e != condition_e)
        PANIC("Attempted to resolve condition with non-condition payload!");
#endif
    switch (cond->p.condition_p) {
    case none_condition_e:
        return true;
    case nzero_condition_e:
        return get_flag_z(self) == 0;
    case zero_condition_e:
        return get_flag_z(self) == 1;
    case ncarry_condition_e:
        return get_flag_c(self) == 0;
    case carry_condition_e:
        return get_flag_c(self) == 1;
    }
}

void noop(cpu *self, argument_t lhs, argument_t rhs) {
    (void)self;
    (void)lhs;
    (void)rhs;
}

void ld(cpu *self, argument_t lhs, argument_t rhs) {
    set_lhs(self, &lhs, get_rhs(self, &rhs));
}

void inc(cpu *self, argument_t lhs, argument_t rhs) {
    uint16_t res = get_rhs(self, &lhs) + 1;
    (void)rhs;
    set_lhs(self, &lhs, res);
    switch (lhs.p.register_p) {
    case bc_register_p:
    case de_register_p:
    case hl_register_p:
    case sp_register_p:
        break;
    default:
        set_flag_z(self, res == 0);
        set_flag_n(self, 0);
        set_flag_h_add(self, res - 1, 1);
        break;
    }
}

void dec(cpu *self, argument_t lhs, argument_t rhs) {
    uint16_t res = get_rhs(self, &lhs) - 1;
    (void)rhs;
    set_lhs(self, &lhs, res);
    switch (lhs.p.register_p) {
    case bc_register_p:
    case de_register_p:
    case hl_register_p:
    case sp_register_p:
        break;
    default:
        set_flag_z(self, res == 0);
        set_flag_n(self, 0);
        set_flag_h_add(self, res + 1, 1);
        break;
    }
}

void add(cpu *self, argument_t lhs, argument_t rhs) {
    uint16_t res;
    res = get_rhs(self, &lhs) + get_rhs(self, &rhs);
    set_lhs(self, &lhs, res);
    switch (lhs.p.register_p) {
    case a_register_p:
        set_flag_z(self, res == 0);
        break;
    case sp_register_p:
        set_flag_z(self, 0);
        break;
    default:
        break;
    }
    set_flag_n(self, 0);
    set_flag_h_add(self, get_rhs(self, &lhs), get_rhs(self, &rhs));
    set_flag_c_add(self, get_rhs(self, &lhs), get_rhs(self, &rhs));
}

void adc(cpu *self, argument_t lhs, argument_t rhs) {
    uint16_t res;
    res = get_rhs(self, &lhs) + get_rhs(self, &rhs) + get_flag_c(self);
    set_lhs(self, &lhs, res);
    set_flag_z(self, res == 0);
    set_flag_n(self, 0);
    set_flag_h_add(self, get_rhs(self, &lhs), get_rhs(self, &rhs) + get_flag_c(self));
    set_flag_c_add(self, get_rhs(self, &lhs), get_rhs(self, &rhs) + get_flag_c(self));
}

void sub(cpu *self, argument_t lhs, argument_t rhs) {
    uint16_t res;
    uint16_t old_lhs = get_rhs(self, &lhs);
    uint16_t old_rhs = get_rhs(self, &rhs);
    res = old_lhs - old_rhs;
    set_lhs(self, &lhs, res);
    set_flag_z(self, res == 0);
    set_flag_n(self, 1);
    /*
     * TODO:
     * I don't know if SET_FLAG_{H,C} properly handle subtraction
     */
    set_flag_h_add(self, old_lhs, old_rhs);
    set_flag_c_add(self, old_rhs, old_rhs);
}

void sbc(cpu *self, argument_t lhs, argument_t rhs) {
    uint16_t res;
    uint16_t old_lhs = get_rhs(self, &lhs);
    uint16_t old_rhs = get_rhs(self, &rhs);
    res = old_lhs - old_rhs - get_flag_c(self);
    set_lhs(self, &lhs, res);
    set_flag_z(self, res == 0);
    set_flag_n(self, 1);
    set_flag_h_add(self, old_lhs, old_rhs - get_flag_c(self));
    set_flag_c_add(self, old_lhs, old_rhs - get_flag_c(self));
}

void andReg(cpu *self, argument_t lhs, argument_t rhs) {
    uint16_t res;
    res = get_rhs(self, &lhs) & get_rhs(self, &rhs);
    set_lhs(self, &lhs, res);
    set_flag_z(self, res == 0);
    set_flag_n(self, 0);
    set_flag_h(self, 1);
    set_flag_c(self, 0);
}

void xorReg(cpu *self, argument_t lhs, argument_t rhs) {
    uint16_t res;
    res = get_rhs(self, &lhs) ^ get_rhs(self, &rhs);
    set_lhs(self, &lhs, res);
    set_flag_z(self, res == 0);
    set_flag_n(self, 0);
    set_flag_h(self, 0);
    set_flag_c(self, 0);
}

void orReg(cpu *self, argument_t lhs, argument_t rhs) {
    uint16_t res;
    res = get_rhs(self, &lhs) | get_rhs(self, &rhs);
    set_lhs(self, &lhs, res);
    set_flag_z(self, res == 0);
    set_flag_n(self, 0);
    set_flag_h(self, 0);
    set_flag_c(self, 0);
}

void cp(cpu *self, argument_t lhs, argument_t rhs) {
    uint16_t res;
    uint16_t old_lhs = get_rhs(self, &lhs);
    uint16_t old_rhs = get_rhs(self, &rhs);
    res = old_lhs - old_rhs;
    set_flag_z(self, res == 0);
    set_flag_n(self, 1);
    set_flag_h_add(self, old_lhs, old_rhs);
    set_flag_c_add(self, old_lhs, old_rhs);
}

void ret(cpu *self, argument_t lhs, argument_t rhs) {
    (void)rhs;
    if (resolve_cond(self, &lhs)) {
        set_pc(self, get_sp_u16(self));
    }
}

void jp(cpu *self, argument_t lhs, argument_t rhs) {
    if (resolve_cond(self, &lhs)) {
        set_pc(self, get_rhs(self, &rhs));
    }
}

void rlc(cpu *self, argument_t lhs, argument_t rhs) {
    uint16_t res;
    uint16_t old_lhs = get_rhs(self, &lhs);
    (void)rhs;
    res = (old_lhs << 1) | (old_lhs >> ((get_raw_size_argument_t(&lhs) << 3) - 1));
    set_lhs(self, &lhs, res);
    set_flag_z(self, res == 0);
    set_flag_n(self, false);
    set_flag_h(self, false);
    set_flag_c(self, old_lhs >> 7);
}

void rrc(cpu *self, argument_t lhs, argument_t rhs) {
    uint16_t res;
    uint16_t old_lhs = get_rhs(self, &lhs);
    (void)rhs;
    res = (old_lhs >> 1) | (old_lhs << ((get_raw_size_argument_t(&lhs) << 3) - 1));
    set_lhs(self, &lhs, res);
    set_flag_z(self, res == 0);
    set_flag_n(self, false);
    set_flag_h(self, false);
    set_flag_c(self, old_lhs & 0x01);
}

void rl(cpu *self, argument_t lhs, argument_t rhs) {
    uint16_t res;
    uint16_t old_lhs = get_rhs(self, &lhs);
    (void)rhs;
    res = (old_lhs << 1) | (get_flag_c(self));
    set_lhs(self, &lhs, res);
    set_flag_z(self, res == 0);
    set_flag_n(self, false);
    set_flag_h(self, false);
    set_flag_c(self, old_lhs >> 7);
}

void rr(cpu *self, argument_t lhs, argument_t rhs) {
    uint16_t res;
    uint16_t old_lhs = get_rhs(self, &lhs);
    (void)rhs;
    res = (get_flag_c(self)) | (old_lhs >> 1);
    set_lhs(self, &lhs, res);
    set_flag_z(self, res == 0);
    set_flag_n(self, false);
    set_flag_h(self, false);
    set_flag_c(self, old_lhs & 0x01);
}

void sla(cpu *self, argument_t lhs, argument_t rhs) {
    uint16_t res;
    uint16_t old_lhs = get_rhs(self, &lhs);
    (void)rhs;
    res = (old_lhs << 1);
    set_lhs(self, &lhs, res);
    set_flag_z(self, res == 0);
    set_flag_n(self, false);
    set_flag_h(self, false);
    set_flag_c(self, old_lhs >> 7);
}

void sra(cpu *self, argument_t lhs, argument_t rhs) {
    uint16_t res;
    uint16_t old_lhs = get_rhs(self, &lhs);
    (void)rhs;
    res = (old_lhs & 0x80) | (old_lhs >> 1);
    set_lhs(self, &lhs, res);
    set_flag_z(self, res == 0);
    set_flag_n(self, false);
    set_flag_h(self, false);
    set_flag_c(self, old_lhs & 0x01);
}

void bit(cpu *self, argument_t lhs, argument_t rhs) {
    if ((get_rhs(self, &rhs) >> get_rhs(self, &lhs)) == 0)
        set_flag_z(self, true);
    else
        set_flag_z(self, false);
    set_flag_n(self, false);
    set_flag_h(self, true);
}

void res(cpu *self, argument_t lhs, argument_t rhs) {
    set_lhs(self, &lhs, get_rhs(self, &rhs) | ~(1 << get_rhs(self, &lhs)));
}

void set(cpu *self, argument_t lhs, argument_t rhs) {
    set_lhs(self, &lhs, get_rhs(self, &rhs) | (1 << get_rhs(self, &lhs)));
}

void swap(cpu *self, argument_t lhs, argument_t rhs) {
    uint16_t res;
    (void)rhs;
    res = (get_rhs(self, &lhs) & 0x0F) | (get_rhs(self, &lhs) >> 4);
    set_lhs(self, &lhs, res);
    set_flag_z(self, res == 0);
    set_flag_n(self, false);
    set_flag_h(self, false);
    set_flag_c(self, false);
}

void srl(cpu *self, argument_t lhs, argument_t rhs) {
    uint16_t res;
    uint16_t old_lhs = get_rhs(self, &lhs);
    (void)rhs;
    res = (old_lhs >> 1);
    set_lhs(self, &lhs, res);
    set_flag_z(self, res == 0);
    set_flag_n(self, false);
    set_flag_h(self, false);
    set_flag_c(self, old_lhs & 0x01);
}

void jr(cpu *self, argument_t lhs, argument_t rhs) {
    if (resolve_cond(self, &lhs)) {
        self->clocks++;
        set_pc(self, get_pc(self) + get_rhs(self, &rhs));
    }
}

void push(cpu *self, argument_t lhs, argument_t rhs) {
    (void)rhs;
    set_sp_u16(self, get_rhs(self, &lhs));
}

void pop(cpu *self, argument_t lhs, argument_t rhs) {
    (void)rhs;
    set_lhs(self, &lhs, get_sp_u16(self));
}

void rla(cpu *self, argument_t lhs, argument_t rhs) {
    uint8_t res;
    uint8_t a = get_reg_a(self);
    (void)lhs;
    (void)rhs;
    res = (a << 1) | get_flag_c(self);
    set_reg_a(self, res);
    set_flag_z(self, res == 0x0);
    set_flag_n(self, false);
    set_flag_h(self, false);
    set_flag_c(self, a >> 7);
}

void di(cpu *self, argument_t lhs, argument_t rhs) {
    (void)self;
    (void)lhs;
    (void)rhs;
    UNIMPLEMENTED("di");
}

void ei(cpu *self, argument_t lhs, argument_t rhs) {
    (void)self;
    (void)lhs;
    (void)rhs;
    UNIMPLEMENTED("ei");
}

void halt(cpu *self, argument_t lhs, argument_t rhs) {
    (void)lhs;
    (void)rhs;
    self->mode = cpu_halted_mode_e;
}

void reti(cpu *self, argument_t lhs, argument_t rhs) {
    (void)self;
    (void)lhs;
    (void)rhs;
    UNIMPLEMENTED("reti");
}

void rlca(cpu *self, argument_t lhs, argument_t rhs) {
    uint8_t res;
    uint8_t a = get_reg_a(self);
    (void)lhs;
    (void)rhs;
    res = (a << 1) | (a >> 7);
    set_reg_a(self, res);
    set_flag_z(self, res == 0x0);
    set_flag_n(self, false);
    set_flag_h(self, false);
    set_flag_c(self, res & 0x1);
}

void rra(cpu *self, argument_t lhs, argument_t rhs) {
    uint8_t res;
    uint8_t a = get_reg_a(self);
    (void)lhs;
    (void)rhs;
    res = (get_flag_c(self) << 7) | (a >> 1);
    set_reg_a(self, res);
    set_flag_z(self, res == 0);
    set_flag_n(self, false);
    set_flag_h(self, false);
    set_flag_c(self, a & 0x1);
}

void rrca(cpu *self, argument_t lhs, argument_t rhs) {
    uint8_t res;
    uint8_t a = get_reg_a(self);
    (void)lhs;
    (void)rhs;
    res = (a & 0x1 << 7) | (a >> 1);
    set_reg_a(self, res);
    set_flag_z(self, res == 0x0);
    set_flag_n(self, false);
    set_flag_h(self, false);
    set_flag_c(self, a & 0x1);
}

void rst(cpu *self, argument_t lhs, argument_t rhs) {
    (void)self;
    (void)lhs;
    (void)rhs;
    UNIMPLEMENTED("rst");
}

void call(cpu *self, argument_t lhs, argument_t rhs) {
    uint16_t pc = get_pc(self);
    if (resolve_cond(self, &lhs)) {
        set_sp_u16(self, pc);
        set_pc(self, get_rhs(self, &rhs));
        self->clocks += 3;
    }
}

void ccf(cpu *self, argument_t lhs, argument_t rhs) {
    uint8_t c = get_flag_c(self);
    (void)lhs;
    (void)rhs;
    set_flag_n(self, false);
    set_flag_h(self, false);
    set_flag_c(self, (~c) & 0x1);
}

void cpl(cpu *self, argument_t lhs, argument_t rhs) {
    uint8_t a = get_reg_a(self);
    (void)lhs;
    (void)rhs;
    set_reg_a(self, ~a);
    set_flag_n(self, true);
    set_flag_h(self, true);
}

void stop(cpu *self, argument_t lhs, argument_t rhs) {
    (void)lhs;
    (void)rhs;
    self->mode = cpu_stop_mode_e;
}

/* https://forums.nesdev.org/viewtopic.php?t=15944 */
void daa(cpu *self, argument_t lhs, argument_t rhs) {
    uint8_t a = get_reg_a(self);
    (void)lhs;
    (void)rhs;
    if (!get_flag_n(self)) {
        if (get_flag_c(self) || a > 0x99) {
            set_reg_a(self, a + 0x60);
            set_flag_c(self, true);
        }
        if (get_flag_h(self) || (a & 0x0f) > 0x09) {
            set_reg_a(self, a + 0x06);
        }
    } else {
        if (get_flag_c(self))
            set_reg_a(self, a - 0x60);
        if (get_flag_h(self))
            set_reg_a(self, a - 0x06);
    }
    set_flag_z(self, get_reg_a(self) == 0x00);
    set_flag_h(self, false);
}

void scf(cpu *self, argument_t lhs, argument_t rhs) {
    (void)lhs;
    (void)rhs;
    set_flag_n(self, false);
    set_flag_h(self, false);
    set_flag_c(self, true);
}

uintptr_t cpu_clock(cpu *self) {
    instruction_t instr;
    uintptr_t old_clocks;
    if (self->mode != cpu_running_mode_e)
        return 0;
    LOG("CPU", "Clocks %#lu", self->clocks);
    /* LOG("CPU", "Reading address %#04x", self->pc); */
    old_clocks = self->clocks;
    if (self->sp == 0x100) {
        PANIC("finished boot");
    }
    instr = decoder_next(&self->decoder);

    {
        char *instr_str = print_instruction(&instr);
        LOG("CPU", "%s", instr_str);
        free(instr_str);
    }
    switch (instr.instruction_type) {
    case adc_instruction:
        adc(self, instr.lhs, instr.rhs);
        break;
    case add_instruction:
        add(self, instr.lhs, instr.rhs);
        break;
    case and_instruction:
        andReg(self, instr.lhs, instr.rhs);
        break;
    case bit_instruction:
        bit(self, instr.lhs, instr.rhs);
        break;
    case call_instruction:
        call(self, instr.lhs, instr.rhs);
        break;
    case ccf_instruction:
        ccf(self, instr.lhs, instr.rhs);
        break;
    case cp_instruction:
        cp(self, instr.lhs, instr.rhs);
        break;
    case cpl_instruction:
        cpl(self, instr.lhs, instr.rhs);
        break;
    case daa_instruction:
        daa(self, instr.lhs, instr.rhs);
        break;
    case dec_instruction:
        dec(self, instr.lhs, instr.rhs);
        break;
    case di_instruction:
        di(self, instr.lhs, instr.rhs);
        break;
    case ei_instruction:
        ei(self, instr.lhs, instr.rhs);
        break;
    case halt_instruction:
        halt(self, instr.lhs, instr.rhs);
        break;
    case inc_instruction:
        inc(self, instr.lhs, instr.rhs);
        break;
    case jp_instruction:
        jp(self, instr.lhs, instr.rhs);
        break;
    case jr_instruction:
        jr(self, instr.lhs, instr.rhs);
        break;
    case ld_instruction:
        ld(self, instr.lhs, instr.rhs);
        break;
    case noop_instruction:
        noop(self, instr.lhs, instr.rhs);
        break;
    case or_instruction:
        orReg(self, instr.lhs, instr.rhs);
        break;
    case pop_instruction:
        pop(self, instr.lhs, instr.rhs);
        break;
    case push_instruction:
        push(self, instr.lhs, instr.rhs);
        break;
    case res_instruction:
        res(self, instr.lhs, instr.rhs);
        break;
    case ret_instruction:
        ret(self, instr.lhs, instr.rhs);
        break;
    case reti_instruction:
        reti(self, instr.lhs, instr.rhs);
        break;
    case rl_instruction:
        rl(self, instr.lhs, instr.rhs);
        break;
    case rla_instruction:
        rla(self, instr.lhs, instr.rhs);
        break;
    case rlca_instruction:
        rlca(self, instr.lhs, instr.rhs);
        break;
    case rlc_instruction:
        rlc(self, instr.lhs, instr.rhs);
        break;
    case rr_instruction:
        rr(self, instr.lhs, instr.rhs);
        break;
    case rra_instruction:
        rra(self, instr.lhs, instr.rhs);
        break;
    case rrca_instruction:
        rrca(self, instr.lhs, instr.rhs);
        break;
    case rrc_instruction:
        rrc(self, instr.lhs, instr.rhs);
        break;
    case rst_instruction:
        rst(self, instr.lhs, instr.rhs);
        break;
    case sbc_instruction:
        sbc(self, instr.lhs, instr.rhs);
        break;
    case scf_instruction:
        scf(self, instr.lhs, instr.rhs);
        break;
    case set_instruction:
        set(self, instr.lhs, instr.rhs);
        break;
    case sla_instruction:
        sla(self, instr.lhs, instr.rhs);
        break;
    case sra_instruction:
        sra(self, instr.lhs, instr.rhs);
        break;
    case srl_instruction:
        srl(self, instr.lhs, instr.rhs);
        break;
    case stop_instruction:
        stop(self, instr.lhs, instr.rhs);
        break;
    case sub_instruction:
        sub(self, instr.lhs, instr.rhs);
        break;
    case swap_instruction:
        swap(self, instr.lhs, instr.rhs);
        break;
    case xor_instruction:
        xorReg(self, instr.lhs, instr.rhs);
        break;
    case illegal_instruction:
        PANIC("Illegal instruction encountered");
        break;
    }
    self->clocks += instr.clocks;

    return self->clocks - old_clocks;
}
