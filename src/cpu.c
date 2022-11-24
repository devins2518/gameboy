#include "cpu.h"
#include "cpu_utils.h"
#include "stdio.h"
#include "utils.h"
#include <stdlib.h>
#include <string.h>

void cpu_write_bus(cpu *self, uint16_t addr, uint8_t n);
uint8_t cpu_read_bus(cpu *self, uint16_t addr);

__attribute((always_inline)) uint8_t get_reg_a(cpu *self) { return self->af.u8.a; }
__attribute((always_inline)) uint8_t get_reg_f(cpu *self) { return self->af.u8.f.u8; }
__attribute((always_inline)) uint8_t get_reg_b(cpu *self) { return self->bc.u8.b; }
__attribute((always_inline)) uint8_t get_reg_c(cpu *self) { return self->bc.u8.c; }
__attribute((always_inline)) uint8_t get_reg_d(cpu *self) { return self->de.u8.d; }
__attribute((always_inline)) uint8_t get_reg_e(cpu *self) { return self->de.u8.e; }
__attribute((always_inline)) uint8_t get_reg_h(cpu *self) { return self->hl.u8.h; }
__attribute((always_inline)) uint8_t get_reg_l(cpu *self) { return self->hl.u8.l; }
uint16_t get_reg_af(cpu *self) { return self->af.u16; }
uint16_t get_reg_bc(cpu *self) { return self->bc.u16; }
uint16_t get_reg_de(cpu *self) { return self->de.u16; }
uint16_t get_reg_hl(cpu *self) { return self->hl.u16; }
uint8_t get_reg_paf(cpu *self) { return cpu_read_bus(self, get_reg_af(self)); }
uint8_t get_reg_pbc(cpu *self) { return cpu_read_bus(self, get_reg_bc(self)); }
uint8_t get_reg_pde(cpu *self) { return cpu_read_bus(self, get_reg_de(self)); }
uint8_t get_reg_phl(cpu *self) { return cpu_read_bus(self, get_reg_hl(self)); }
uint16_t get_sp(cpu *self) { return self->sp; }
uint16_t get_pc(cpu *self) { return self->pc; }
uint8_t get_flag_z(cpu *self) { return ((get_reg_f(self) >> 7) & 0x01); }
uint8_t get_flag_n(cpu *self) { return ((get_reg_f(self) >> 6) & 0x01); }
uint8_t get_flag_h(cpu *self) { return ((get_reg_f(self) >> 5) & 0x01); }
uint8_t get_flag_c(cpu *self) { return ((get_reg_f(self) >> 4) & 0x01); }
void set_reg_a(cpu *self, uint8_t n) { self->af.u8.a = n; }
void set_reg_f(cpu *self, uint8_t n) { self->af.u8.f.u8 = n; }
void set_reg_b(cpu *self, uint8_t n) { self->bc.u8.b = n; }
void set_reg_c(cpu *self, uint8_t n) { self->bc.u8.c = n; }
void set_reg_d(cpu *self, uint8_t n) { self->de.u8.d = n; }
void set_reg_e(cpu *self, uint8_t n) { self->de.u8.e = n; }
void set_reg_h(cpu *self, uint8_t n) { self->hl.u8.h = n; }
void set_reg_l(cpu *self, uint8_t n) { self->hl.u8.l = n; }
void set_reg_af(cpu *self, uint16_t n) { self->af.u16 = n; }
void set_reg_bc(cpu *self, uint16_t n) { self->bc.u16 = n; }
void set_reg_de(cpu *self, uint16_t n) { self->de.u16 = n; }
void set_reg_hl(cpu *self, uint16_t n) { self->hl.u16 = n; }
void set_reg_paf(cpu *self, uint8_t n) { cpu_write_bus(self, get_reg_af(self), n); }
void set_reg_pbc(cpu *self, uint8_t n) { cpu_write_bus(self, get_reg_bc(self), n); }
void set_reg_pde(cpu *self, uint8_t n) { cpu_write_bus(self, get_reg_de(self), n); }
void set_reg_phl(cpu *self, uint8_t n) { cpu_write_bus(self, get_reg_hl(self), n); }
void set_flag_z(cpu *self, bool z) { self->af.u8.f.bits.z = z; }
void set_flag_n(cpu *self, bool z) { self->af.u8.f.bits.n = z; }
void set_flag_h(cpu *self, bool z) { self->af.u8.f.bits.h = z; }
void set_flag_c(cpu *self, bool z) { self->af.u8.f.bits.c = z; }
void set_flag_h_add(cpu *self, uint16_t a, uint16_t b) {
    set_flag_h(self, (get_reg_f(self) & ((((a & 0xf0) + (b & 0xf0)) & 0x100) == 0x100 << 4)) == 1);
}
void set_flag_c_add(cpu *self, uint16_t a, uint16_t b) {
    set_flag_c(self, (get_reg_f(self) & ((((a & 0xf) + (b & 0xf)) & 0x10) == 0x10)) == 1);
}
void set_sp(cpu *self, uint16_t n) { self->sp = n; }
void set_pc(cpu *self, uint16_t n) { self->pc = n; }
void set_reg(cpu *self, argument_t r, uint16_t n) {
    switch (r.type) {
    case a:
        set_reg_a(self, n);
        break;
    case f:
        set_reg_f(self, n);
        break;
    case b:
        set_reg_b(self, n);
        break;
    case c:
        set_reg_c(self, n);
        break;
    case d:
        set_reg_d(self, n);
        break;
    case e:
        set_reg_e(self, n);
        break;
    case h:
        set_reg_h(self, n);
        break;
    case l:
        set_reg_l(self, n);
        break;
    case af:
        set_reg_af(self, n);
        break;
    case bc:
        set_reg_bc(self, n);
        break;
    case de:
        set_reg_de(self, n);
        break;
    case hl:
        set_reg_hl(self, n);
        break;
    case sp:
        set_sp(self, n);
        break;
    case pc:
        set_pc(self, n);
        break;
    case p:
        if (r.payload <= 0xFF)
            cpu_write_bus(self, r.payload, n);
        else {
            cpu_write_bus(self, r.payload, n & 0xFF);
            cpu_write_bus(self, r.payload + 1, (n >> 8) & 0xFF);
        }

        break;
    case paf:
        set_reg_paf(self, n);
        break;
    case pbc:
        set_reg_pbc(self, n);
        break;
    case pde:
        set_reg_pde(self, n);
        break;
    case phl:
        set_reg_phl(self, n);
        break;
    case phli:
        set_reg_phl(self, n);
        self->hl.u16++;
        break;
    case phld:
        set_reg_phl(self, n);
        self->hl.u16--;
        break;
    default:
        fflush(stdout);
        fprintf(stderr, "Could not assign to type: %s\n", ARGUMENT_NAME[r.type]);
        abort();
    }
}
void set_arg_payload(cpu *self, argument_t *arg) {
    switch (arg->type) {
    case a:
        arg->payload = get_reg_a(self);
        break;
    case f:
        arg->payload = get_reg_f(self);
        break;
    case b:
        arg->payload = get_reg_b(self);
        break;
    case c:
        arg->payload = get_reg_c(self);
        break;
    case d:
        arg->payload = get_reg_d(self);
        break;
    case e:
        arg->payload = get_reg_e(self);
        break;
    case h:
        arg->payload = get_reg_h(self);
        break;
    case l:
        arg->payload = get_reg_l(self);
        break;
    case paf:
        arg->payload = bus_read(self->bus, self->af.u16);
        break;
    case pbc:
        arg->payload = bus_read(self->bus, self->bc.u16);
        break;
    case pde:
        arg->payload = bus_read(self->bus, self->de.u16);
        break;
    case phl:
        arg->payload = bus_read(self->bus, self->hl.u16);
        break;
    case phld:
        arg->payload = bus_read(self->bus, self->hl.u16--);
        break;
    case phli:
        arg->payload = bus_read(self->bus, self->hl.u16++);
        break;
    case af:
        arg->payload = get_reg_af(self);
        break;
    case bc:
        arg->payload = get_reg_bc(self);
        break;
    case de:
        arg->payload = get_reg_de(self);
        break;
    case hl:
        arg->payload = get_reg_hl(self);
        break;
    case sp:
        arg->payload = get_sp(self);
        break;
    case pc:
        arg->payload = get_pc(self);
        break;
    case sp_offset:
        arg->payload += get_sp(self);
        break;
    case imm_u8:
        arg->payload = cpu_get_imm_u8(self);
        break;
    case imm_i8:
        arg->payload = (int8_t)cpu_get_imm_u8(self);
        break;
    case imm_u16:
        arg->payload = cpu_get_imm_u16(self);
        break;
    case p:
        arg->payload = cpu_get_imm_u16(self);
        break;
    }
}

