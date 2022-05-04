#include "cpu.h"
#include "cpu_utils.h"

#define NULL_ARG                                                                                   \
    { a, 0x0000, none, 0x00 }
#define A_ARG                                                                                      \
    { a, 0x0000, none, 0x00 }
#define B_ARG                                                                                      \
    { b, 0x0000, none, 0x00 }
#define C_ARG                                                                                      \
    { c, 0x0000, none, 0x00 }
#define D_ARG                                                                                      \
    { d, 0x0000, none, 0x00 }
#define E_ARG                                                                                      \
    { e, 0x0000, none, 0x00 }
#define H_ARG                                                                                      \
    { h, 0x0000, none, 0x00 }
#define L_ARG                                                                                      \
    { l, 0x0000, none, 0x00 }
#define AF_ARG                                                                                     \
    { af, 0x0000, none, 0x00 }
#define BC_ARG                                                                                     \
    { bc, 0x0000, none, 0x00 }
#define DE_ARG                                                                                     \
    { de, 0x0000, none, 0x00 }
#define HL_ARG                                                                                     \
    { hl, 0x0000, none, 0x00 }
#define SP_ARG                                                                                     \
    { sp, 0x0000, none, 0x00 }
#define PC_ARG                                                                                     \
    { pc, 0x0000, none, 0x00 }
#define P_ARG                                                                                      \
    { p, 0x0000, none, 0x00 }
#define IMM_I8_ARG(cond)                                                                           \
    { imm_i8, 0x0000, cond, 0x00 }
#define IMM_U8_ARG                                                                                 \
    { imm_u8, 0x0000, none, 0x00 }
#define IMM_U16_ARG                                                                                \
    { imm_u16, 0x0000, none, 0x00 }
#define IO_OFFSET_ARG                                                                              \
    { io_offset, 0x0000, none, 0x00 }
#define SP_OFFSET_ARG                                                                              \
    { io_offset, 0x0000, none, 0x00 }
#define PAF_ARG                                                                                    \
    { paf, 0x0000, none, 0x00 }
#define PBC_ARG                                                                                    \
    { pbc, 0x0000, none, 0x00 }
#define PDE_ARG                                                                                    \
    { pde, 0x0000, none, 0x00 }
#define PHL_ARG                                                                                    \
    { phl, 0x0000, none, 0x00 }
#define PHLI_ARG                                                                                   \
    { phli, 0x0000, none, 0x00 }
#define PHLD_ARG                                                                                   \
    { phld, 0x0000, none, 0x00 }

const char *ARGUMENT_NAME[26] = {
    "a",         /* A Register */
    "f",         /* F Register */
    "b",         /* B Register */
    "c",         /* C Register */
    "d",         /* D Register */
    "e",         /* E Register */
    "h",         /* H Register */
    "l",         /* L Register */
    "paf",       /* Pointer to [af] */
    "pbc",       /* Pointer to [bc] */
    "pde",       /* Pointer to [de] */
    "phl",       /* Pointer to [hl] */
    "phli",      /* Pointer to [hl+] */
    "phld",      /* Pointer to [hl-] */
    "af",        /* AF Register */
    "bc",        /* BC Register */
    "de",        /* DE Register */
    "hl",        /* HL Register */
    "sp",        /* SP Register */
    "pc",        /* PC Register */
    "p",         /* LHS u16 */
    "imm_u8",    /* Immediate unsigned 8 bit value */
    "imm_i8",    /* Immediate signed 8 bit value */
    "imm_u16",   /* Immediate unsigned 16 bit value */
    "io_offset", /* Offset from IO base address 0xFF00 */
    "sp_offset"  /* Offset from stack pointer address */
};

