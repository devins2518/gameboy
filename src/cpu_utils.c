#include "cpu.h"
#include "cpu_utils.h"

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

void ignore_arg(argument_t *arg) { (void)arg; }