void handle_cb(cpu *self);

cpu cpu_new(bus *bus) {
    cpu c;
    c.af.u16 = 0x0000;
    c.bc.u16 = 0x0000;
    c.de.u16 = 0x0000;
    c.hl.u16 = 0x0000;
    c.pc = 0x0000;
    c.sp = 0xFFFE;
    c.mode = cpu_running_mode_e;
    c.bus = bus;
    c.clocks = 0x0000;
    return c;
}

uint8_t next_instruction(cpu *self) {
    uint8_t v;
    v = bus_read(self->bus, self->pc++);
#ifndef DEBUG
    self->clocks++;
#endif
    return v;
}

uint8_t cpu_get_imm_u8(cpu *self) { return next_instruction(self); }

uint16_t cpu_get_imm_u16(cpu *self) {
    uint8_t b1 = next_instruction(self);
    uint8_t b2 = next_instruction(self);
    return (uint16_t)((b2 << 8) | b1);
}

void cpu_write_bus(cpu *self, uint16_t addr, uint8_t n) {
    bus_write(self->bus, addr, n);
#ifndef DEBUG
    self->clocks++;
#endif
}

uint8_t cpu_read_bus(cpu *self, uint16_t addr) {
#ifndef DEBUG
    self->clocks++;
#endif
    return bus_read(self->bus, addr);
}

void noop(cpu *self, argument_t lhs, argument_t rhs) {
    (void)self;
    (void)lhs;
    (void)rhs;
}

/* Caller is required to set lhs.type and rhs.payload */
void ld(cpu *self, argument_t lhs, argument_t rhs) {
#ifdef DEBUG
    set_arg_payload(self, &lhs);
    set_arg_payload(self, &rhs);
#endif
    set_reg(self, lhs, rhs.payload);
}

void inc(cpu *self, argument_t lhs, argument_t rhs) {
    uint16_t res;
#ifdef DEBUG
    set_arg_payload(self, &lhs);
#endif
    res = lhs.payload + 1;
    (void)rhs;
    set_reg(self, lhs, res);
    switch (lhs.type) {
    case bc:
    case de:
    case hl:
    case sp:
#ifndef DEBUG
        self->clocks++;
#endif
        break;
    default:
        set_flag_z(self, res == 0);
        set_flag_n(self, 0);
        set_flag_h_add(self, lhs.payload, 1);
        break;
    }
}

void dec(cpu *self, argument_t lhs, argument_t rhs) {
    uint16_t res;
#ifdef DEBUG
    set_arg_payload(self, &lhs);
#endif
    res = lhs.payload - 1;
    (void)rhs;
    set_reg(self, lhs, res);
    switch (lhs.type) {
    case bc:
    case de:
    case hl:
    case sp:
#ifndef DEBUG
        self->clocks++;
#endif
        break;
    default:
        set_flag_z(self, res == 0);
        set_flag_n(self, 0);
        set_flag_h_add(self, lhs.payload, 1);
        break;
    }
}

void add(cpu *self, argument_t lhs, argument_t rhs) {
    uint16_t res;
#ifdef DEBUG
    set_arg_payload(self, &lhs);
    set_arg_payload(self, &rhs);
#endif
    res = lhs.payload + rhs.payload;
    set_reg(self, lhs, res);
    switch (lhs.type) {
    case a:
        set_flag_z(self, res == 0);
        break;
    case sp:
        set_flag_z(self, 0);
        break;
    case hl:
#ifndef DEBUG
        self->clocks++;
#endif
        break;
    default:
        break;
    }
    set_flag_n(self, 0);
    set_flag_h_add(self, lhs.payload, rhs.payload);
    set_flag_c_add(self, lhs.payload, rhs.payload);
}

void adc(cpu *self, argument_t lhs, argument_t rhs) {
    uint16_t res;
    res = lhs.payload + rhs.payload + get_flag_c(self);
    set_reg(self, lhs, res);
    set_flag_z(self, res == 0);
    set_flag_n(self, 0);
    set_flag_h_add(self, lhs.payload, rhs.payload + get_flag_c(self));
    set_flag_c_add(self, lhs.payload, rhs.payload + get_flag_c(self));
}

void sub(cpu *self, argument_t lhs, argument_t rhs) {
    uint16_t res;
    res = lhs.payload - rhs.payload;
    set_reg(self, lhs, res);
    set_flag_z(self, res == 0);
    set_flag_n(self, 1);
    /*
     * TODO:
     * I don't know if SET_FLAG_{H,C} properly handle subtraction
     */
    set_flag_h_add(self, lhs.payload, rhs.payload);
    set_flag_c_add(self, lhs.payload, rhs.payload);
}

void sbc(cpu *self, argument_t lhs, argument_t rhs) {
    uint16_t res;
    res = lhs.payload - rhs.payload - get_flag_c(self);
    set_reg(self, lhs, res);
    set_flag_z(self, res == 0);
    set_flag_n(self, 1);
    set_flag_h_add(self, lhs.payload, rhs.payload - get_flag_c(self));
    set_flag_c_add(self, lhs.payload, rhs.payload - get_flag_c(self));
}

void andReg(cpu *self, argument_t lhs, argument_t rhs) {
    uint16_t res;
    res = lhs.payload & rhs.payload;
    set_reg(self, lhs, res);
    set_flag_z(self, res == 0);
    set_flag_n(self, 0);
    set_flag_h(self, 1);
    set_flag_c(self, 0);
}

void xorReg(cpu *self, argument_t lhs, argument_t rhs) {
    uint16_t res;
    res = lhs.payload ^ rhs.payload;
    set_reg(self, lhs, res);
    set_flag_z(self, res == 0);
    set_flag_n(self, 0);
    set_flag_h(self, 0);
    set_flag_c(self, 0);
}

void orReg(cpu *self, argument_t lhs, argument_t rhs) {
    uint16_t res;
    res = lhs.payload | rhs.payload;
    set_reg(self, lhs, res);
    set_flag_z(self, res == 0);
    set_flag_n(self, 0);
    set_flag_h(self, 0);
    set_flag_c(self, 0);
}

void cp(cpu *self, argument_t lhs, argument_t rhs) {
    uint16_t res;
    res = lhs.payload - rhs.payload;
    set_flag_z(self, res == 0);
    set_flag_n(self, 1);
    set_flag_h_add(self, lhs.payload, rhs.payload);
    set_flag_c_add(self, lhs.payload, rhs.payload);
}

void ret(cpu *self, argument_t lhs, argument_t rhs) {
    (void)rhs;
    if (lhs.cond) {
        set_pc(self, (bus_read(self->bus, self->sp--) << 8) | (bus_read(self->bus, self->sp--)));
    }
}

void jp(cpu *self, argument_t lhs, argument_t rhs) {
    (void)rhs;
    if (lhs.cond) {
        set_pc(self, lhs.payload);
    }
}

void rlc(cpu *self, argument_t lhs, argument_t rhs) {
    uint16_t res;
    (void)rhs;
    res = (lhs.payload << 1) | (lhs.payload >> ((sizeof(lhs.payload) << 3) - 1));
    set_reg(self, lhs, res);
    set_flag_z(self, res == 0);
    set_flag_n(self, false);
    set_flag_h(self, false);
    set_flag_c(self, lhs.payload >> 7);
}

void rrc(cpu *self, argument_t lhs, argument_t rhs) {
    uint16_t res;
    (void)rhs;
    res = (lhs.payload >> 1) | (lhs.payload << ((sizeof(lhs.payload) << 3) - 1));
    set_reg(self, lhs, res);
    set_flag_z(self, res == 0);
    set_flag_n(self, false);
    set_flag_h(self, false);
    set_flag_c(self, lhs.payload & 0x01);
}

void rl(cpu *self, argument_t lhs, argument_t rhs) {
    uint16_t res;
    (void)rhs;
    res = (lhs.payload << 1) | (get_flag_c(self));
    set_reg(self, lhs, res);
    set_flag_z(self, res == 0);
    set_flag_n(self, false);
    set_flag_h(self, false);
    set_flag_c(self, lhs.payload >> 7);
}

