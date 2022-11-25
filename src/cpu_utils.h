#ifndef CPU_UTILS_H
#define CPU_UTILS_H

#include "cpu.h"
#include <stdint.h>

/* Caller is responsible for ensuring that payload does not contain invalid
   data. Only valid values with payloads is pointer, immediate values, IO
   offset, and stack pointer offset. CALLING WITH OTHER DISCRIMINANTS WILL
   RETURN GARBAGE. */
typedef struct {
    enum {
        a,         /* A Register */
        f,         /* F Register */
        b,         /* B Register */
        c,         /* C Register */
        d,         /* D Register */
        e,         /* E Register */
        h,         /* H Register */
        l,         /* L Register */
        paf,       /* Pointer to [af] */
        pbc,       /* Pointer to [bc] */
        pde,       /* Pointer to [de] */
        phl,       /* Pointer to [hl] */
        phli,      /* Pointer to [hl+] */
        phld,      /* Pointer to [hl-] */
        pio_u8,    /* Pointer to [IO_START + u8] */
        pio_c,     /* Pointer to [IO_START + c] */
        af,        /* AF Register */
        bc,        /* BC Register */
        de,        /* DE Register */
        hl,        /* HL Register */
        sp,        /* SP Register */
        pc,        /* PC Register */
        p,         /* LHS u16 */
        imm_u8,    /* Immediate unsigned 8 bit value */
        imm_i8,    /* Immediate signed 8 bit value */
        imm_u16,   /* Immediate unsigned 16 bit value */
        sp_offset, /* Offset from stack pointer address */
        none       /* No argument */
    } type;
    uint16_t payload;
    enum { zero_cond, nzero_cond, carry_cond, ncarry_cond, none_cond } cond;
    bool should_branch;
} argument_t;

typedef struct {
    argument_t lhs;
    argument_t rhs;
    enum {
        adc_instr,
        add_instr,
        and_instr,
        bit_instr,
        call_instr,
        cb_instr,
        ccf_instr,
        cp_instr,
        cpl_instr,
        daa_instr,
        dec_instr,
        di_instr,
        ei_instr,
        halt_instr,
        inc_instr,
        jp_instr,
        jr_instr,
        ld_instr,
        noop_instr,
        or_instr,
        pop_instr,
        push_instr,
        res_instr,
        ret_instr,
        reti_instr,
        rl_instr,
        rla_instr,
        rlca_instr,
        rr_instr,
        rra_instr,
        rrca_instr,
        rst_instr,
        sbc_instr,
        scf_instr,
        set_instr,
        sla_instr,
        sra_instr,
        srl_instr,
        stop_instr,
        sub_instr,
        swap_instr,
        xor_instr,
        illegal_instr
    } instr;
    uint8_t clocks;
} instr;

const char *ARGUMENT_NAME[27];
const instr OPCODE_TABLE[0x100];

char *print_instr(instr *instr);

void resolve_cond(cpu *self, argument_t *arg);

void ignore_arg(argument_t *arg);

#endif
