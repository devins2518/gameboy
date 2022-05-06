#include "cpu.h"
#include "cpu_utils.h"

#define NULL_ARG                                                                                   \
    { a, false, 0x0000, none_cond, false }
#define COND_ARG(cond)                                                                             \
    { a, false, 0x0000, cond, false }
#define A_ARG                                                                                      \
    { a, false, 0x0000, none_cond, false }
#define B_ARG                                                                                      \
    { b, false, 0x0000, none_cond, false }
#define C_ARG                                                                                      \
    { c, false, 0x0000, none_cond, false }
#define D_ARG                                                                                      \
    { d, false, 0x0000, none_cond, false }
#define E_ARG                                                                                      \
    { e, false, 0x0000, none_cond, false }
#define H_ARG                                                                                      \
    { h, false, 0x0000, none_cond, false }
#define L_ARG                                                                                      \
    { l, false, 0x0000, none_cond, false }
#define AF_ARG                                                                                     \
    { af, false, 0x0000, none_cond, false }
#define BC_ARG                                                                                     \
    { bc, false, 0x0000, none_cond, false }
#define DE_ARG                                                                                     \
    { de, false, 0x0000, none_cond, false }
#define HL_ARG                                                                                     \
    { hl, false, 0x0000, none_cond, false }
#define SP_ARG                                                                                     \
    { sp, false, 0x0000, none_cond, false }
#define PC_ARG                                                                                     \
    { pc, false, 0x0000, none_cond, false }
#define P_ARG                                                                                      \
    { p, false, 0x0000, none_cond, false }
#define IMM_I8_ARG(cond)                                                                           \
    { imm_i8, false, 0x0000, cond, false }
#define IMM_U8_ARG                                                                                 \
    { imm_u8, false, 0x0000, none_cond, false }
#define IMM_U16_ARG                                                                                \
    { imm_u16, false, 0x0000, none_cond, false }
#define IO_OFFSET_ARG(type)                                                                        \
    { type, true, 0x0000, none_cond, false }
#define SP_OFFSET_ARG                                                                              \
    { imm_i8, false, 0x0000, none_cond, false }
#define PAF_ARG                                                                                    \
    { paf, false, 0x0000, none_cond, false }
#define PBC_ARG                                                                                    \
    { pbc, false, 0x0000, none_cond, false }
#define PDE_ARG                                                                                    \
    { pde, false, 0x0000, none_cond, false }
#define PHL_ARG                                                                                    \
    { phl, false, 0x0000, none_cond, false }
#define PHLI_ARG                                                                                   \
    { phli, false, 0x0000, none_cond, false }
#define PHLD_ARG                                                                                   \
    { phld, false, 0x0000, none_cond, false }
#define FIXED_PAYLOAD_ARG(payload)                                                                 \
    { a, false, payload, none_cond, false }

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