void rr(cpu *self, argument_t lhs, argument_t rhs) {
    uint16_t res;
    (void)rhs;
    res = (get_flag_c(self)) | (lhs.payload >> 1);
    set_reg(self, lhs, res);
    set_flag_z(self, res == 0);
    set_flag_n(self, false);
    set_flag_h(self, false);
    set_flag_c(self, lhs.payload & 0x01);
}

void sla(cpu *self, argument_t lhs, argument_t rhs) {
    uint16_t res;
    (void)rhs;
    res = (lhs.payload << 1);
    set_reg(self, lhs, res);
    set_flag_z(self, res == 0);
    set_flag_n(self, false);
    set_flag_h(self, false);
    set_flag_c(self, lhs.payload >> 7);
}

void sra(cpu *self, argument_t lhs, argument_t rhs) {
    uint16_t res;
    (void)rhs;
    res = (lhs.payload & 0x80) | (lhs.payload >> 1);
    set_reg(self, lhs, res);
    set_flag_z(self, res == 0);
    set_flag_n(self, false);
    set_flag_h(self, false);
    set_flag_c(self, lhs.payload & 0x01);
}

void bit(cpu *self, argument_t lhs, argument_t rhs) {
    set_flag_z(self, ~((lhs.payload >> rhs.payload) & 0x01));
    set_flag_n(self, false);
    set_flag_h(self, false);
}

void res(cpu *self, argument_t lhs, argument_t rhs) {
    set_reg(self, lhs, lhs.payload | ~(1 << rhs.payload));
}

void set(cpu *self, argument_t lhs, argument_t rhs) {
    set_reg(self, lhs, lhs.payload | (1 << rhs.payload));
}

void swap(cpu *self, argument_t lhs, argument_t rhs) {
    uint16_t res;
    (void)rhs;
    res = (lhs.payload & 0x0F) | (lhs.payload >> 4);
    set_reg(self, lhs, res);
    set_flag_z(self, res == 0);
    set_flag_n(self, false);
    set_flag_h(self, false);
    set_flag_c(self, false);
}

void srl(cpu *self, argument_t lhs, argument_t rhs) {
    uint16_t res;
    (void)rhs;
    res = (lhs.payload >> 1);
    set_reg(self, lhs, res);
    set_flag_z(self, res == 0);
    set_flag_n(self, false);
    set_flag_h(self, false);
    set_flag_c(self, lhs.payload & 0x01);
}

void jr(cpu *self, argument_t lhs, argument_t rhs) {
    int8_t offset;
    (void)rhs;
    resolve_cond(self, &lhs);
    offset = (int8_t)cpu_get_imm_u8(self);
    if (lhs.should_branch) {
        set_pc(self, get_pc(self) + offset);
    }
}

void push(cpu *self, argument_t lhs, argument_t rhs) {
    (void)self;
    (void)lhs;
    (void)rhs;
    UNIMPLEMENTED("push");
}

void pop(cpu *self, argument_t lhs, argument_t rhs) {
    (void)self;
    (void)lhs;
    (void)rhs;
    UNIMPLEMENTED("pop");
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
    (void)self;
    (void)lhs;
    (void)rhs;
    UNIMPLEMENTED("halt");
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
    (void)self;
    (void)lhs;
    (void)rhs;
    UNIMPLEMENTED("call");
}

void ccf(cpu *self, argument_t lhs, argument_t rhs) {
    (void)self;
    (void)lhs;
    (void)rhs;
    UNIMPLEMENTED("ccf");
}

void cpl(cpu *self, argument_t lhs, argument_t rhs) {
    (void)self;
    (void)lhs;
    (void)rhs;
    UNIMPLEMENTED("cpl");
}

void stop(cpu *self, argument_t lhs, argument_t rhs) {
    (void)lhs;
    (void)rhs;
    self->mode = cpu_stop_mode_e;
}

void daa(cpu *self, argument_t lhs, argument_t rhs) {
    (void)self;
    (void)lhs;
    (void)rhs;
    UNIMPLEMENTED("daa");
}

void scf(cpu *self, argument_t lhs, argument_t rhs) {
    (void)self;
    (void)lhs;
    (void)rhs;
    UNIMPLEMENTED("scf");
}

uintptr_t cpu_clock(cpu *self) {
    uint8_t opcode;
    instr instr;
    uintptr_t old_clocks;
    if (self->mode != cpu_running_mode_e)
        return 0;
    opcode = next_instruction(self);
    old_clocks = self->clocks;
    if (self->sp == 0x100) {
        PANIC("finished boot");
    }
    instr = OPCODE_TABLE[opcode];

    LOG("CPU", "CPU Instr: %#x", opcode);
    switch (instr.instr) {
    case adc_instr:
        adc(self, instr.lhs, instr.rhs);
        break;
    case add_instr:
        add(self, instr.lhs, instr.rhs);
        break;
    case and_instr:
        andReg(self, instr.lhs, instr.rhs);
        break;
    case bit_instr:
        bit(self, instr.lhs, instr.rhs);
        break;
    case call_instr:
        call(self, instr.lhs, instr.rhs);
        break;
    case cb_instr:
        handle_cb(self);
        break;
    case ccf_instr:
        ccf(self, instr.lhs, instr.rhs);
        break;
    case cp_instr:
        cp(self, instr.lhs, instr.rhs);
        break;
    case cpl_instr:
        cpl(self, instr.lhs, instr.rhs);
        break;
    case daa_instr:
        daa(self, instr.lhs, instr.rhs);
        break;
    case dec_instr:
        dec(self, instr.lhs, instr.rhs);
        break;
    case di_instr:
        di(self, instr.lhs, instr.rhs);
        break;
    case ei_instr:
        ei(self, instr.lhs, instr.rhs);
        break;
    case halt_instr:
        halt(self, instr.lhs, instr.rhs);
        break;
    case inc_instr:
        inc(self, instr.lhs, instr.rhs);
        break;
    case jp_instr:
        jp(self, instr.lhs, instr.rhs);
        break;
    case jr_instr:
        jr(self, instr.lhs, instr.rhs);
        break;
    case ld_instr:
        ld(self, instr.lhs, instr.rhs);
        break;
    case noop_instr:
        noop(self, instr.lhs, instr.rhs);
        break;
    case or_instr:
        orReg(self, instr.lhs, instr.rhs);
        break;
    case pop_instr:
        pop(self, instr.lhs, instr.rhs);
        break;
    case push_instr:
        push(self, instr.lhs, instr.rhs);
        break;
    case res_instr:
        res(self, instr.lhs, instr.rhs);
        break;
    case ret_instr:
        ret(self, instr.lhs, instr.rhs);
        break;
    case reti_instr:
        reti(self, instr.lhs, instr.rhs);
        break;
    case rl_instr:
        rl(self, instr.lhs, instr.rhs);
        break;
    case rla_instr:
        rla(self, instr.lhs, instr.rhs);
        break;
    case rlca_instr:
        rlca(self, instr.lhs, instr.rhs);
        break;
    case rr_instr:
        rr(self, instr.lhs, instr.rhs);
        break;
    case rra_instr:
        rra(self, instr.lhs, instr.rhs);
        break;
    case rrca_instr:
        rrca(self, instr.lhs, instr.rhs);
        break;
    case rst_instr:
        rst(self, instr.lhs, instr.rhs);
        break;
    case sbc_instr:
        sbc(self, instr.lhs, instr.rhs);
        break;
    case scf_instr:
        scf(self, instr.lhs, instr.rhs);
        break;
    case set_instr:
        set(self, instr.lhs, instr.rhs);
        break;
    case sla_instr:
        sla(self, instr.lhs, instr.rhs);
        break;
    case sra_instr:
        sra(self, instr.lhs, instr.rhs);
        break;
    case srl_instr:
        srl(self, instr.lhs, instr.rhs);
        break;
    case stop_instr:
        stop(self, instr.lhs, instr.rhs);
        break;
    case sub_instr:
        sub(self, instr.lhs, instr.rhs);
        break;
    case swap_instr:
        swap(self, instr.lhs, instr.rhs);
        break;
    case xor_instr:
        xorReg(self, instr.lhs, instr.rhs);
        break;
    case illegal_instr:
        PANIC("Illegal instruction encountered");
        break;
    }
    self->clocks += instr.clocks;

    return self->clocks - old_clocks;
}

