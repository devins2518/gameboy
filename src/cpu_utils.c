#include "cpu_utils.h"
#include "cpu.h"

void resolve_payload(cpu *self, argument_t *arg) {
    switch (arg->type) {
    case io_offset:
        arg->payload += 0xFF00;
        break;
    case sp_offset:
        arg->payload += get_sp(self);
        break;
    case imm_u8:
        arg->payload = get_imm_u8(self);
        break;
    case imm_u16:
        arg->payload = get_imm_u16(self);
        break;
    default:
        break;
    };
}

void resolve_cond(cpu *self, argument_t *arg) {
    switch (arg->cond) {
    case zero:
        arg->should_continue = get_flag_z(self) == 0;
        break;
    case nzero:
        arg->should_continue = get_flag_z(self) != 0;
        break;
    case carry:
        arg->should_continue = get_flag_c(self) == 0;
        break;
    case ncarry:
        arg->should_continue = get_flag_c(self) != 0;
        break;
    case none:
        arg->should_continue = TRUE;
        break;
    default:
        arg->should_continue = FALSE;
        break;
    };
}