const instr OPCODE_TABLE[0x100] = {
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
        IMM_I8_ARG(none_cond),
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
        IMM_I8_ARG(nzero_cond),
        NULL_ARG,
        jr_instr,
        /* TODO: branching */ 3,
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
        IMM_I8_ARG(zero_cond),
        NULL_ARG,
        jr_instr,
        /* TODO: Branching */ 3,
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
        IMM_I8_ARG(ncarry_cond),
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
        IMM_I8_ARG(carry_cond),
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
    /* 0x40 */
    {
        B_ARG,
        B_ARG,
        ld_instr,
        1,
    },
    /* 0x41 */
    {
        B_ARG,
        C_ARG,
        ld_instr,
        1,
    },
    /* 0x42 */
    {
        B_ARG,
        D_ARG,
        ld_instr,
        1,
    },
    /* 0x43 */
    {
        B_ARG,
        E_ARG,
        ld_instr,
        1,
    },
    /* 0x44 */
    {
        B_ARG,
        H_ARG,
        ld_instr,
        1,
    },
    /* 0x45 */
    {
        B_ARG,
        L_ARG,
        ld_instr,
        1,
    },
    /* 0x46 */
    {
        B_ARG,
        PHL_ARG,
        ld_instr,
        2,
    },
    /* 0x47 */
    {
        B_ARG,
        A_ARG,
        ld_instr,
        1,
    },
    /* 0x48 */
    {
        C_ARG,
        B_ARG,
        ld_instr,
        1,
    },
    /* 0x49 */
    {
        C_ARG,
        C_ARG,
        ld_instr,
        1,
    },
    /* 0x4A */
    {
        C_ARG,
        D_ARG,
        ld_instr,
        1,
    },
    /* 0x4B */
    {
        C_ARG,
        E_ARG,
        ld_instr,
        1,
    },
    /* 0x4C */
    {
        C_ARG,
        H_ARG,
        ld_instr,
        1,
    },
    /* 0x4D */
    {
        C_ARG,
        L_ARG,
        ld_instr,
        1,
    },
    /* 0x4E */
    {
        C_ARG,
        PHL_ARG,
        ld_instr,
        2,
    },
    /* 0x4F */
    {
        C_ARG,
        A_ARG,
        ld_instr,
        1,
    },
    /* 0x50 */
    {
        D_ARG,
        B_ARG,
        ld_instr,
        1,
    },
    /* 0x51 */
    {
        D_ARG,
        C_ARG,
        ld_instr,
        1,
    },
    /* 0x52 */
    {
        D_ARG,
        D_ARG,
        ld_instr,
        1,
    },
    /* 0x53 */
    {
        D_ARG,
        E_ARG,
        ld_instr,
        1,
    },
    /* 0x54 */
    {
        D_ARG,
        H_ARG,
        ld_instr,
        1,
    },
    /* 0x55 */
    {
        D_ARG,
        L_ARG,
        ld_instr,
        1,
    },
    /* 0x56 */
    {
        D_ARG,
        PHL_ARG,
        ld_instr,
        2,
    },
    /* 0x57 */
    {
        D_ARG,
        A_ARG,
        ld_instr,
        1,
    },
    /* 0x58 */
    {
        E_ARG,
        B_ARG,
        ld_instr,
        1,
    },
    /* 0x59 */
    {
        E_ARG,
        C_ARG,
        ld_instr,
        1,
    },
    /* 0x5A */
    {
        E_ARG,
        D_ARG,
        ld_instr,
        1,
    },
    /* 0x5B */
    {
        E_ARG,
        E_ARG,
        ld_instr,
        1,
    },
    /* 0x5C */
    {
        E_ARG,
        H_ARG,
        ld_instr,
        1,
    },
    /* 0x5D */
    {
        E_ARG,
        L_ARG,
        ld_instr,
        1,
    },
    /* 0x5E */
    {
        E_ARG,
        PHL_ARG,
        ld_instr,
        2,
    },
    /* 0x5F */
    {
        E_ARG,
        A_ARG,
        ld_instr,
        1,
    },
    /* 0x60 */
    {
        H_ARG,
        B_ARG,
        ld_instr,
        1,
    },
    /* 0x61 */
    {
        H_ARG,
        C_ARG,
        ld_instr,
        1,
    },
    /* 0x62 */
    {
        H_ARG,
        D_ARG,
        ld_instr,
        1,
    },
    /* 0x63 */
    {
        H_ARG,
        E_ARG,
        ld_instr,
        1,
    },
    /* 0x64 */
    {
        H_ARG,
        H_ARG,
        ld_instr,
        1,
    },
    /* 0x65 */
    {
        H_ARG,
        L_ARG,
        ld_instr,
        1,
    },
    /* 0x66 */
    {
        H_ARG,
        PHL_ARG,
        ld_instr,
        2,
    },
    /* 0x67 */
    {
        H_ARG,
        A_ARG,
        ld_instr,
        1,
    },
    /* 0x68 */
    {
        L_ARG,
        B_ARG,
        ld_instr,
        1,
    },
    /* 0x69 */
    {
        L_ARG,
        C_ARG,
        ld_instr,
        1,
    },
    /* 0x6A */
    {
        L_ARG,
        D_ARG,
        ld_instr,
        1,
    },
    /* 0x6B */
    {
        L_ARG,
        E_ARG,
        ld_instr,
        1,
    },
    /* 0x6C */
    {
        L_ARG,
        H_ARG,
        ld_instr,
        1,
    },
    /* 0x6D */
    {
        L_ARG,
        L_ARG,
        ld_instr,
        1,
    },
    /* 0x6E */
    {
        L_ARG,
        PHL_ARG,
        ld_instr,
        2,
    },
    /* 0x6F */
    {
        L_ARG,
        A_ARG,
        ld_instr,
        1,
    },
    /* 0x70 */
    {
        PHL_ARG,
        B_ARG,
        ld_instr,
        2,
    },
    /* 0x71 */
    {
        PHL_ARG,
        C_ARG,
        ld_instr,
        2,
    },
    /* 0x72 */
    {
        PHL_ARG,
        D_ARG,
        ld_instr,
        2,
    },
    /* 0x73 */
    {
        PHL_ARG,
        E_ARG,
        ld_instr,
        2,
    },
    /* 0x74 */
    {
        PHL_ARG,
        H_ARG,
        ld_instr,
        2,
    },
    /* 0x75 */
    {
        PHL_ARG,
        L_ARG,
        ld_instr,
        2,
    },
    /* 0x76 */
    {
        NULL_ARG,
        NULL_ARG,
        halt_instr,
        1,
    },
    /* 0x77 */
    {
        PHL_ARG,
        A_ARG,
        ld_instr,
        2,
    },
    /* 0x78 */
    {
        A_ARG,
        B_ARG,
        ld_instr,
        1,
    },
    /* 0x79 */
    {
        A_ARG,
        C_ARG,
        ld_instr,
        1,
    },
    /* 0x7A */
    {
        A_ARG,
        D_ARG,
        ld_instr,
        1,
    },
    /* 0x7B */
    {
        A_ARG,
        E_ARG,
        ld_instr,
        1,
    },
    /* 0x7C */
    {
        A_ARG,
        H_ARG,
        ld_instr,
        1,
    },
    /* 0x7D */
    {
        A_ARG,
        L_ARG,
        ld_instr,
        1,
    },
    /* 0x7E */
    {
        A_ARG,
        PHL_ARG,
        ld_instr,
        2,
    },
    /* 0x7F */
    {
        A_ARG,
        A_ARG,
        ld_instr,
        1,
    },
    /* 0x80 */
    {A_ARG, B_ARG, add_instr, 1},
    /* 0x81 */
    {A_ARG, C_ARG, add_instr, 1},
    /* 0x82 */
    {A_ARG, D_ARG, add_instr, 1},
    /* 0x83 */
    {A_ARG, E_ARG, add_instr, 1},
    /* 0x84 */
    {A_ARG, H_ARG, add_instr, 1},
    /* 0x85 */
    {A_ARG, L_ARG, add_instr, 1},
    /* 0x86 */
    {A_ARG, PHL_ARG, add_instr, 2},
    /* 0x87 */
    {A_ARG, A_ARG, add_instr, 1},
    /* 0x88 */
    {A_ARG, B_ARG, adc_instr, 1},
    /* 0x89 */
    {A_ARG, C_ARG, adc_instr, 1},
    /* 0x8A */
    {A_ARG, D_ARG, adc_instr, 1},
    /* 0x8B */
    {A_ARG, E_ARG, adc_instr, 1},
    /* 0x8C */
    {A_ARG, H_ARG, adc_instr, 1},
    /* 0x8D */
    {A_ARG, L_ARG, adc_instr, 1},
    /* 0x8E */
    {A_ARG, PHL_ARG, adc_instr, 2},
    /* 0x8F */
    {A_ARG, A_ARG, adc_instr, 1},
    /* 0x90 */
    {A_ARG, B_ARG, sub_instr, 1},
    /* 0x91 */
    {A_ARG, C_ARG, sub_instr, 1},
    /* 0x92 */
    {A_ARG, D_ARG, sub_instr, 1},
    /* 0x93 */
    {A_ARG, E_ARG, sub_instr, 1},
    /* 0x94 */
    {A_ARG, H_ARG, sub_instr, 1},
    /* 0x95 */
    {A_ARG, L_ARG, sub_instr, 1},
    /* 0x96 */
    {A_ARG, PHL_ARG, sub_instr, 2},
    /* 0x97 */
    {A_ARG, A_ARG, sub_instr, 1},
    /* 0x98 */
    {A_ARG, B_ARG, sbc_instr, 1},
    /* 0x99 */
    {A_ARG, C_ARG, sbc_instr, 1},
    /* 0x9A */
    {A_ARG, D_ARG, sbc_instr, 1},
    /* 0x9B */
    {A_ARG, E_ARG, sbc_instr, 1},
    /* 0x9C */
    {A_ARG, H_ARG, sbc_instr, 1},
    /* 0x9D */
    {A_ARG, L_ARG, sbc_instr, 1},
    /* 0x9E */
    {A_ARG, PHL_ARG, sbc_instr, 2},
    /* 0x9F */
    {A_ARG, A_ARG, sbc_instr, 1},
    /* 0xA0 */
    {A_ARG, B_ARG, and_instr, 1},
    /* 0xA1 */
    {A_ARG, C_ARG, and_instr, 1},
    /* 0xA2 */
    {A_ARG, D_ARG, and_instr, 1},
    /* 0xA3 */
    {A_ARG, E_ARG, and_instr, 1},
    /* 0xA4 */
    {A_ARG, H_ARG, and_instr, 1},
    /* 0xA5 */
    {A_ARG, L_ARG, and_instr, 1},
    /* 0xA6 */
    {A_ARG, PHL_ARG, and_instr, 2},
    /* 0xA7 */
    {A_ARG, A_ARG, and_instr, 1},
    /* 0xA8 */
    {A_ARG, B_ARG, xor_instr, 1},
    /* 0xA9 */
    {A_ARG, C_ARG, xor_instr, 1},
    /* 0xAA */
    {A_ARG, D_ARG, xor_instr, 1},
    /* 0xAB */
    {A_ARG, E_ARG, xor_instr, 1},
    /* 0xAC */
    {A_ARG, H_ARG, xor_instr, 1},
    /* 0xAD */
    {A_ARG, L_ARG, xor_instr, 1},
    /* 0xAE */
    {A_ARG, PHL_ARG, xor_instr, 2},
    /* 0xAF */
    {A_ARG, A_ARG, xor_instr, 1},
    /* 0xB0 */
    {A_ARG, B_ARG, or_instr, 1},
    /* 0xB1 */
    {A_ARG, C_ARG, or_instr, 1},
    /* 0xB2 */
    {A_ARG, D_ARG, or_instr, 1},
    /* 0xB3 */
    {A_ARG, E_ARG, or_instr, 1},
    /* 0xB4 */
    {A_ARG, H_ARG, or_instr, 1},
    /* 0xB5 */
    {A_ARG, L_ARG, or_instr, 1},
    /* 0xB6 */
    {A_ARG, PHL_ARG, or_instr, 2},
    /* 0xB7 */
    {A_ARG, A_ARG, or_instr, 1},
    /* 0xB8 */
    {A_ARG, B_ARG, cp_instr, 1},
    /* 0xB9 */
    {A_ARG, C_ARG, cp_instr, 1},
    /* 0xBA */
    {A_ARG, D_ARG, cp_instr, 1},
    /* 0xBB */
    {A_ARG, E_ARG, cp_instr, 1},
    /* 0xBC */
    {A_ARG, H_ARG, cp_instr, 1},
    /* 0xBD */
    {A_ARG, L_ARG, cp_instr, 1},
    /* 0xBE */
    {A_ARG, PHL_ARG, cp_instr, 2},
    /* 0xBF */
    {A_ARG, A_ARG, cp_instr, 1},
    /* 0xC0 */
    {COND_ARG(nzero_cond), NULL_ARG, ret_instr, 5},
    /* 0xC1 */
    {BC_ARG, NULL_ARG, pop_instr, 3},
    /* 0xC2 */
    {COND_ARG(nzero_cond), IMM_U16_ARG, jp_instr, 4},
    /* 0xC3 */
    {IMM_U16_ARG, NULL_ARG, jp_instr, 4},
    /* 0xC4 */
    {COND_ARG(nzero_cond), IMM_U16_ARG, call_instr, 6},
    /* 0xC5 */
    {BC_ARG, NULL_ARG, push_instr, 4},
    /* 0xC6 */
    {A_ARG, IMM_U8_ARG, add_instr, 2},
    /* 0xC7 */
    {FIXED_PAYLOAD_ARG(0x0000), NULL_ARG, rst_instr, 4},
    /* 0xC8 */
    {COND_ARG(zero_cond), NULL_ARG, ret_instr, 5},
    /* 0xC9 */
    {NULL_ARG, NULL_ARG, ret_instr, 4},
    /* 0xCA */
    {COND_ARG(zero_cond), IMM_U16_ARG, jp_instr, 4},
    /* 0xCB */
    {NULL_ARG, NULL_ARG, cb_instr, 1},
    /* 0xCC */
    {COND_ARG(zero_cond), IMM_U16_ARG, call_instr, 6},
    /* 0xCD */
    {IMM_U16_ARG, NULL_ARG, call_instr, 6},
    /* 0xCE */
    {A_ARG, IMM_U8_ARG, adc_instr, 2},
    /* 0xCF */
    {FIXED_PAYLOAD_ARG(0x0008), NULL_ARG, rst_instr, 4},
    /* 0xD0 */
    {COND_ARG(ncarry_cond), NULL_ARG, ret_instr, 5},
    /* 0xD1 */
    {DE_ARG, NULL_ARG, pop_instr, 3},
    /* 0xD2 */
    {COND_ARG(ncarry_cond), IMM_U16_ARG, jp_instr, 4},
    /* 0xD3 */
    {NULL_ARG, NULL_ARG, illegal_instr, 0},
    /* 0xD4 */
    {COND_ARG(ncarry_cond), IMM_U16_ARG, call_instr, 6},
    /* 0xD5 */
    {DE_ARG, NULL_ARG, push_instr, 4},
    /* 0xD6 */
    {A_ARG, IMM_U8_ARG, sub_instr, 2},
    /* 0xD7 */
    {FIXED_PAYLOAD_ARG(0x0010), NULL_ARG, rst_instr, 4},
    /* 0xD8 */
    {COND_ARG(carry_cond), NULL_ARG, ret_instr, 5},
    /* 0xD9 */
    {NULL_ARG, NULL_ARG, reti_instr, 4},
    /* 0xDA */
    {COND_ARG(carry_cond), IMM_U16_ARG, jp_instr, 4},
    /* 0xDB */
    {NULL_ARG, NULL_ARG, illegal_instr, 0},
    /* 0xDC */
    {COND_ARG(carry_cond), IMM_U16_ARG, call_instr, 6},
    /* 0xDD */
    {NULL_ARG, NULL_ARG, illegal_instr, 0},
    /* 0xDE */
    {A_ARG, IMM_U8_ARG, sbc_instr, 2},
    /* 0xDF */
    {FIXED_PAYLOAD_ARG(0x0018), NULL_ARG, rst_instr, 4},
    /* 0xE0 */
    {IO_OFFSET_ARG(imm_i8), A_ARG, ld_instr, 3},
    /* 0xE1 */
    {HL_ARG, NULL_ARG, pop_instr, 3},
    /* 0xE2 */
    {IO_OFFSET_ARG(c), A_ARG, ld_instr, 3},
    /* 0xE3 */
    {NULL_ARG, NULL_ARG, illegal_instr, 0},
    /* 0xE4 */
    {NULL_ARG, NULL_ARG, illegal_instr, 0},
    /* 0xE5 */
    {HL_ARG, NULL_ARG, push_instr, 4},
    /* 0xE6 */
    {A_ARG, IMM_U8_ARG, and_instr, 2},
    /* 0xE7 */
    {FIXED_PAYLOAD_ARG(0x0020), NULL_ARG, rst_instr, 4},
    /* 0xE8 */
    /* TODO: Cond really should be part of instr */
    {SP_ARG, IMM_I8_ARG(none_cond), add_instr, 4},
    /* 0xE9 */
    {HL_ARG, NULL_ARG, jp_instr, 1},
    /* 0xEA */
    {P_ARG, A_ARG, ld_instr, 4},
    /* 0xEB */
    {NULL_ARG, NULL_ARG, illegal_instr, 0},
    /* 0xEC */
    {NULL_ARG, NULL_ARG, illegal_instr, 0},
    /* 0xED */
    {NULL_ARG, NULL_ARG, illegal_instr, 0},
    /* 0xEE */
    {A_ARG, IMM_U8_ARG, xor_instr, 2},
    /* 0xEF */
    {FIXED_PAYLOAD_ARG(0x0028), NULL_ARG, rst_instr, 4},
    /* 0xF0 */
    {A_ARG, IO_OFFSET_ARG(imm_i8), ld_instr, 3},
    /* 0xF1 */
    {AF_ARG, NULL_ARG, pop_instr, 3},
    /* 0xF2 */
    {A_ARG, IO_OFFSET_ARG(c), ld_instr, 2},
    /* 0xF3 */
    {NULL_ARG, NULL_ARG, di_instr, 1},
    /* 0xF4 */
    {NULL_ARG, NULL_ARG, illegal_instr, 0},
    /* 0xF5 */
    {AF_ARG, NULL_ARG, push_instr, 0},
    /* 0xF6 */
    {A_ARG, IMM_U8_ARG, or_instr, 2},
    /* 0xF7 */
    {FIXED_PAYLOAD_ARG(0x0030), NULL_ARG, rst_instr, 4},
    /* 0xF8 */
    {HL_ARG, SP_OFFSET_ARG, ld_instr, 3},
    /* 0xF9 */
    {SP_ARG, HL_ARG, ld_instr, 2},
    /* 0xFA */
    {A_ARG, P_ARG, ld_instr, 4},
    /* 0xFB */
    {NULL_ARG, NULL_ARG, ei_instr, 1},
    /* 0xFC */
    {NULL_ARG, NULL_ARG, illegal_instr, 0},
    /* 0xFD */
    {NULL_ARG, NULL_ARG, illegal_instr, 0},
    /* 0xFE */
    {A_ARG, IMM_U8_ARG, cp_instr, 2},
    /* 0xFF */
    {FIXED_PAYLOAD_ARG(0x0038), NULL_ARG, rst_instr, 4},
};

void resolve_cond(cpu *self, argument_t *arg) {
    switch (arg->cond) {
    case zero_cond:
        arg->should_branch = get_flag_z(self) == 0;
        break;
    case nzero_cond:
        arg->should_branch = get_flag_z(self) != 0;
        break;
    case carry_cond:
        arg->should_branch = get_flag_c(self) == 0;
        break;
    case ncarry_cond:
        arg->should_branch = get_flag_c(self) != 0;
        break;
    case none_cond:
        arg->should_branch = false;
        break;
    };
}

void ignore_arg(argument_t *arg) { (void)arg; }