void handle_cb(cpu *self) {
    uint8_t opcode;
    argument_t lhs;
    argument_t rhs;

    opcode = next_instruction(self);
    switch (opcode) {
    /* RLC */
    case 0x00:
        lhs.type = b;
        set_arg_payload(self, &lhs);
        ignore_arg(&rhs);
        rlc(self, lhs, rhs);
        break;
    case 0x01:
        lhs.type = c;
        set_arg_payload(self, &lhs);
        ignore_arg(&rhs);
        rlc(self, lhs, rhs);
        break;
    case 0x02:
        lhs.type = d;
        set_arg_payload(self, &lhs);
        ignore_arg(&rhs);
        rlc(self, lhs, rhs);
        break;
    case 0x03:
        lhs.type = e;
        set_arg_payload(self, &lhs);
        ignore_arg(&rhs);
        rlc(self, lhs, rhs);
        break;
    case 0x04:
        lhs.type = h;
        set_arg_payload(self, &lhs);
        ignore_arg(&rhs);
        rlc(self, lhs, rhs);
        break;
    case 0x05:
        lhs.type = l;
        set_arg_payload(self, &lhs);
        ignore_arg(&rhs);
        rlc(self, lhs, rhs);
        break;
    case 0x06:
        lhs.type = phl;
        set_arg_payload(self, &lhs);
        ignore_arg(&rhs);
        rlc(self, lhs, rhs);
        break;
    case 0x07:
        lhs.type = a;
        set_arg_payload(self, &lhs);
        ignore_arg(&rhs);
        rlc(self, lhs, rhs);
        break;
    /* RRC */
    case 0x08:
        lhs.type = b;
        set_arg_payload(self, &lhs);
        ignore_arg(&rhs);
        rrc(self, lhs, rhs);
        break;
    case 0x09:
        lhs.type = c;
        set_arg_payload(self, &lhs);
        ignore_arg(&rhs);
        rrc(self, lhs, rhs);
        break;
    case 0x0A:
        lhs.type = d;
        set_arg_payload(self, &lhs);
        ignore_arg(&rhs);
        rrc(self, lhs, rhs);
        break;
    case 0x0B:
        lhs.type = e;
        set_arg_payload(self, &lhs);
        ignore_arg(&rhs);
        rrc(self, lhs, rhs);
        break;
    case 0x0C:
        lhs.type = h;
        set_arg_payload(self, &lhs);
        ignore_arg(&rhs);
        rrc(self, lhs, rhs);
        break;
    case 0x0D:
        lhs.type = l;
        set_arg_payload(self, &lhs);
        ignore_arg(&rhs);
        rrc(self, lhs, rhs);
        break;
    case 0x0E:
        lhs.type = phl;
        set_arg_payload(self, &lhs);
        ignore_arg(&rhs);
        rrc(self, lhs, rhs);
        break;
    case 0x0F:
        lhs.type = a;
        set_arg_payload(self, &lhs);
        ignore_arg(&rhs);
        rrc(self, lhs, rhs);
        break;
    /* RL */
    case 0x10:
        lhs.type = b;
        set_arg_payload(self, &lhs);
        ignore_arg(&rhs);
        rl(self, lhs, rhs);
        break;
    case 0x11:
        lhs.type = c;
        set_arg_payload(self, &lhs);
        ignore_arg(&rhs);
        rl(self, lhs, rhs);
        break;
    case 0x12:
        lhs.type = d;
        set_arg_payload(self, &lhs);
        ignore_arg(&rhs);
        rl(self, lhs, rhs);
        break;
    case 0x13:
        lhs.type = e;
        set_arg_payload(self, &lhs);
        ignore_arg(&rhs);
        rl(self, lhs, rhs);
        break;
    case 0x14:
        lhs.type = h;
        set_arg_payload(self, &lhs);
        ignore_arg(&rhs);
        rl(self, lhs, rhs);
        break;
    case 0x15:
        lhs.type = l;
        set_arg_payload(self, &lhs);
        ignore_arg(&rhs);
        rl(self, lhs, rhs);
        break;
    case 0x16:
        lhs.type = phl;
        set_arg_payload(self, &lhs);
        ignore_arg(&rhs);
        rl(self, lhs, rhs);
        break;
    case 0x17:
        lhs.type = a;
        set_arg_payload(self, &lhs);
        ignore_arg(&rhs);
        rl(self, lhs, rhs);
        break;
    /* RR */
    case 0x18:
        lhs.type = b;
        set_arg_payload(self, &lhs);
        ignore_arg(&rhs);
        rr(self, lhs, rhs);
        break;
    case 0x19:
        lhs.type = c;
        set_arg_payload(self, &lhs);
        ignore_arg(&rhs);
        rr(self, lhs, rhs);
        break;
    case 0x1A:
        lhs.type = d;
        set_arg_payload(self, &lhs);
        ignore_arg(&rhs);
        rr(self, lhs, rhs);
        break;
    case 0x1B:
        lhs.type = e;
        set_arg_payload(self, &lhs);
        ignore_arg(&rhs);
        rr(self, lhs, rhs);
        break;
    case 0x1C:
        lhs.type = h;
        set_arg_payload(self, &lhs);
        ignore_arg(&rhs);
        rr(self, lhs, rhs);
        break;
    case 0x1D:
        lhs.type = l;
        set_arg_payload(self, &lhs);
        ignore_arg(&rhs);
        rr(self, lhs, rhs);
        break;
    case 0x1E:
        lhs.type = phl;
        set_arg_payload(self, &lhs);
        ignore_arg(&rhs);
        rr(self, lhs, rhs);
        break;
    case 0x1F:
        lhs.type = a;
        set_arg_payload(self, &lhs);
        ignore_arg(&rhs);
        rr(self, lhs, rhs);
        break;
    /* SLA */
    case 0x20:
        lhs.type = b;
        set_arg_payload(self, &lhs);
        ignore_arg(&rhs);
        sla(self, lhs, rhs);
        break;
    case 0x21:
        lhs.type = c;
        set_arg_payload(self, &lhs);
        ignore_arg(&rhs);
        sla(self, lhs, rhs);
        break;
    case 0x22:
        lhs.type = d;
        set_arg_payload(self, &lhs);
        ignore_arg(&rhs);
        sla(self, lhs, rhs);
        break;
    case 0x23:
        lhs.type = e;
        set_arg_payload(self, &lhs);
        ignore_arg(&rhs);
        sla(self, lhs, rhs);
        break;
    case 0x24:
        lhs.type = h;
        set_arg_payload(self, &lhs);
        ignore_arg(&rhs);
        sla(self, lhs, rhs);
        break;
    case 0x25:
        lhs.type = l;
        set_arg_payload(self, &lhs);
        ignore_arg(&rhs);
        sla(self, lhs, rhs);
        break;
    case 0x26:
        lhs.type = phl;
        set_arg_payload(self, &lhs);
        ignore_arg(&rhs);
        sla(self, lhs, rhs);
        break;
    case 0x27:
        lhs.type = a;
        set_arg_payload(self, &lhs);
        ignore_arg(&rhs);
        sla(self, lhs, rhs);
        break;
    /* SRA */
    case 0x28:
        lhs.type = b;
        set_arg_payload(self, &lhs);
        ignore_arg(&rhs);
        sra(self, lhs, rhs);
        break;
    case 0x29:
        lhs.type = c;
        set_arg_payload(self, &lhs);
        ignore_arg(&rhs);
        sra(self, lhs, rhs);
        break;
    case 0x2A:
        lhs.type = d;
        set_arg_payload(self, &lhs);
        ignore_arg(&rhs);
        sra(self, lhs, rhs);
        break;
    case 0x2B:
        lhs.type = e;
        set_arg_payload(self, &lhs);
        ignore_arg(&rhs);
        sra(self, lhs, rhs);
        break;
    case 0x2C:
        lhs.type = h;
        set_arg_payload(self, &lhs);
        ignore_arg(&rhs);
        sra(self, lhs, rhs);
        break;
    case 0x2D:
        lhs.type = l;
        set_arg_payload(self, &lhs);
        ignore_arg(&rhs);
        sra(self, lhs, rhs);
        break;
    case 0x2E:
        lhs.type = phl;
        set_arg_payload(self, &lhs);
        ignore_arg(&rhs);
        sra(self, lhs, rhs);
        break;
    case 0x2F:
        lhs.type = a;
        set_arg_payload(self, &lhs);
        ignore_arg(&rhs);
        sra(self, lhs, rhs);
        break;
    /* SWAP */
    case 0x30:
        lhs.type = b;
        set_arg_payload(self, &lhs);
        ignore_arg(&rhs);
        swap(self, lhs, rhs);
        break;
    case 0x31:
        lhs.type = c;
        set_arg_payload(self, &lhs);
        ignore_arg(&rhs);
        swap(self, lhs, rhs);
        break;
    case 0x32:
        lhs.type = d;
        set_arg_payload(self, &lhs);
        ignore_arg(&rhs);
        swap(self, lhs, rhs);
        break;
    case 0x33:
        lhs.type = e;
        set_arg_payload(self, &lhs);
        ignore_arg(&rhs);
        swap(self, lhs, rhs);
        break;
    case 0x34:
        lhs.type = h;
        set_arg_payload(self, &lhs);
        ignore_arg(&rhs);
        swap(self, lhs, rhs);
        break;
    case 0x35:
        lhs.type = l;
        set_arg_payload(self, &lhs);
        ignore_arg(&rhs);
        swap(self, lhs, rhs);
        break;
    case 0x36:
        lhs.type = phl;
        set_arg_payload(self, &lhs);
        ignore_arg(&rhs);
        swap(self, lhs, rhs);
        break;
    case 0x37:
        lhs.type = a;
        set_arg_payload(self, &lhs);
        ignore_arg(&rhs);
        swap(self, lhs, rhs);
        break;
    /* SRL */
    case 0x38:
        lhs.type = b;
        set_arg_payload(self, &lhs);
        ignore_arg(&rhs);
        srl(self, lhs, rhs);
        break;
    case 0x39:
        lhs.type = c;
        set_arg_payload(self, &lhs);
        ignore_arg(&rhs);
        srl(self, lhs, rhs);
        break;
    case 0x3A:
        lhs.type = d;
        set_arg_payload(self, &lhs);
        ignore_arg(&rhs);
        srl(self, lhs, rhs);
        break;
    case 0x3B:
        lhs.type = e;
        set_arg_payload(self, &lhs);
        ignore_arg(&rhs);
        srl(self, lhs, rhs);
        break;
    case 0x3C:
        lhs.type = h;
        set_arg_payload(self, &lhs);
        ignore_arg(&rhs);
        srl(self, lhs, rhs);
        break;
    case 0x3D:
        lhs.type = l;
        set_arg_payload(self, &lhs);
        ignore_arg(&rhs);
        srl(self, lhs, rhs);
        break;
    case 0x3E:
        lhs.type = phl;
        set_arg_payload(self, &lhs);
        ignore_arg(&rhs);
        srl(self, lhs, rhs);
        break;
    case 0x3F:
        lhs.type = a;
        set_arg_payload(self, &lhs);
        ignore_arg(&rhs);
        srl(self, lhs, rhs);
        break;
    /* BIT */
    case 0x40:
        lhs.type = b;
        set_arg_payload(self, &lhs);
        rhs.payload = 0;
        bit(self, lhs, rhs);
        break;
    case 0x41:
        lhs.type = c;
        set_arg_payload(self, &lhs);
        rhs.payload = 0;
        bit(self, lhs, rhs);
        break;
    case 0x42:
        lhs.type = d;
        set_arg_payload(self, &lhs);
        rhs.payload = 0;
        bit(self, lhs, rhs);
        break;
    case 0x43:
        lhs.type = e;
        set_arg_payload(self, &lhs);
        rhs.payload = 0;
        bit(self, lhs, rhs);
        break;
    case 0x44:
        lhs.type = h;
        set_arg_payload(self, &lhs);
        rhs.payload = 0;
        bit(self, lhs, rhs);
        break;
    case 0x45:
        lhs.type = l;
        set_arg_payload(self, &lhs);
        rhs.payload = 0;
        bit(self, lhs, rhs);
        break;
    case 0x46:
        lhs.type = phl;
        set_arg_payload(self, &lhs);
        rhs.payload = 0;
        bit(self, lhs, rhs);
        break;
    case 0x47:
        lhs.type = a;
        set_arg_payload(self, &lhs);
        rhs.payload = 0;
        bit(self, lhs, rhs);
        break;
    case 0x48:
        lhs.type = b;
        set_arg_payload(self, &lhs);
        rhs.payload = 1;
        bit(self, lhs, rhs);
        break;
    case 0x49:
        lhs.type = c;
        set_arg_payload(self, &lhs);
        rhs.payload = 1;
        bit(self, lhs, rhs);
        break;
    case 0x4A:
        lhs.type = d;
        set_arg_payload(self, &lhs);
        rhs.payload = 1;
        bit(self, lhs, rhs);
        break;
    case 0x4B:
        lhs.type = e;
        set_arg_payload(self, &lhs);
        rhs.payload = 1;
        bit(self, lhs, rhs);
        break;
    case 0x4C:
        lhs.type = h;
        set_arg_payload(self, &lhs);
        rhs.payload = 1;
        bit(self, lhs, rhs);
        break;
    case 0x4D:
        lhs.type = l;
        set_arg_payload(self, &lhs);
        rhs.payload = 1;
        bit(self, lhs, rhs);
        break;
    case 0x4E:
        lhs.type = phl;
        set_arg_payload(self, &lhs);
        rhs.payload = 1;
        bit(self, lhs, rhs);
        break;
    case 0x4F:
        lhs.type = a;
        set_arg_payload(self, &lhs);
        rhs.payload = 1;
        bit(self, lhs, rhs);
        break;
    case 0x50:
        lhs.type = b;
        set_arg_payload(self, &lhs);
        rhs.payload = 2;
        bit(self, lhs, rhs);
        break;
    case 0x51:
        lhs.type = c;
        set_arg_payload(self, &lhs);
        rhs.payload = 2;
        bit(self, lhs, rhs);
        break;
    case 0x52:
        lhs.type = d;
        set_arg_payload(self, &lhs);
        rhs.payload = 2;
        bit(self, lhs, rhs);
        break;
    case 0x53:
        lhs.type = e;
        set_arg_payload(self, &lhs);
        rhs.payload = 2;
        bit(self, lhs, rhs);
        break;
    case 0x54:
        lhs.type = h;
        set_arg_payload(self, &lhs);
        rhs.payload = 2;
        bit(self, lhs, rhs);
        break;
    case 0x55:
        lhs.type = l;
        set_arg_payload(self, &lhs);
        rhs.payload = 2;
        bit(self, lhs, rhs);
        break;
    case 0x56:
        lhs.type = phl;
        set_arg_payload(self, &lhs);
        rhs.payload = 2;
        bit(self, lhs, rhs);
        break;
    case 0x57:
        lhs.type = a;
        set_arg_payload(self, &lhs);
        rhs.payload = 2;
        bit(self, lhs, rhs);
        break;
    case 0x58:
        lhs.type = b;
        set_arg_payload(self, &lhs);
        rhs.payload = 3;
        bit(self, lhs, rhs);
        break;
    case 0x59:
        lhs.type = c;
        set_arg_payload(self, &lhs);
        rhs.payload = 3;
        bit(self, lhs, rhs);
        break;
    case 0x5A:
        lhs.type = d;
        set_arg_payload(self, &lhs);
        rhs.payload = 3;
        bit(self, lhs, rhs);
        break;
    case 0x5B:
        lhs.type = e;
        set_arg_payload(self, &lhs);
        rhs.payload = 3;
        bit(self, lhs, rhs);
        break;
    case 0x5C:
        lhs.type = h;
        set_arg_payload(self, &lhs);
        rhs.payload = 3;
        bit(self, lhs, rhs);
        break;
    case 0x5D:
        lhs.type = l;
        set_arg_payload(self, &lhs);
        rhs.payload = 3;
        bit(self, lhs, rhs);
        break;
    case 0x5E:
        lhs.type = phl;
        set_arg_payload(self, &lhs);
        rhs.payload = 3;
        bit(self, lhs, rhs);
        break;
    case 0x5F:
        lhs.type = a;
        set_arg_payload(self, &lhs);
        rhs.payload = 3;
        bit(self, lhs, rhs);
        break;
    case 0x60:
        lhs.type = b;
        set_arg_payload(self, &lhs);
        rhs.payload = 4;
        bit(self, lhs, rhs);
        break;
    case 0x61:
        lhs.type = c;
        set_arg_payload(self, &lhs);
        rhs.payload = 4;
        bit(self, lhs, rhs);
        break;
    case 0x62:
        lhs.type = d;
        set_arg_payload(self, &lhs);
        rhs.payload = 4;
        bit(self, lhs, rhs);
        break;
    case 0x63:
        lhs.type = e;
        set_arg_payload(self, &lhs);
        rhs.payload = 4;
        bit(self, lhs, rhs);
        break;
    case 0x64:
        lhs.type = h;
        set_arg_payload(self, &lhs);
        rhs.payload = 4;
        bit(self, lhs, rhs);
        break;
    case 0x65:
        lhs.type = l;
        set_arg_payload(self, &lhs);
        rhs.payload = 4;
        bit(self, lhs, rhs);
        break;
    case 0x66:
        lhs.type = phl;
        set_arg_payload(self, &lhs);
        rhs.payload = 4;
        bit(self, lhs, rhs);
        break;
    case 0x67:
        lhs.type = a;
        set_arg_payload(self, &lhs);
        rhs.payload = 4;
        bit(self, lhs, rhs);
        break;
    case 0x68:
        lhs.type = b;
        set_arg_payload(self, &lhs);
        rhs.payload = 5;
        bit(self, lhs, rhs);
        break;
    case 0x69:
        lhs.type = c;
        set_arg_payload(self, &lhs);
        rhs.payload = 5;
        bit(self, lhs, rhs);
        break;
    case 0x6A:
        lhs.type = d;
        set_arg_payload(self, &lhs);
        rhs.payload = 5;
        bit(self, lhs, rhs);
        break;
    case 0x6B:
        lhs.type = e;
        set_arg_payload(self, &lhs);
        rhs.payload = 5;
        bit(self, lhs, rhs);
        break;
    case 0x6C:
        lhs.type = h;
        set_arg_payload(self, &lhs);
        rhs.payload = 5;
        bit(self, lhs, rhs);
        break;
    case 0x6D:
        lhs.type = l;
        set_arg_payload(self, &lhs);
        rhs.payload = 5;
        bit(self, lhs, rhs);
        break;
    case 0x6E:
        lhs.type = phl;
        set_arg_payload(self, &lhs);
        rhs.payload = 5;
        bit(self, lhs, rhs);
        break;
    case 0x6F:
        lhs.type = a;
        set_arg_payload(self, &lhs);
        rhs.payload = 5;
        bit(self, lhs, rhs);
        break;
    case 0x70:
        lhs.type = b;
        set_arg_payload(self, &lhs);
        rhs.payload = 6;
        bit(self, lhs, rhs);
        break;
    case 0x71:
        lhs.type = c;
        set_arg_payload(self, &lhs);
        rhs.payload = 6;
        bit(self, lhs, rhs);
        break;
    case 0x72:
        lhs.type = d;
        set_arg_payload(self, &lhs);
        rhs.payload = 6;
        bit(self, lhs, rhs);
        break;
    case 0x73:
        lhs.type = e;
        set_arg_payload(self, &lhs);
        rhs.payload = 6;
        bit(self, lhs, rhs);
        break;
    case 0x74:
        lhs.type = h;
        set_arg_payload(self, &lhs);
        rhs.payload = 6;
        bit(self, lhs, rhs);
        break;
    case 0x75:
        lhs.type = l;
        set_arg_payload(self, &lhs);
        rhs.payload = 6;
        bit(self, lhs, rhs);
        break;
    case 0x76:
        lhs.type = phl;
        set_arg_payload(self, &lhs);
        rhs.payload = 6;
        bit(self, lhs, rhs);
        break;
    case 0x77:
        lhs.type = a;
        set_arg_payload(self, &lhs);
        rhs.payload = 6;
        bit(self, lhs, rhs);
        break;
    case 0x78:
        lhs.type = b;
        set_arg_payload(self, &lhs);
        rhs.payload = 7;
        bit(self, lhs, rhs);
        break;
    case 0x79:
        lhs.type = c;
        set_arg_payload(self, &lhs);
        rhs.payload = 7;
        bit(self, lhs, rhs);
        break;
    case 0x7A:
        lhs.type = d;
        set_arg_payload(self, &lhs);
        rhs.payload = 7;
        bit(self, lhs, rhs);
        break;
    case 0x7B:
        lhs.type = e;
        set_arg_payload(self, &lhs);
        rhs.payload = 7;
        bit(self, lhs, rhs);
        break;
    case 0x7C:
        lhs.type = h;
        set_arg_payload(self, &lhs);
        rhs.payload = 7;
        bit(self, lhs, rhs);
        break;
    case 0x7D:
        lhs.type = l;
        set_arg_payload(self, &lhs);
        rhs.payload = 7;
        bit(self, lhs, rhs);
        break;
    case 0x7E:
        lhs.type = phl;
        set_arg_payload(self, &lhs);
        rhs.payload = 7;
        bit(self, lhs, rhs);
        break;
    case 0x7F:
        lhs.type = a;
        set_arg_payload(self, &lhs);
        rhs.payload = 7;
        bit(self, lhs, rhs);
        break;
    /* RES */
    case 0x80:
        lhs.type = b;
        set_arg_payload(self, &lhs);
        rhs.payload = 0;
        res(self, lhs, rhs);
        break;
    case 0x81:
        lhs.type = c;
        set_arg_payload(self, &lhs);
        rhs.payload = 0;
        res(self, lhs, rhs);
        break;
    case 0x82:
        lhs.type = d;
        set_arg_payload(self, &lhs);
        rhs.payload = 0;
        res(self, lhs, rhs);
        break;
    case 0x83:
        lhs.type = e;
        set_arg_payload(self, &lhs);
        rhs.payload = 0;
        res(self, lhs, rhs);
        break;
    case 0x84:
        lhs.type = h;
        set_arg_payload(self, &lhs);
        rhs.payload = 0;
        res(self, lhs, rhs);
        break;
    case 0x85:
        lhs.type = l;
        set_arg_payload(self, &lhs);
        rhs.payload = 0;
        res(self, lhs, rhs);
        break;
    case 0x86:
        lhs.type = phl;
        set_arg_payload(self, &lhs);
        rhs.payload = 0;
        res(self, lhs, rhs);
        break;
    case 0x87:
        lhs.type = a;
        set_arg_payload(self, &lhs);
        rhs.payload = 0;
        res(self, lhs, rhs);
        break;
    case 0x88:
        lhs.type = b;
        set_arg_payload(self, &lhs);
        rhs.payload = 1;
        res(self, lhs, rhs);
        break;
    case 0x89:
        lhs.type = c;
        set_arg_payload(self, &lhs);
        rhs.payload = 1;
        res(self, lhs, rhs);
        break;
    case 0x8A:
        lhs.type = d;
        set_arg_payload(self, &lhs);
        rhs.payload = 1;
        res(self, lhs, rhs);
        break;
    case 0x8B:
        lhs.type = e;
        set_arg_payload(self, &lhs);
        rhs.payload = 1;
        res(self, lhs, rhs);
        break;
    case 0x8C:
        lhs.type = h;
        set_arg_payload(self, &lhs);
        rhs.payload = 1;
        res(self, lhs, rhs);
        break;
    case 0x8D:
        lhs.type = l;
        set_arg_payload(self, &lhs);
        rhs.payload = 1;
        res(self, lhs, rhs);
        break;
    case 0x8E:
        lhs.type = phl;
        set_arg_payload(self, &lhs);
        rhs.payload = 1;
        res(self, lhs, rhs);
        break;
    case 0x8F:
        lhs.type = a;
        set_arg_payload(self, &lhs);
        rhs.payload = 1;
        res(self, lhs, rhs);
        break;
    case 0x90:
        lhs.type = b;
        set_arg_payload(self, &lhs);
        rhs.payload = 2;
        res(self, lhs, rhs);
        break;
    case 0x91:
        lhs.type = c;
        set_arg_payload(self, &lhs);
        rhs.payload = 2;
        res(self, lhs, rhs);
        break;
    case 0x92:
        lhs.type = d;
        set_arg_payload(self, &lhs);
        rhs.payload = 2;
        res(self, lhs, rhs);
        break;
    case 0x93:
        lhs.type = e;
        set_arg_payload(self, &lhs);
        rhs.payload = 2;
        res(self, lhs, rhs);
        break;
    case 0x94:
        lhs.type = h;
        set_arg_payload(self, &lhs);
        rhs.payload = 2;
        res(self, lhs, rhs);
        break;
    case 0x95:
        lhs.type = l;
        set_arg_payload(self, &lhs);
        rhs.payload = 2;
        res(self, lhs, rhs);
        break;
    case 0x96:
        lhs.type = phl;
        set_arg_payload(self, &lhs);
        rhs.payload = 2;
        res(self, lhs, rhs);
        ;
        break;
    case 0x97:
        lhs.type = a;
        set_arg_payload(self, &lhs);
        rhs.payload = 2;
        res(self, lhs, rhs);
        break;
    case 0x98:
        lhs.type = b;
        set_arg_payload(self, &lhs);
        rhs.payload = 3;
        res(self, lhs, rhs);
        break;
    case 0x99:
        lhs.type = c;
        set_arg_payload(self, &lhs);
        rhs.payload = 3;
        res(self, lhs, rhs);
        break;
    case 0x9A:
        lhs.type = d;
        set_arg_payload(self, &lhs);
        rhs.payload = 3;
        res(self, lhs, rhs);
        break;
    case 0x9B:
        lhs.type = e;
        set_arg_payload(self, &lhs);
        rhs.payload = 3;
        res(self, lhs, rhs);
        break;
    case 0x9C:
        lhs.type = h;
        set_arg_payload(self, &lhs);
        rhs.payload = 3;
        res(self, lhs, rhs);
        break;
    case 0x9D:
        lhs.type = l;
        set_arg_payload(self, &lhs);
        rhs.payload = 3;
        res(self, lhs, rhs);
        break;
    case 0x9E:
        lhs.type = phl;
        set_arg_payload(self, &lhs);
        rhs.payload = 3;
        res(self, lhs, rhs);
        break;
    case 0x9F:
        lhs.type = a;
        set_arg_payload(self, &lhs);
        rhs.payload = 3;
        res(self, lhs, rhs);
        break;
    case 0xA0:
        lhs.type = b;
        set_arg_payload(self, &lhs);
        rhs.payload = 4;
        res(self, lhs, rhs);
        break;
    case 0xA1:
        lhs.type = c;
        set_arg_payload(self, &lhs);
        rhs.payload = 4;
        res(self, lhs, rhs);
        break;
    case 0xA2:
        lhs.type = d;
        set_arg_payload(self, &lhs);
        rhs.payload = 4;
        res(self, lhs, rhs);
        break;
    case 0xA3:
        lhs.type = e;
        set_arg_payload(self, &lhs);
        rhs.payload = 4;
        res(self, lhs, rhs);
        break;
    case 0xA4:
        lhs.type = h;
        set_arg_payload(self, &lhs);
        rhs.payload = 4;
        res(self, lhs, rhs);
        break;
    case 0xA5:
        lhs.type = l;
        set_arg_payload(self, &lhs);
        rhs.payload = 4;
        res(self, lhs, rhs);
        break;
    case 0xA6:
        lhs.type = phl;
        set_arg_payload(self, &lhs);
        rhs.payload = 4;
        res(self, lhs, rhs);
        break;
    case 0xA7:
        lhs.type = a;
        set_arg_payload(self, &lhs);
        rhs.payload = 4;
        res(self, lhs, rhs);
        break;
    case 0xA8:
        lhs.type = b;
        set_arg_payload(self, &lhs);
        rhs.payload = 5;
        res(self, lhs, rhs);
        break;
    case 0xA9:
        lhs.type = c;
        set_arg_payload(self, &lhs);
        rhs.payload = 5;
        res(self, lhs, rhs);
        break;
    case 0xAA:
        lhs.type = d;
        set_arg_payload(self, &lhs);
        rhs.payload = 5;
        res(self, lhs, rhs);
        break;
    case 0xAB:
        lhs.type = e;
        set_arg_payload(self, &lhs);
        rhs.payload = 5;
        res(self, lhs, rhs);
        break;
    case 0xAC:
        lhs.type = h;
        set_arg_payload(self, &lhs);
        rhs.payload = 5;
        res(self, lhs, rhs);
        break;
    case 0xAD:
        lhs.type = l;
        set_arg_payload(self, &lhs);
        rhs.payload = 5;
        res(self, lhs, rhs);
        break;
    case 0xAE:
        lhs.type = phl;
        set_arg_payload(self, &lhs);
        rhs.payload = 5;
        res(self, lhs, rhs);
        break;
    case 0xAF:
        lhs.type = a;
        set_arg_payload(self, &lhs);
        rhs.payload = 5;
        res(self, lhs, rhs);
        break;
    case 0xB0:
        lhs.type = b;
        set_arg_payload(self, &lhs);
        rhs.payload = 6;
        res(self, lhs, rhs);
        break;
    case 0xB1:
        lhs.type = c;
        set_arg_payload(self, &lhs);
        rhs.payload = 6;
        res(self, lhs, rhs);
        break;
    case 0xB2:
        lhs.type = d;
        set_arg_payload(self, &lhs);
        rhs.payload = 6;
        res(self, lhs, rhs);
        break;
    case 0xB3:
        lhs.type = e;
        set_arg_payload(self, &lhs);
        rhs.payload = 6;
        res(self, lhs, rhs);
        break;
    case 0xB4:
        lhs.type = h;
        set_arg_payload(self, &lhs);
        rhs.payload = 6;
        res(self, lhs, rhs);
        break;
    case 0xB5:
        lhs.type = l;
        set_arg_payload(self, &lhs);
        rhs.payload = 6;
        res(self, lhs, rhs);
        break;
    case 0xB6:
        lhs.type = phl;
        set_arg_payload(self, &lhs);
        rhs.payload = 6;
        res(self, lhs, rhs);
        break;
    case 0xB7:
        lhs.type = a;
        set_arg_payload(self, &lhs);
        rhs.payload = 6;
        res(self, lhs, rhs);
        break;
    case 0xB8:
        lhs.type = b;
        set_arg_payload(self, &lhs);
        rhs.payload = 7;
        res(self, lhs, rhs);
        break;
    case 0xB9:
        lhs.type = c;
        set_arg_payload(self, &lhs);
        rhs.payload = 7;
        res(self, lhs, rhs);
        break;
    case 0xBA:
        lhs.type = d;
        set_arg_payload(self, &lhs);
        rhs.payload = 7;
        res(self, lhs, rhs);
        break;
    case 0xBB:
        lhs.type = e;
        set_arg_payload(self, &lhs);
        rhs.payload = 7;
        res(self, lhs, rhs);
        break;
    case 0xBC:
        lhs.type = h;
        set_arg_payload(self, &lhs);
        rhs.payload = 7;
        res(self, lhs, rhs);
        break;
    case 0xBD:
        lhs.type = l;
        set_arg_payload(self, &lhs);
        rhs.payload = 7;
        res(self, lhs, rhs);
        break;
    case 0xBE:
        lhs.type = phl;
        set_arg_payload(self, &lhs);
        rhs.payload = 7;
        res(self, lhs, rhs);
        break;
    case 0xBF:
        lhs.type = a;
        set_arg_payload(self, &lhs);
        rhs.payload = 7;
        res(self, lhs, rhs);
        break;
    /* SET */
    case 0xC0:
        lhs.type = b;
        set_arg_payload(self, &lhs);
        rhs.payload = 0;
        set(self, lhs, rhs);
        break;
    case 0xC1:
        lhs.type = c;
        set_arg_payload(self, &lhs);
        rhs.payload = 0;
        set(self, lhs, rhs);
        break;
    case 0xC2:
        lhs.type = d;
        set_arg_payload(self, &lhs);
        rhs.payload = 0;
        set(self, lhs, rhs);
        break;
    case 0xC3:
        lhs.type = e;
        set_arg_payload(self, &lhs);
        rhs.payload = 0;
        set(self, lhs, rhs);
        break;
    case 0xC4:
        lhs.type = h;
        set_arg_payload(self, &lhs);
        rhs.payload = 0;
        set(self, lhs, rhs);
        break;
    case 0xC5:
        lhs.type = l;
        set_arg_payload(self, &lhs);
        rhs.payload = 0;
        set(self, lhs, rhs);
        break;
    case 0xC6:
        lhs.type = phl;
        set_arg_payload(self, &lhs);
        rhs.payload = 0;
        set(self, lhs, rhs);
        break;
    case 0xC7:
        lhs.type = a;
        set_arg_payload(self, &lhs);
        rhs.payload = 0;
        set(self, lhs, rhs);
        break;
    case 0xC8:
        lhs.type = b;
        set_arg_payload(self, &lhs);
        rhs.payload = 1;
        set(self, lhs, rhs);
        break;
    case 0xC9:
        lhs.type = c;
        set_arg_payload(self, &lhs);
        rhs.payload = 1;
        set(self, lhs, rhs);
        break;
    case 0xCA:
        lhs.type = d;
        set_arg_payload(self, &lhs);
        rhs.payload = 1;
        set(self, lhs, rhs);
        break;
    case 0xCB:
        lhs.type = e;
        set_arg_payload(self, &lhs);
        rhs.payload = 1;
        set(self, lhs, rhs);
        break;
    case 0xCC:
        lhs.type = h;
        set_arg_payload(self, &lhs);
        rhs.payload = 1;
        set(self, lhs, rhs);
        break;
    case 0xCD:
        lhs.type = l;
        set_arg_payload(self, &lhs);
        rhs.payload = 1;
        set(self, lhs, rhs);
        break;
    case 0xCE:
        lhs.type = phl;
        set_arg_payload(self, &lhs);
        rhs.payload = 1;
        set(self, lhs, rhs);
        break;
    case 0xCF:
        lhs.type = a;
        set_arg_payload(self, &lhs);
        rhs.payload = 1;
        set(self, lhs, rhs);
        break;
    case 0xD0:
        lhs.type = b;
        set_arg_payload(self, &lhs);
        rhs.payload = 2;
        set(self, lhs, rhs);
        break;
    case 0xD1:
        lhs.type = c;
        set_arg_payload(self, &lhs);
        rhs.payload = 2;
        set(self, lhs, rhs);
        break;
    case 0xD2:
        lhs.type = d;
        set_arg_payload(self, &lhs);
        rhs.payload = 2;
        set(self, lhs, rhs);
        break;
    case 0xD3:
        lhs.type = e;
        set_arg_payload(self, &lhs);
        rhs.payload = 2;
        set(self, lhs, rhs);
        break;
    case 0xD4:
        lhs.type = h;
        set_arg_payload(self, &lhs);
        rhs.payload = 2;
        set(self, lhs, rhs);
        break;
    case 0xD5:
        lhs.type = l;
        set_arg_payload(self, &lhs);
        rhs.payload = 2;
        set(self, lhs, rhs);
        break;
    case 0xD6:
        lhs.type = phl;
        set_arg_payload(self, &lhs);
        rhs.payload = 2;
        set(self, lhs, rhs);
        break;
    case 0xD7:
        lhs.type = a;
        set_arg_payload(self, &lhs);
        rhs.payload = 2;
        set(self, lhs, rhs);
        break;
    case 0xD8:
        lhs.type = b;
        set_arg_payload(self, &lhs);
        rhs.payload = 3;
        set(self, lhs, rhs);
        break;
    case 0xD9:
        lhs.type = c;
        set_arg_payload(self, &lhs);
        rhs.payload = 3;
        set(self, lhs, rhs);
        break;
    case 0xDA:
        lhs.type = d;
        set_arg_payload(self, &lhs);
        rhs.payload = 3;
        set(self, lhs, rhs);
        break;
    case 0xDB:
        lhs.type = e;
        set_arg_payload(self, &lhs);
        rhs.payload = 3;
        set(self, lhs, rhs);
        break;
    case 0xDC:
        lhs.type = h;
        set_arg_payload(self, &lhs);
        rhs.payload = 3;
        set(self, lhs, rhs);
        break;
    case 0xDD:
        lhs.type = l;
        set_arg_payload(self, &lhs);
        rhs.payload = 3;
        set(self, lhs, rhs);
        break;
    case 0xDE:
        lhs.type = phl;
        set_arg_payload(self, &lhs);
        rhs.payload = 3;
        set(self, lhs, rhs);
        break;
    case 0xDF:
        lhs.type = a;
        set_arg_payload(self, &lhs);
        rhs.payload = 3;
        set(self, lhs, rhs);
        break;
    case 0xE0:
        lhs.type = b;
        set_arg_payload(self, &lhs);
        rhs.payload = 4;
        set(self, lhs, rhs);
        break;
    case 0xE1:
        lhs.type = c;
        set_arg_payload(self, &lhs);
        rhs.payload = 4;
        set(self, lhs, rhs);
        break;
    case 0xE2:
        lhs.type = d;
        set_arg_payload(self, &lhs);
        rhs.payload = 4;
        set(self, lhs, rhs);
        break;
    case 0xE3:
        lhs.type = e;
        set_arg_payload(self, &lhs);
        rhs.payload = 4;
        set(self, lhs, rhs);
        break;
    case 0xE4:
        lhs.type = h;
        set_arg_payload(self, &lhs);
        rhs.payload = 4;
        set(self, lhs, rhs);
        break;
    case 0xE5:
        lhs.type = l;
        set_arg_payload(self, &lhs);
        rhs.payload = 4;
        set(self, lhs, rhs);
        break;
    case 0xE6:
        lhs.type = phl;
        set_arg_payload(self, &lhs);
        rhs.payload = 4;
        set(self, lhs, rhs);
        break;
    case 0xE7:
        lhs.type = a;
        set_arg_payload(self, &lhs);
        rhs.payload = 4;
        set(self, lhs, rhs);
        break;
    case 0xE8:
        lhs.type = b;
        set_arg_payload(self, &lhs);
        rhs.payload = 5;
        set(self, lhs, rhs);
        break;
    case 0xE9:
        lhs.type = c;
        set_arg_payload(self, &lhs);
        rhs.payload = 5;
        set(self, lhs, rhs);
        break;
    case 0xEA:
        lhs.type = d;
        set_arg_payload(self, &lhs);
        rhs.payload = 5;
        set(self, lhs, rhs);
        break;
    case 0xEB:
        lhs.type = e;
        set_arg_payload(self, &lhs);
        rhs.payload = 5;
        set(self, lhs, rhs);
        break;
    case 0xEC:
        lhs.type = h;
        set_arg_payload(self, &lhs);
        rhs.payload = 5;
        set(self, lhs, rhs);
        break;
    case 0xED:
        lhs.type = l;
        set_arg_payload(self, &lhs);
        rhs.payload = 5;
        set(self, lhs, rhs);
        break;
    case 0xEE:
        lhs.type = phl;
        set_arg_payload(self, &lhs);
        rhs.payload = 5;
        set(self, lhs, rhs);
        break;
    case 0xEF:
        lhs.type = a;
        set_arg_payload(self, &lhs);
        rhs.payload = 5;
        set(self, lhs, rhs);
        break;
    case 0xF0:
        lhs.type = b;
        set_arg_payload(self, &lhs);
        rhs.payload = 6;
        set(self, lhs, rhs);
        break;
    case 0xF1:
        lhs.type = c;
        set_arg_payload(self, &lhs);
        rhs.payload = 6;
        set(self, lhs, rhs);
        break;
    case 0xF2:
        lhs.type = d;
        set_arg_payload(self, &lhs);
        rhs.payload = 6;
        set(self, lhs, rhs);
        break;
    case 0xF3:
        lhs.type = e;
        set_arg_payload(self, &lhs);
        rhs.payload = 6;
        set(self, lhs, rhs);
        break;
    case 0xF4:
        lhs.type = h;
        set_arg_payload(self, &lhs);
        rhs.payload = 6;
        set(self, lhs, rhs);
        break;
    case 0xF5:
        lhs.type = l;
        set_arg_payload(self, &lhs);
        rhs.payload = 6;
        set(self, lhs, rhs);
        break;
    case 0xF6:
        lhs.type = phl;
        set_arg_payload(self, &lhs);
        rhs.payload = 6;
        set(self, lhs, rhs);
        break;
    case 0xF7:
        lhs.type = a;
        set_arg_payload(self, &lhs);
        rhs.payload = 6;
        set(self, lhs, rhs);
        break;
    case 0xF8:
        lhs.type = b;
        set_arg_payload(self, &lhs);
        rhs.payload = 7;
        set(self, lhs, rhs);
        break;
    case 0xF9:
        lhs.type = c;
        set_arg_payload(self, &lhs);
        rhs.payload = 7;
        set(self, lhs, rhs);
        break;
    case 0xFA:
        lhs.type = d;
        set_arg_payload(self, &lhs);
        rhs.payload = 7;
        set(self, lhs, rhs);
        break;
    case 0xFB:
        lhs.type = e;
        set_arg_payload(self, &lhs);
        rhs.payload = 7;
        set(self, lhs, rhs);
        break;
    case 0xFC:
        lhs.type = h;
        set_arg_payload(self, &lhs);
        rhs.payload = 7;
        set(self, lhs, rhs);
        break;
    case 0xFD:
        lhs.type = l;
        set_arg_payload(self, &lhs);
        rhs.payload = 7;
        set(self, lhs, rhs);
        break;
    case 0xFE:
        lhs.type = phl;
        set_arg_payload(self, &lhs);
        rhs.payload = 7;
        set(self, lhs, rhs);
        break;
    case 0xFF:
        lhs.type = a;
        set_arg_payload(self, &lhs);
        rhs.payload = 7;
        set(self, lhs, rhs);
        break;
    }
}