const instr OPCODE_TABLE[0x40] = {
    /* 0x00 */
    {
        NULL_ARG,
        NULL_ARG,
        noop_instr,
        1,
    },
    /* 0x01 */
    {
        BC_ARG,
        IMM_U16_ARG,
        ld_instr,
        3,
    },
    /* 0x02 */
    {
        PBC_ARG,
        A_ARG,
        ld_instr,
        2,
    },
    /* 0x03 */
    {
        BC_ARG,
        NULL_ARG,
        inc_instr,
        2,
    },
    /* 0x04 */
    {
        B_ARG,
        NULL_ARG,
        inc_instr,
        1,
    },
    /* 0x05 */
    {
        B_ARG,
        NULL_ARG,
        dec_instr,
        1,
    },
    /* 0x06 */
    {
        B_ARG,
        IMM_U8_ARG,
        ld_instr,
        2,
    },
    /* 0x07 */
    {
        NULL_ARG,
        NULL_ARG,
        rlca_instr,
        1,
    },
    /* 0x08 */
    {
        P_ARG,
        SP_ARG,
        ld_instr,
        5,
    },
    /* 0x09 */
    {
        HL_ARG,
        BC_ARG,
        add_instr,
        2,
    },
    /* 0x0A */
    {
        A_ARG,
        PBC_ARG,
        ld_instr,
        2,
    },
    /* 0x0B */
    {
        BC_ARG,
        NULL_ARG,
        dec_instr,
        2,
    },
    /* 0x0C */
    {
        C_ARG,
        NULL_ARG,
        inc_instr,
        1,
    },
    /* 0x0D */
    {
        C_ARG,
        NULL_ARG,
        dec_instr,
        1,
    },
    /* 0x0E */
    {
        C_ARG,
        IMM_U8_ARG,
        ld_instr,
        2,
    },
    /* 0x0F */
    {
        NULL_ARG,
        NULL_ARG,
        rrca_instr,
        1,
    },
    /* 0x10 */
    {
        NULL_ARG,
        NULL_ARG,
        stop_instr,
        1,
    },
    /* 0x11 */
    {
        DE_ARG,
        IMM_U16_ARG,
        ld_instr,
        3,
    },
    /* 0x12 */
    {
        PDE_ARG,
        A_ARG,
        ld_instr,
        2,
    },
    /* 0x13 */
    {
        DE_ARG,
        NULL_ARG,
        inc_instr,
        2,
    },
    /* 0x14 */
    {
        D_ARG,
        NULL_ARG,
        inc_instr,
        1,
    },
    /* 0x15 */
    {
        D_ARG,
        NULL_ARG,
        dec_instr,
        1,
    },
    /* 0x16 */
    {
        D_ARG,
        IMM_U8_ARG,
        ld_instr,
        2,
    },
    /* 0x17 */
    {
        NULL_ARG,
        NULL_ARG,
        rla_instr,
        1,
    },
    /* 0x18 */
    {
        IMM_I8_ARG(none),
        NULL_ARG,
        jr_instr,
        3,
    },
    /* 0x19 */
    {
        HL_ARG,
        DE_ARG,
        add_instr,
        2,
    },
    /* 0x1A */
    {
        A_ARG,
        PDE_ARG,
        ld_instr,
        2,
    },
    /* 0x1B */
    {
        DE_ARG,
        NULL_ARG,
        dec_instr,
        2,
    },
    /* 0x1C */
    {
        E_ARG,
        NULL_ARG,
        inc_instr,
        1,
    },
    /* 0x1D */
    {
        E_ARG,
        NULL_ARG,
        dec_instr,
        1,
    },
    /* 0x1E */
    {
        E_ARG,
        IMM_U8_ARG,
        ld_instr,
        2,
    },
    /* 0x1F */
    {
        NULL_ARG,
        NULL_ARG,
        rra_instr,
        1,
    },
    /* 0x20 */
    {
        IMM_I8_ARG(nzero),
        NULL_ARG,
        jr_instr,
        /* TODO: branching */
        3,
    },
    /* 0x21 */
    {
        HL_ARG,
        IMM_U16_ARG,
        ld_instr,
        3,
    },
    /* 0x22 */
    {
        PHLI_ARG,
        A_ARG,
        ld_instr,
        2,
    },
    /* 0x23 */
    {
        HL_ARG,
        NULL_ARG,
        inc_instr,
        2,
    },
    /* 0x24 */
    {
        H_ARG,
        NULL_ARG,
        inc_instr,
        1,
    },
    /* 0x25 */
    {
        H_ARG,
        NULL_ARG,
        dec_instr,
        1,
    },
    /* 0x26 */
    {
        H_ARG,
        IMM_U8_ARG,
        ld_instr,
        2,
    },
    /* 0x27 */
    {
        NULL_ARG,
        NULL_ARG,
        daa_instr,
        1,
    },
    /* 0x28 */
    {
        IMM_I8_ARG(zero),
        NULL_ARG,
        jr_instr,
        /* TODO: Branching */
        3,
    },
    /* 0x29 */
    {
        HL_ARG,
        HL_ARG,
        add_instr,
        2,
    },
    /* 0x2A */
    {
        A_ARG,
        PHL_ARG,
        ld_instr,
        2,
    },
    /* 0x2B */
    {
        HL_ARG,
        NULL_ARG,
        dec_instr,
        2,
    },
    /* 0x2C */
    {
        L_ARG,
        NULL_ARG,
        inc_instr,
        1,
    },
    /* 0x2D */
    {
        L_ARG,
        NULL_ARG,
        dec_instr,
        1,
    },
    /* 0x2E */
    {
        L_ARG,
        IMM_U8_ARG,
        ld_instr,
        2,
    },
    /* 0x2F */
    {
        NULL_ARG,
        NULL_ARG,
        cpl_instr,
        1,
    },
    /* 0x30 */
    {
        IMM_I8_ARG(ncarry),
        NULL_ARG,
        jr_instr,
        3,
    },
    /* 0x31 */
    {
        SP_ARG,
        IMM_U16_ARG,
        ld_instr,
        3,
    },
    /* 0x32 */
    {
        PHLD_ARG,
        A_ARG,
        ld_instr,
        2,
    },
    /* 0x33 */
    {
        SP_ARG,
        NULL_ARG,
        inc_instr,
        2,
    },
    /* 0x34 */
    {
        PHL_ARG,
        NULL_ARG,
        inc_instr,
        3,
    },
    /* 0x35 */
    {
        PHL_ARG,
        NULL_ARG,
        dec_instr,
        3,
    },
    /* 0x36 */
    {
        HL_ARG,
        IMM_U8_ARG,
        ld_instr,
        3,
    },
    /* 0x37 */
    {
        NULL_ARG,
        NULL_ARG,
        scf_instr,
        1,
    },
    /* 0x38 */
    {
        IMM_I8_ARG(carry),
        NULL_ARG,
        jr_instr,
        3,
    },
    /* 0x39 */
    {
        HL_ARG,
        SP_ARG,
        add_instr,
        2,
    },
    /* 0x3A */
    {
        A_ARG,
        PHLD_ARG,
        ld_instr,
        2,
    },
    /* 0x3B */
    {
        SP_ARG,
        NULL_ARG,
        dec_instr,
        2,
    },
    /* 0x3C */
    {
        A_ARG,
        NULL_ARG,
        inc_instr,
        1,
    },
    /* 0x3D */
    {
        A_ARG,
        NULL_ARG,
        dec_instr,
        1,
    },
    /* 0x3E */
    {
        A_ARG,
        IMM_U8_ARG,
        ld_instr,
        2,
    },
    /* 0x3F */
    {
        NULL_ARG,
        NULL_ARG,
        ccf_instr,
        1,
    },
};

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
