#include "decoder.h"
#include "instruction.h"
#include <stdio.h>

/* clang-format off */
#define NONE_ARG                                                                                   \
    { {0}, none_e }
#define A_ARG                                                                                      \
    { { a_register_p }, register_e }
#define F_ARG                                                                                      \
    { { f_register_p }, register_e }
#define B_ARG                                                                                      \
    { { b_register_p }, register_e }
#define C_ARG                                                                                      \
    { { c_register_p }, register_e }
#define D_ARG                                                                                      \
    { { d_register_p }, register_e }
#define E_ARG                                                                                      \
    { { e_register_p }, register_e }
#define H_ARG                                                                                      \
    { { h_register_p }, register_e }
#define L_ARG                                                                                      \
    { { l_register_p }, register_e }
#define AF_ARG                                                                                     \
    { { af_register_p }, register_e }
#define BC_ARG                                                                                     \
    { { bc_register_p }, register_e }
#define DE_ARG                                                                                     \
    { { de_register_p }, register_e }
#define HL_ARG                                                                                     \
    { { hl_register_p }, register_e }
#define SP_ARG                                                                                     \
    { { sp_register_p }, register_e }
#define NONE_COND_ARG                                                                              \
    { { none_condition_e }, condition_e }
#define NZERO_COND_ARG                                                                             \
    { { nzero_condition_e }, condition_e }
#define ZERO_COND_ARG                                                                              \
    { { zero_condition_e }, condition_e }
#define NCARRY_COND_ARG                                                                            \
    { { ncarry_condition_e }, condition_e }
#define CARRY_COND_ARG                                                                             \
    { { carry_condition_e }, condition_e }
#define IMM_U8_ARG                                                                                 \
    { { 0 }, imm_u8_e }
#define IMM_I8_ARG                                                                                 \
    { { 0 }, imm_i8_e }
#define IMM_U16_ARG                                                                                \
    { { 0 }, imm_u16_e }
#define IMM_U16_PTR_ARG                                                                            \
    { { 0 }, imm_u16_ptr_e }
#define BC_PTR_ARG                                                                                 \
    { { bc_register_ptr_e }, register_ptr_e }
#define DE_PTR_ARG                                                                                 \
    { { de_register_ptr_e }, register_ptr_e }
#define HL_PTR_ARG                                                                                 \
    { { hl_register_ptr_e }, register_ptr_e }
#define HL_PTR_INC_ARG                                                                             \
    { { hl_ptr_inc_e }, hl_ptr_e }
#define HL_PTR_DEC_ARG                                                                             \
    { { hl_ptr_dec_e }, hl_ptr_e }
#define IO_OFFSET_U8                                                                               \
    { { 0 }, io_offset_u8_e }
#define IO_OFFSET_C                                                                                \
    { { c_register_p }, io_offset_c_e }
#define FIXED_PAYLOAD_ARG(payload)                                                                 \
    { { payload }, fixed_payload_e }
#define SP_OFFSET_ARG                                                                              \
    { { 0 }, sp_offset_e }
/* clang-format on */

const instruction_t OP_TABLE[0x100] = {
    {noop_instruction, NONE_ARG, NONE_ARG, 1, 1},               /* 0x00 */
    {ld_instruction, BC_ARG, IMM_U16_ARG, 3, 3},                /* 0x01 */
    {ld_instruction, BC_PTR_ARG, A_ARG, 1, 2},                  /* 0x02 */
    {inc_instruction, BC_ARG, NONE_ARG, 1, 2},                  /* 0x03 */
    {inc_instruction, B_ARG, NONE_ARG, 1, 1},                   /* 0x04 */
    {dec_instruction, B_ARG, NONE_ARG, 1, 1},                   /* 0x05 */
    {ld_instruction, B_ARG, IMM_U8_ARG, 2, 2},                  /* 0x06 */
    {rlca_instruction, NONE_ARG, NONE_ARG, 1, 1},               /* 0x07 */
    {ld_instruction, IMM_U16_PTR_ARG, SP_ARG, 3, 5},            /* 0x08 */
    {add_instruction, HL_ARG, BC_ARG, 1, 2},                    /* 0x09 */
    {ld_instruction, A_ARG, BC_PTR_ARG, 1, 2},                  /* 0x0A */
    {dec_instruction, BC_ARG, NONE_ARG, 1, 2},                  /* 0x0B */
    {inc_instruction, C_ARG, NONE_ARG, 1, 1},                   /* 0x0C */
    {dec_instruction, C_ARG, NONE_ARG, 1, 1},                   /* 0x0D */
    {ld_instruction, C_ARG, IMM_U8_ARG, 2, 2},                  /* 0x0E */
    {rrca_instruction, NONE_ARG, NONE_ARG, 1, 1},               /* 0x0F */
    {stop_instruction, NONE_ARG, NONE_ARG, 1, 1},               /* 0x10 */
    {ld_instruction, DE_ARG, IMM_U16_ARG, 3, 3},                /* 0x11 */
    {ld_instruction, DE_PTR_ARG, A_ARG, 1, 2},                  /* 0x12 */
    {inc_instruction, DE_ARG, NONE_ARG, 1, 2},                  /* 0x13 */
    {inc_instruction, D_ARG, NONE_ARG, 1, 1},                   /* 0x14 */
    {dec_instruction, D_ARG, NONE_ARG, 1, 1},                   /* 0x15 */
    {ld_instruction, D_ARG, IMM_U8_ARG, 2, 2},                  /* 0x16 */
    {rla_instruction, NONE_ARG, NONE_ARG, 1, 1},                /* 0x17 */
    {jr_instruction, NONE_COND_ARG, IMM_I8_ARG, 2, 2},          /* 0x18 */
    {add_instruction, HL_ARG, DE_ARG, 1, 2},                    /* 0x19 */
    {ld_instruction, A_ARG, DE_PTR_ARG, 1, 2},                  /* 0x1A */
    {dec_instruction, DE_ARG, NONE_ARG, 1, 2},                  /* 0x1B */
    {inc_instruction, E_ARG, NONE_ARG, 1, 1},                   /* 0x1C */
    {dec_instruction, E_ARG, NONE_ARG, 1, 1},                   /* 0x1D */
    {ld_instruction, E_ARG, IMM_U8_ARG, 2, 2},                  /* 0x1E */
    {rra_instruction, NONE_ARG, NONE_ARG, 1, 1},                /* 0x1F */
    {jr_instruction, NZERO_COND_ARG, IMM_I8_ARG, 2, 2},         /* 0x20 */
    {ld_instruction, HL_ARG, IMM_U16_ARG, 3, 3},                /* 0x21 */
    {ld_instruction, HL_PTR_INC_ARG, A_ARG, 1, 2},              /* 0x22 */
    {inc_instruction, HL_ARG, NONE_ARG, 1, 2},                  /* 0x23 */
    {inc_instruction, H_ARG, NONE_ARG, 1, 1},                   /* 0x24 */
    {dec_instruction, H_ARG, NONE_ARG, 1, 1},                   /* 0x25 */
    {ld_instruction, H_ARG, IMM_U8_ARG, 2, 2},                  /* 0x26 */
    {daa_instruction, NONE_ARG, NONE_ARG, 1, 1},                /* 0x27 */
    {jr_instruction, ZERO_COND_ARG, IMM_I8_ARG, 2, 2},          /* 0x28 */
    {add_instruction, HL_ARG, HL_ARG, 1, 2},                    /* 0x29 */
    {ld_instruction, A_ARG, HL_PTR_INC_ARG, 1, 2},              /* 0x2A */
    {dec_instruction, HL_ARG, NONE_ARG, 1, 2},                  /* 0x2B */
    {inc_instruction, L_ARG, NONE_ARG, 1, 1},                   /* 0x2C */
    {dec_instruction, L_ARG, NONE_ARG, 1, 1},                   /* 0x2D */
    {ld_instruction, L_ARG, IMM_U8_ARG, 2, 2},                  /* 0x2E */
    {cpl_instruction, NONE_ARG, NONE_ARG, 1, 1},                /* 0x2F */
    {jr_instruction, NCARRY_COND_ARG, IMM_I8_ARG, 2, 2},        /* 0x30 */
    {ld_instruction, SP_ARG, IMM_U16_ARG, 3, 3},                /* 0x31 */
    {ld_instruction, HL_PTR_DEC_ARG, A_ARG, 1, 2},              /* 0x32 */
    {inc_instruction, SP_ARG, NONE_ARG, 1, 2},                  /* 0x33 */
    {inc_instruction, HL_PTR_ARG, NONE_ARG, 1, 3},              /* 0x34 */
    {dec_instruction, HL_PTR_ARG, NONE_ARG, 1, 3},              /* 0x35 */
    {ld_instruction, HL_PTR_ARG, IMM_U8_ARG, 2, 3},             /* 0x36 */
    {scf_instruction, NONE_ARG, NONE_ARG, 1, 1},                /* 0x37 */
    {jr_instruction, CARRY_COND_ARG, IMM_I8_ARG, 2, 2},         /* 0x38 */
    {add_instruction, HL_ARG, SP_ARG, 1, 2},                    /* 0x39 */
    {ld_instruction, A_ARG, HL_PTR_DEC_ARG, 1, 2},              /* 0x3A */
    {dec_instruction, SP_ARG, NONE_ARG, 1, 2},                  /* 0x3B */
    {inc_instruction, A_ARG, NONE_ARG, 1, 1},                   /* 0x3C */
    {dec_instruction, A_ARG, NONE_ARG, 1, 1},                   /* 0x3D */
    {ld_instruction, A_ARG, IMM_U8_ARG, 2, 2},                  /* 0x3E */
    {ccf_instruction, NONE_ARG, NONE_ARG, 1, 1},                /* 0x3F */
    {ld_instruction, B_ARG, B_ARG, 1, 1},                       /* 0x40 */
    {ld_instruction, B_ARG, C_ARG, 1, 1},                       /* 0x41 */
    {ld_instruction, B_ARG, D_ARG, 1, 1},                       /* 0x42 */
    {ld_instruction, B_ARG, E_ARG, 1, 1},                       /* 0x43 */
    {ld_instruction, B_ARG, H_ARG, 1, 1},                       /* 0x44 */
    {ld_instruction, B_ARG, L_ARG, 1, 1},                       /* 0x45 */
    {ld_instruction, B_ARG, HL_PTR_ARG, 1, 2},                  /* 0x46 */
    {ld_instruction, B_ARG, A_ARG, 1, 1},                       /* 0x47 */
    {ld_instruction, C_ARG, B_ARG, 1, 1},                       /* 0x48 */
    {ld_instruction, C_ARG, C_ARG, 1, 1},                       /* 0x49 */
    {ld_instruction, C_ARG, D_ARG, 1, 1},                       /* 0x4A */
    {ld_instruction, C_ARG, E_ARG, 1, 1},                       /* 0x4B */
    {ld_instruction, C_ARG, H_ARG, 1, 1},                       /* 0x4C */
    {ld_instruction, C_ARG, L_ARG, 1, 1},                       /* 0x4D */
    {ld_instruction, C_ARG, HL_PTR_ARG, 1, 2},                  /* 0x4E */
    {ld_instruction, C_ARG, A_ARG, 1, 1},                       /* 0x4F */
    {ld_instruction, D_ARG, B_ARG, 1, 1},                       /* 0x50 */
    {ld_instruction, D_ARG, C_ARG, 1, 1},                       /* 0x51 */
    {ld_instruction, D_ARG, D_ARG, 1, 1},                       /* 0x52 */
    {ld_instruction, D_ARG, E_ARG, 1, 1},                       /* 0x53 */
    {ld_instruction, D_ARG, H_ARG, 1, 1},                       /* 0x54 */
    {ld_instruction, D_ARG, L_ARG, 1, 1},                       /* 0x55 */
    {ld_instruction, D_ARG, HL_PTR_ARG, 1, 2},                  /* 0x56 */
    {ld_instruction, D_ARG, A_ARG, 1, 1},                       /* 0x57 */
    {ld_instruction, E_ARG, B_ARG, 1, 1},                       /* 0x58 */
    {ld_instruction, E_ARG, C_ARG, 1, 1},                       /* 0x59 */
    {ld_instruction, E_ARG, D_ARG, 1, 1},                       /* 0x5A */
    {ld_instruction, E_ARG, E_ARG, 1, 1},                       /* 0x5B */
    {ld_instruction, E_ARG, H_ARG, 1, 1},                       /* 0x5C */
    {ld_instruction, E_ARG, L_ARG, 1, 1},                       /* 0x5D */
    {ld_instruction, E_ARG, HL_PTR_ARG, 1, 2},                  /* 0x5E */
    {ld_instruction, E_ARG, A_ARG, 1, 1},                       /* 0x5F */
    {ld_instruction, H_ARG, B_ARG, 1, 1},                       /* 0x60 */
    {ld_instruction, H_ARG, C_ARG, 1, 1},                       /* 0x61 */
    {ld_instruction, H_ARG, D_ARG, 1, 1},                       /* 0x62 */
    {ld_instruction, H_ARG, E_ARG, 1, 1},                       /* 0x63 */
    {ld_instruction, H_ARG, H_ARG, 1, 1},                       /* 0x64 */
    {ld_instruction, H_ARG, L_ARG, 1, 1},                       /* 0x65 */
    {ld_instruction, H_ARG, HL_PTR_ARG, 1, 2},                  /* 0x66 */
    {ld_instruction, H_ARG, A_ARG, 1, 1},                       /* 0x67 */
    {ld_instruction, L_ARG, B_ARG, 1, 1},                       /* 0x68 */
    {ld_instruction, L_ARG, C_ARG, 1, 1},                       /* 0x69 */
    {ld_instruction, L_ARG, D_ARG, 1, 1},                       /* 0x6A */
    {ld_instruction, L_ARG, E_ARG, 1, 1},                       /* 0x6B */
    {ld_instruction, L_ARG, H_ARG, 1, 1},                       /* 0x6C */
    {ld_instruction, L_ARG, L_ARG, 1, 1},                       /* 0x6D */
    {ld_instruction, L_ARG, HL_PTR_ARG, 1, 2},                  /* 0x6E */
    {ld_instruction, L_ARG, A_ARG, 1, 1},                       /* 0x6F */
    {ld_instruction, HL_PTR_ARG, B_ARG, 1, 2},                  /* 0x70 */
    {ld_instruction, HL_PTR_ARG, C_ARG, 1, 2},                  /* 0x71 */
    {ld_instruction, HL_PTR_ARG, D_ARG, 1, 2},                  /* 0x72 */
    {ld_instruction, HL_PTR_ARG, E_ARG, 1, 2},                  /* 0x73 */
    {ld_instruction, HL_PTR_ARG, H_ARG, 1, 2},                  /* 0x74 */
    {ld_instruction, HL_PTR_ARG, L_ARG, 1, 2},                  /* 0x75 */
    {halt_instruction, NONE_ARG, NONE_ARG, 1, 1},               /* 0x76 */
    {ld_instruction, HL_PTR_ARG, A_ARG, 1, 2},                  /* 0x77 */
    {ld_instruction, A_ARG, B_ARG, 1, 1},                       /* 0x78 */
    {ld_instruction, A_ARG, C_ARG, 1, 1},                       /* 0x79 */
    {ld_instruction, A_ARG, D_ARG, 1, 1},                       /* 0x7A */
    {ld_instruction, A_ARG, E_ARG, 1, 1},                       /* 0x7B */
    {ld_instruction, A_ARG, H_ARG, 1, 1},                       /* 0x7C */
    {ld_instruction, A_ARG, L_ARG, 1, 1},                       /* 0x7D */
    {ld_instruction, A_ARG, HL_PTR_ARG, 1, 2},                  /* 0x7E */
    {ld_instruction, A_ARG, A_ARG, 1, 1},                       /* 0x7F */
    {add_instruction, A_ARG, B_ARG, 1, 1},                      /* 0x80 */
    {add_instruction, A_ARG, C_ARG, 1, 1},                      /* 0x81 */
    {add_instruction, A_ARG, D_ARG, 1, 1},                      /* 0x82 */
    {add_instruction, A_ARG, E_ARG, 1, 1},                      /* 0x83 */
    {add_instruction, A_ARG, H_ARG, 1, 1},                      /* 0x84 */
    {add_instruction, A_ARG, L_ARG, 1, 1},                      /* 0x85 */
    {add_instruction, A_ARG, HL_PTR_ARG, 1, 2},                 /* 0x86 */
    {add_instruction, A_ARG, A_ARG, 1, 1},                      /* 0x87 */
    {adc_instruction, A_ARG, B_ARG, 1, 1},                      /* 0x88 */
    {adc_instruction, A_ARG, C_ARG, 1, 1},                      /* 0x89 */
    {adc_instruction, A_ARG, D_ARG, 1, 1},                      /* 0x8A */
    {adc_instruction, A_ARG, E_ARG, 1, 1},                      /* 0x8B */
    {adc_instruction, A_ARG, H_ARG, 1, 1},                      /* 0x8C */
    {adc_instruction, A_ARG, L_ARG, 1, 1},                      /* 0x8D */
    {adc_instruction, A_ARG, HL_PTR_ARG, 1, 2},                 /* 0x8E */
    {adc_instruction, A_ARG, A_ARG, 1, 1},                      /* 0x8F */
    {sub_instruction, A_ARG, B_ARG, 1, 1},                      /* 0x90 */
    {sub_instruction, A_ARG, C_ARG, 1, 1},                      /* 0x91 */
    {sub_instruction, A_ARG, D_ARG, 1, 1},                      /* 0x92 */
    {sub_instruction, A_ARG, E_ARG, 1, 1},                      /* 0x93 */
    {sub_instruction, A_ARG, H_ARG, 1, 1},                      /* 0x94 */
    {sub_instruction, A_ARG, L_ARG, 1, 1},                      /* 0x95 */
    {sub_instruction, A_ARG, HL_PTR_ARG, 1, 2},                 /* 0x96 */
    {sub_instruction, A_ARG, A_ARG, 1, 1},                      /* 0x97 */
    {sbc_instruction, A_ARG, B_ARG, 1, 1},                      /* 0x98 */
    {sbc_instruction, A_ARG, C_ARG, 1, 1},                      /* 0x99 */
    {sbc_instruction, A_ARG, D_ARG, 1, 1},                      /* 0x9A */
    {sbc_instruction, A_ARG, E_ARG, 1, 1},                      /* 0x9B */
    {sbc_instruction, A_ARG, H_ARG, 1, 1},                      /* 0x9C */
    {sbc_instruction, A_ARG, L_ARG, 1, 1},                      /* 0x9D */
    {sbc_instruction, A_ARG, HL_PTR_ARG, 1, 2},                 /* 0x9E */
    {sbc_instruction, A_ARG, A_ARG, 1, 1},                      /* 0x9F */
    {and_instruction, A_ARG, B_ARG, 1, 1},                      /* 0xA0 */
    {and_instruction, A_ARG, C_ARG, 1, 1},                      /* 0xA1 */
    {and_instruction, A_ARG, D_ARG, 1, 1},                      /* 0xA2 */
    {and_instruction, A_ARG, E_ARG, 1, 1},                      /* 0xA3 */
    {and_instruction, A_ARG, H_ARG, 1, 1},                      /* 0xA4 */
    {and_instruction, A_ARG, L_ARG, 1, 1},                      /* 0xA5 */
    {and_instruction, A_ARG, HL_PTR_ARG, 1, 2},                 /* 0xA6 */
    {and_instruction, A_ARG, A_ARG, 1, 1},                      /* 0xA7 */
    {xor_instruction, A_ARG, B_ARG, 1, 1},                      /* 0xA8 */
    {xor_instruction, A_ARG, C_ARG, 1, 1},                      /* 0xA9 */
    {xor_instruction, A_ARG, D_ARG, 1, 1},                      /* 0xAA */
    {xor_instruction, A_ARG, E_ARG, 1, 1},                      /* 0xAB */
    {xor_instruction, A_ARG, H_ARG, 1, 1},                      /* 0xAC */
    {xor_instruction, A_ARG, L_ARG, 1, 1},                      /* 0xAD */
    {xor_instruction, A_ARG, HL_PTR_ARG, 1, 2},                 /* 0xAE */
    {xor_instruction, A_ARG, A_ARG, 1, 1},                      /* 0xAF */
    {or_instruction, A_ARG, B_ARG, 1, 1},                       /* 0xB0 */
    {or_instruction, A_ARG, C_ARG, 1, 1},                       /* 0xB1 */
    {or_instruction, A_ARG, D_ARG, 1, 1},                       /* 0xB2 */
    {or_instruction, A_ARG, E_ARG, 1, 1},                       /* 0xB3 */
    {or_instruction, A_ARG, H_ARG, 1, 1},                       /* 0xB4 */
    {or_instruction, A_ARG, L_ARG, 1, 1},                       /* 0xB5 */
    {or_instruction, A_ARG, HL_PTR_ARG, 1, 2},                  /* 0xB6 */
    {or_instruction, A_ARG, A_ARG, 1, 1},                       /* 0xB7 */
    {cp_instruction, A_ARG, B_ARG, 1, 1},                       /* 0xB8 */
    {cp_instruction, A_ARG, C_ARG, 1, 1},                       /* 0xB9 */
    {cp_instruction, A_ARG, D_ARG, 1, 1},                       /* 0xBA */
    {cp_instruction, A_ARG, E_ARG, 1, 1},                       /* 0xBB */
    {cp_instruction, A_ARG, H_ARG, 1, 1},                       /* 0xBC */
    {cp_instruction, A_ARG, L_ARG, 1, 1},                       /* 0xBD */
    {cp_instruction, A_ARG, HL_PTR_ARG, 1, 2},                  /* 0xBE */
    {cp_instruction, A_ARG, A_ARG, 1, 1},                       /* 0xBF */
    {ret_instruction, NZERO_COND_ARG, NONE_ARG, 1, 2},          /* 0xC0 */
    {pop_instruction, BC_ARG, NONE_ARG, 1, 3},                  /* 0xC1 */
    {jp_instruction, NZERO_COND_ARG, IMM_U16_ARG, 3, 3},        /* 0xC2 */
    {jp_instruction, NONE_COND_ARG, IMM_U16_ARG, 3, 4},         /* 0xC3 */
    {call_instruction, NZERO_COND_ARG, IMM_U16_ARG, 3, 3},      /* 0xC4 */
    {push_instruction, BC_ARG, NONE_ARG, 1, 4},                 /* 0xC5 */
    {add_instruction, A_ARG, IMM_U8_ARG, 2, 2},                 /* 0xC6 */
    {rst_instruction, FIXED_PAYLOAD_ARG(0x00), NONE_ARG, 1, 4}, /* 0xC7 */
    {ret_instruction, ZERO_COND_ARG, NONE_ARG, 1, 2},           /* 0xC8 */
    {ret_instruction, NONE_COND_ARG, NONE_ARG, 1, 4},           /* 0xC9 */
    {jp_instruction, ZERO_COND_ARG, IMM_U16_ARG, 3, 3},         /* 0xCA */
    {illegal_instruction, NONE_ARG, NONE_ARG, 1, 1},            /* 0xCB */
    {call_instruction, ZERO_COND_ARG, IMM_U16_ARG, 3, 3},       /* 0xCC */
    {call_instruction, NONE_COND_ARG, IMM_U16_ARG, 3, 6},       /* 0xCD */
    {adc_instruction, A_ARG, IMM_U8_ARG, 2, 2},                 /* 0xCE */
    {rst_instruction, FIXED_PAYLOAD_ARG(0x08), NONE_ARG, 1, 4}, /* 0xCF */
    {ret_instruction, NCARRY_COND_ARG, NONE_ARG, 1, 2},         /* 0xD0 */
    {pop_instruction, DE_ARG, NONE_ARG, 1, 3},                  /* 0xD1 */
    {jp_instruction, NCARRY_COND_ARG, IMM_U16_ARG, 3, 3},       /* 0xD2 */
    {illegal_instruction, NONE_ARG, NONE_ARG, 1, 1},            /* 0xD3 */
    {call_instruction, NCARRY_COND_ARG, IMM_U16_ARG, 3, 3},     /* 0xD4 */
    {push_instruction, DE_ARG, NONE_ARG, 1, 4},                 /* 0xD5 */
    {sub_instruction, A_ARG, IMM_U8_ARG, 2, 2},                 /* 0xD6 */
    {rst_instruction, FIXED_PAYLOAD_ARG(0x10), NONE_ARG, 1, 4}, /* 0xD7 */
    {ret_instruction, CARRY_COND_ARG, NONE_ARG, 1, 2},          /* 0xD8 */
    {reti_instruction, NONE_ARG, NONE_ARG, 1, 4},               /* 0xD9 */
    {jp_instruction, CARRY_COND_ARG, IMM_U16_ARG, 3, 3},        /* 0xDA */
    {illegal_instruction, NONE_ARG, NONE_ARG, 1, 1},            /* 0xDB */
    {call_instruction, CARRY_COND_ARG, IMM_U16_ARG, 3, 3},      /* 0xDC */
    {illegal_instruction, NONE_ARG, NONE_ARG, 1, 1},            /* 0xDD */
    {sbc_instruction, A_ARG, IMM_U8_ARG, 2, 2},                 /* 0xDE */
    {rst_instruction, FIXED_PAYLOAD_ARG(0x18), NONE_ARG, 1, 4}, /* 0xDF */
    {ld_instruction, IO_OFFSET_U8, A_ARG, 2, 3},                /* 0xE0 */
    {pop_instruction, HL_ARG, NONE_ARG, 1, 3},                  /* 0xE1 */
    {ld_instruction, IO_OFFSET_C, A_ARG, 1, 2},                 /* 0xE2 */
    {illegal_instruction, NONE_ARG, NONE_ARG, 1, 1},            /* 0xE3 */
    {illegal_instruction, NONE_ARG, NONE_ARG, 1, 1},            /* 0xE4 */
    {push_instruction, HL_ARG, NONE_ARG, 1, 4},                 /* 0xE5 */
    {and_instruction, A_ARG, IMM_U8_ARG, 2, 2},                 /* 0xE6 */
    {rst_instruction, FIXED_PAYLOAD_ARG(0x20), NONE_ARG, 1, 4}, /* 0xE7 */
    {add_instruction, SP_ARG, IMM_I8_ARG, 2, 4},                /* 0xE8 */
    {jp_instruction, NONE_COND_ARG, HL_ARG, 1, 1},              /* 0xE9 */
    {ld_instruction, IMM_U16_PTR_ARG, A_ARG, 3, 4},             /* 0xEA */
    {illegal_instruction, NONE_ARG, NONE_ARG, 1, 1},            /* 0xEB */
    {illegal_instruction, NONE_ARG, NONE_ARG, 1, 1},            /* 0xEC */
    {illegal_instruction, NONE_ARG, NONE_ARG, 1, 1},            /* 0xED */
    {xor_instruction, A_ARG, IMM_U8_ARG, 2, 2},                 /* 0xEE */
    {rst_instruction, FIXED_PAYLOAD_ARG(0x28), NONE_ARG, 1, 4}, /* 0xEF */
    {ld_instruction, A_ARG, IO_OFFSET_U8, 2, 3},                /* 0xF0 */
    {pop_instruction, AF_ARG, NONE_ARG, 1, 3},                  /* 0xF1 */
    {ld_instruction, A_ARG, IO_OFFSET_C, 1, 2},                 /* 0xF2 */
    {di_instruction, NONE_ARG, NONE_ARG, 1, 1},                 /* 0xF3 */
    {illegal_instruction, NONE_ARG, NONE_ARG, 1, 1},            /* 0xF4 */
    {push_instruction, AF_ARG, NONE_ARG, 1, 4},                 /* 0xF5 */
    {or_instruction, A_ARG, IMM_U8_ARG, 2, 2},                  /* 0xF6 */
    {rst_instruction, FIXED_PAYLOAD_ARG(0x30), NONE_ARG, 1, 4}, /* 0xF7 */
    {ld_instruction, HL_ARG, SP_OFFSET_ARG, 2, 3},              /* 0xF8 */
    {ld_instruction, SP_ARG, HL_ARG, 1, 2},                     /* 0xF9 */
    {ld_instruction, A_ARG, IMM_U16_PTR_ARG, 3, 4},             /* 0xFA */
    {ei_instruction, NONE_ARG, NONE_ARG, 1, 1},                 /* 0xFB */
    {illegal_instruction, NONE_ARG, NONE_ARG, 1, 1},            /* 0xFC */
    {illegal_instruction, NONE_ARG, NONE_ARG, 1, 1},            /* 0xFD */
    {cp_instruction, A_ARG, IMM_U8_ARG, 2, 2},                  /* 0xFE */
    {rst_instruction, FIXED_PAYLOAD_ARG(0x38), NONE_ARG, 1, 4}, /* 0xFF */
};
const instruction_t CB_TABLE[0x100] = {
    {rlc_instruction, B_ARG, NONE_ARG, 2, 2},                  /* 0xCB00 */
    {rlc_instruction, C_ARG, NONE_ARG, 2, 2},                  /* 0xCB01 */
    {rlc_instruction, D_ARG, NONE_ARG, 2, 2},                  /* 0xCB02 */
    {rlc_instruction, E_ARG, NONE_ARG, 2, 2},                  /* 0xCB03 */
    {rlc_instruction, H_ARG, NONE_ARG, 2, 2},                  /* 0xCB04 */
    {rlc_instruction, L_ARG, NONE_ARG, 2, 2},                  /* 0xCB05 */
    {rlc_instruction, HL_PTR_ARG, NONE_ARG, 2, 4},             /* 0xCB06 */
    {rlc_instruction, A_ARG, NONE_ARG, 2, 2},                  /* 0xCB07 */
    {rrc_instruction, B_ARG, NONE_ARG, 2, 2},                  /* 0xCB08 */
    {rrc_instruction, C_ARG, NONE_ARG, 2, 2},                  /* 0xCB09 */
    {rrc_instruction, D_ARG, NONE_ARG, 2, 2},                  /* 0xCB0A */
    {rrc_instruction, E_ARG, NONE_ARG, 2, 2},                  /* 0xCB0B */
    {rrc_instruction, H_ARG, NONE_ARG, 2, 2},                  /* 0xCB0C */
    {rrc_instruction, L_ARG, NONE_ARG, 2, 2},                  /* 0xCB0D */
    {rrc_instruction, HL_PTR_ARG, NONE_ARG, 2, 4},             /* 0xCB0E */
    {rrc_instruction, A_ARG, NONE_ARG, 2, 2},                  /* 0xCB0F */
    {rl_instruction, B_ARG, NONE_ARG, 2, 2},                   /* 0xCB10 */
    {rl_instruction, C_ARG, NONE_ARG, 2, 2},                   /* 0xCB11 */
    {rl_instruction, D_ARG, NONE_ARG, 2, 2},                   /* 0xCB12 */
    {rl_instruction, E_ARG, NONE_ARG, 2, 2},                   /* 0xCB13 */
    {rl_instruction, H_ARG, NONE_ARG, 2, 2},                   /* 0xCB14 */
    {rl_instruction, L_ARG, NONE_ARG, 2, 2},                   /* 0xCB15 */
    {rl_instruction, HL_PTR_ARG, NONE_ARG, 2, 4},              /* 0xCB16 */
    {rl_instruction, A_ARG, NONE_ARG, 2, 2},                   /* 0xCB17 */
    {rr_instruction, B_ARG, NONE_ARG, 2, 2},                   /* 0xCB18 */
    {rr_instruction, C_ARG, NONE_ARG, 2, 2},                   /* 0xCB19 */
    {rr_instruction, D_ARG, NONE_ARG, 2, 2},                   /* 0xCB1A */
    {rr_instruction, E_ARG, NONE_ARG, 2, 2},                   /* 0xCB1B */
    {rr_instruction, H_ARG, NONE_ARG, 2, 2},                   /* 0xCB1C */
    {rr_instruction, L_ARG, NONE_ARG, 2, 2},                   /* 0xCB1D */
    {rr_instruction, HL_PTR_ARG, NONE_ARG, 2, 4},              /* 0xCB1E */
    {rr_instruction, A_ARG, NONE_ARG, 2, 2},                   /* 0xCB1F */
    {sla_instruction, B_ARG, NONE_ARG, 2, 2},                  /* 0xCB20 */
    {sla_instruction, C_ARG, NONE_ARG, 2, 2},                  /* 0xCB21 */
    {sla_instruction, D_ARG, NONE_ARG, 2, 2},                  /* 0xCB22 */
    {sla_instruction, E_ARG, NONE_ARG, 2, 2},                  /* 0xCB23 */
    {sla_instruction, H_ARG, NONE_ARG, 2, 2},                  /* 0xCB24 */
    {sla_instruction, L_ARG, NONE_ARG, 2, 2},                  /* 0xCB25 */
    {sla_instruction, HL_PTR_ARG, NONE_ARG, 2, 4},             /* 0xCB26 */
    {sla_instruction, A_ARG, NONE_ARG, 2, 2},                  /* 0xCB27 */
    {sra_instruction, B_ARG, NONE_ARG, 2, 2},                  /* 0xCB28 */
    {sra_instruction, C_ARG, NONE_ARG, 2, 2},                  /* 0xCB29 */
    {sra_instruction, D_ARG, NONE_ARG, 2, 2},                  /* 0xCB2A */
    {sra_instruction, E_ARG, NONE_ARG, 2, 2},                  /* 0xCB2B */
    {sra_instruction, H_ARG, NONE_ARG, 2, 2},                  /* 0xCB2C */
    {sra_instruction, L_ARG, NONE_ARG, 2, 2},                  /* 0xCB2D */
    {sra_instruction, HL_PTR_ARG, NONE_ARG, 2, 4},             /* 0xCB2E */
    {sra_instruction, A_ARG, NONE_ARG, 2, 2},                  /* 0xCB2F */
    {swap_instruction, B_ARG, NONE_ARG, 2, 2},                 /* 0xCB30 */
    {swap_instruction, C_ARG, NONE_ARG, 2, 2},                 /* 0xCB31 */
    {swap_instruction, D_ARG, NONE_ARG, 2, 2},                 /* 0xCB32 */
    {swap_instruction, E_ARG, NONE_ARG, 2, 2},                 /* 0xCB33 */
    {swap_instruction, H_ARG, NONE_ARG, 2, 2},                 /* 0xCB34 */
    {swap_instruction, L_ARG, NONE_ARG, 2, 2},                 /* 0xCB35 */
    {swap_instruction, HL_PTR_ARG, NONE_ARG, 2, 4},            /* 0xCB36 */
    {swap_instruction, A_ARG, NONE_ARG, 2, 2},                 /* 0xCB37 */
    {srl_instruction, B_ARG, NONE_ARG, 2, 2},                  /* 0xCB38 */
    {srl_instruction, C_ARG, NONE_ARG, 2, 2},                  /* 0xCB39 */
    {srl_instruction, D_ARG, NONE_ARG, 2, 2},                  /* 0xCB3A */
    {srl_instruction, E_ARG, NONE_ARG, 2, 2},                  /* 0xCB3B */
    {srl_instruction, H_ARG, NONE_ARG, 2, 2},                  /* 0xCB3C */
    {srl_instruction, L_ARG, NONE_ARG, 2, 2},                  /* 0xCB3D */
    {srl_instruction, HL_PTR_ARG, NONE_ARG, 2, 4},             /* 0xCB3E */
    {srl_instruction, A_ARG, NONE_ARG, 2, 2},                  /* 0xCB3F */
    {bit_instruction, FIXED_PAYLOAD_ARG(0), B_ARG, 2, 2},      /* 0xCB40 */
    {bit_instruction, FIXED_PAYLOAD_ARG(0), C_ARG, 2, 2},      /* 0xCB41 */
    {bit_instruction, FIXED_PAYLOAD_ARG(0), D_ARG, 2, 2},      /* 0xCB42 */
    {bit_instruction, FIXED_PAYLOAD_ARG(0), E_ARG, 2, 2},      /* 0xCB43 */
    {bit_instruction, FIXED_PAYLOAD_ARG(0), H_ARG, 2, 2},      /* 0xCB44 */
    {bit_instruction, FIXED_PAYLOAD_ARG(0), L_ARG, 2, 2},      /* 0xCB45 */
    {bit_instruction, FIXED_PAYLOAD_ARG(0), HL_PTR_ARG, 2, 3}, /* 0xCB46 */
    {bit_instruction, FIXED_PAYLOAD_ARG(0), A_ARG, 2, 2},      /* 0xCB47 */
    {bit_instruction, FIXED_PAYLOAD_ARG(1), B_ARG, 2, 2},      /* 0xCB48 */
    {bit_instruction, FIXED_PAYLOAD_ARG(1), C_ARG, 2, 2},      /* 0xCB49 */
    {bit_instruction, FIXED_PAYLOAD_ARG(1), D_ARG, 2, 2},      /* 0xCB4A */
    {bit_instruction, FIXED_PAYLOAD_ARG(1), E_ARG, 2, 2},      /* 0xCB4B */
    {bit_instruction, FIXED_PAYLOAD_ARG(1), H_ARG, 2, 2},      /* 0xCB4C */
    {bit_instruction, FIXED_PAYLOAD_ARG(1), L_ARG, 2, 2},      /* 0xCB4D */
    {bit_instruction, FIXED_PAYLOAD_ARG(1), HL_PTR_ARG, 2, 3}, /* 0xCB4E */
    {bit_instruction, FIXED_PAYLOAD_ARG(1), A_ARG, 2, 2},      /* 0xCB4F */
    {bit_instruction, FIXED_PAYLOAD_ARG(2), B_ARG, 2, 2},      /* 0xCB50 */
    {bit_instruction, FIXED_PAYLOAD_ARG(2), C_ARG, 2, 2},      /* 0xCB51 */
    {bit_instruction, FIXED_PAYLOAD_ARG(2), D_ARG, 2, 2},      /* 0xCB52 */
    {bit_instruction, FIXED_PAYLOAD_ARG(2), E_ARG, 2, 2},      /* 0xCB53 */
    {bit_instruction, FIXED_PAYLOAD_ARG(2), H_ARG, 2, 2},      /* 0xCB54 */
    {bit_instruction, FIXED_PAYLOAD_ARG(2), L_ARG, 2, 2},      /* 0xCB55 */
    {bit_instruction, FIXED_PAYLOAD_ARG(2), HL_PTR_ARG, 2, 3}, /* 0xCB56 */
    {bit_instruction, FIXED_PAYLOAD_ARG(2), A_ARG, 2, 2},      /* 0xCB57 */
    {bit_instruction, FIXED_PAYLOAD_ARG(3), B_ARG, 2, 2},      /* 0xCB58 */
    {bit_instruction, FIXED_PAYLOAD_ARG(3), C_ARG, 2, 2},      /* 0xCB59 */
    {bit_instruction, FIXED_PAYLOAD_ARG(3), D_ARG, 2, 2},      /* 0xCB5A */
    {bit_instruction, FIXED_PAYLOAD_ARG(3), E_ARG, 2, 2},      /* 0xCB5B */
    {bit_instruction, FIXED_PAYLOAD_ARG(3), H_ARG, 2, 2},      /* 0xCB5C */
    {bit_instruction, FIXED_PAYLOAD_ARG(3), L_ARG, 2, 2},      /* 0xCB5D */
    {bit_instruction, FIXED_PAYLOAD_ARG(3), HL_PTR_ARG, 2, 3}, /* 0xCB5E */
    {bit_instruction, FIXED_PAYLOAD_ARG(3), A_ARG, 2, 2},      /* 0xCB5F */
    {bit_instruction, FIXED_PAYLOAD_ARG(4), B_ARG, 2, 2},      /* 0xCB60 */
    {bit_instruction, FIXED_PAYLOAD_ARG(4), C_ARG, 2, 2},      /* 0xCB61 */
    {bit_instruction, FIXED_PAYLOAD_ARG(4), D_ARG, 2, 2},      /* 0xCB62 */
    {bit_instruction, FIXED_PAYLOAD_ARG(4), E_ARG, 2, 2},      /* 0xCB63 */
    {bit_instruction, FIXED_PAYLOAD_ARG(4), H_ARG, 2, 2},      /* 0xCB64 */
    {bit_instruction, FIXED_PAYLOAD_ARG(4), L_ARG, 2, 2},      /* 0xCB65 */
    {bit_instruction, FIXED_PAYLOAD_ARG(4), HL_PTR_ARG, 2, 3}, /* 0xCB66 */
    {bit_instruction, FIXED_PAYLOAD_ARG(4), A_ARG, 2, 2},      /* 0xCB67 */
    {bit_instruction, FIXED_PAYLOAD_ARG(5), B_ARG, 2, 2},      /* 0xCB68 */
    {bit_instruction, FIXED_PAYLOAD_ARG(5), C_ARG, 2, 2},      /* 0xCB69 */
    {bit_instruction, FIXED_PAYLOAD_ARG(5), D_ARG, 2, 2},      /* 0xCB6A */
    {bit_instruction, FIXED_PAYLOAD_ARG(5), E_ARG, 2, 2},      /* 0xCB6B */
    {bit_instruction, FIXED_PAYLOAD_ARG(5), H_ARG, 2, 2},      /* 0xCB6C */
    {bit_instruction, FIXED_PAYLOAD_ARG(5), L_ARG, 2, 2},      /* 0xCB6D */
    {bit_instruction, FIXED_PAYLOAD_ARG(5), HL_PTR_ARG, 2, 3}, /* 0xCB6E */
    {bit_instruction, FIXED_PAYLOAD_ARG(5), A_ARG, 2, 2},      /* 0xCB6F */
    {bit_instruction, FIXED_PAYLOAD_ARG(6), B_ARG, 2, 2},      /* 0xCB70 */
    {bit_instruction, FIXED_PAYLOAD_ARG(6), C_ARG, 2, 2},      /* 0xCB71 */
    {bit_instruction, FIXED_PAYLOAD_ARG(6), D_ARG, 2, 2},      /* 0xCB72 */
    {bit_instruction, FIXED_PAYLOAD_ARG(6), E_ARG, 2, 2},      /* 0xCB73 */
    {bit_instruction, FIXED_PAYLOAD_ARG(6), H_ARG, 2, 2},      /* 0xCB74 */
    {bit_instruction, FIXED_PAYLOAD_ARG(6), L_ARG, 2, 2},      /* 0xCB75 */
    {bit_instruction, FIXED_PAYLOAD_ARG(6), HL_PTR_ARG, 2, 3}, /* 0xCB76 */
    {bit_instruction, FIXED_PAYLOAD_ARG(6), A_ARG, 2, 2},      /* 0xCB77 */
    {bit_instruction, FIXED_PAYLOAD_ARG(7), B_ARG, 2, 2},      /* 0xCB78 */
    {bit_instruction, FIXED_PAYLOAD_ARG(7), C_ARG, 2, 2},      /* 0xCB79 */
    {bit_instruction, FIXED_PAYLOAD_ARG(7), D_ARG, 2, 2},      /* 0xCB7A */
    {bit_instruction, FIXED_PAYLOAD_ARG(7), E_ARG, 2, 2},      /* 0xCB7B */
    {bit_instruction, FIXED_PAYLOAD_ARG(7), H_ARG, 2, 2},      /* 0xCB7C */
    {bit_instruction, FIXED_PAYLOAD_ARG(7), L_ARG, 2, 2},      /* 0xCB7D */
    {bit_instruction, FIXED_PAYLOAD_ARG(7), HL_PTR_ARG, 2, 3}, /* 0xCB7E */
    {bit_instruction, FIXED_PAYLOAD_ARG(7), A_ARG, 2, 2},      /* 0xCB7F */
    {res_instruction, FIXED_PAYLOAD_ARG(0), B_ARG, 2, 2},      /* 0xCB80 */
    {res_instruction, FIXED_PAYLOAD_ARG(0), C_ARG, 2, 2},      /* 0xCB81 */
    {res_instruction, FIXED_PAYLOAD_ARG(0), D_ARG, 2, 2},      /* 0xCB82 */
    {res_instruction, FIXED_PAYLOAD_ARG(0), E_ARG, 2, 2},      /* 0xCB83 */
    {res_instruction, FIXED_PAYLOAD_ARG(0), H_ARG, 2, 2},      /* 0xCB84 */
    {res_instruction, FIXED_PAYLOAD_ARG(0), L_ARG, 2, 2},      /* 0xCB85 */
    {res_instruction, FIXED_PAYLOAD_ARG(0), HL_PTR_ARG, 2, 4}, /* 0xCB86 */
    {res_instruction, FIXED_PAYLOAD_ARG(0), A_ARG, 2, 2},      /* 0xCB87 */
    {res_instruction, FIXED_PAYLOAD_ARG(1), B_ARG, 2, 2},      /* 0xCB88 */
    {res_instruction, FIXED_PAYLOAD_ARG(1), C_ARG, 2, 2},      /* 0xCB89 */
    {res_instruction, FIXED_PAYLOAD_ARG(1), D_ARG, 2, 2},      /* 0xCB8A */
    {res_instruction, FIXED_PAYLOAD_ARG(1), E_ARG, 2, 2},      /* 0xCB8B */
    {res_instruction, FIXED_PAYLOAD_ARG(1), H_ARG, 2, 2},      /* 0xCB8C */
    {res_instruction, FIXED_PAYLOAD_ARG(1), L_ARG, 2, 2},      /* 0xCB8D */
    {res_instruction, FIXED_PAYLOAD_ARG(1), HL_PTR_ARG, 2, 4}, /* 0xCB8E */
    {res_instruction, FIXED_PAYLOAD_ARG(1), A_ARG, 2, 2},      /* 0xCB8F */
    {res_instruction, FIXED_PAYLOAD_ARG(2), B_ARG, 2, 2},      /* 0xCB90 */
    {res_instruction, FIXED_PAYLOAD_ARG(2), C_ARG, 2, 2},      /* 0xCB91 */
    {res_instruction, FIXED_PAYLOAD_ARG(2), D_ARG, 2, 2},      /* 0xCB92 */
    {res_instruction, FIXED_PAYLOAD_ARG(2), E_ARG, 2, 2},      /* 0xCB93 */
    {res_instruction, FIXED_PAYLOAD_ARG(2), H_ARG, 2, 2},      /* 0xCB94 */
    {res_instruction, FIXED_PAYLOAD_ARG(2), L_ARG, 2, 2},      /* 0xCB95 */
    {res_instruction, FIXED_PAYLOAD_ARG(2), HL_PTR_ARG, 2, 4}, /* 0xCB96 */
    {res_instruction, FIXED_PAYLOAD_ARG(2), A_ARG, 2, 2},      /* 0xCB97 */
    {res_instruction, FIXED_PAYLOAD_ARG(3), B_ARG, 2, 2},      /* 0xCB98 */
    {res_instruction, FIXED_PAYLOAD_ARG(3), C_ARG, 2, 2},      /* 0xCB99 */
    {res_instruction, FIXED_PAYLOAD_ARG(3), D_ARG, 2, 2},      /* 0xCB9A */
    {res_instruction, FIXED_PAYLOAD_ARG(3), E_ARG, 2, 2},      /* 0xCB9B */
    {res_instruction, FIXED_PAYLOAD_ARG(3), H_ARG, 2, 2},      /* 0xCB9C */
    {res_instruction, FIXED_PAYLOAD_ARG(3), L_ARG, 2, 2},      /* 0xCB9D */
    {res_instruction, FIXED_PAYLOAD_ARG(3), HL_PTR_ARG, 2, 4}, /* 0xCB9E */
    {res_instruction, FIXED_PAYLOAD_ARG(3), A_ARG, 2, 2},      /* 0xCB9F */
    {res_instruction, FIXED_PAYLOAD_ARG(4), B_ARG, 2, 2},      /* 0xCBA0 */
    {res_instruction, FIXED_PAYLOAD_ARG(4), C_ARG, 2, 2},      /* 0xCBA1 */
    {res_instruction, FIXED_PAYLOAD_ARG(4), D_ARG, 2, 2},      /* 0xCBA2 */
    {res_instruction, FIXED_PAYLOAD_ARG(4), E_ARG, 2, 2},      /* 0xCBA3 */
    {res_instruction, FIXED_PAYLOAD_ARG(4), H_ARG, 2, 2},      /* 0xCBA4 */
    {res_instruction, FIXED_PAYLOAD_ARG(4), L_ARG, 2, 2},      /* 0xCBA5 */
    {res_instruction, FIXED_PAYLOAD_ARG(4), HL_PTR_ARG, 2, 4}, /* 0xCBA6 */
    {res_instruction, FIXED_PAYLOAD_ARG(4), A_ARG, 2, 2},      /* 0xCBA7 */
    {res_instruction, FIXED_PAYLOAD_ARG(5), B_ARG, 2, 2},      /* 0xCBA8 */
    {res_instruction, FIXED_PAYLOAD_ARG(5), C_ARG, 2, 2},      /* 0xCBA9 */
    {res_instruction, FIXED_PAYLOAD_ARG(5), D_ARG, 2, 2},      /* 0xCBAA */
    {res_instruction, FIXED_PAYLOAD_ARG(5), E_ARG, 2, 2},      /* 0xCBAB */
    {res_instruction, FIXED_PAYLOAD_ARG(5), H_ARG, 2, 2},      /* 0xCBAC */
    {res_instruction, FIXED_PAYLOAD_ARG(5), L_ARG, 2, 2},      /* 0xCBAD */
    {res_instruction, FIXED_PAYLOAD_ARG(5), HL_PTR_ARG, 2, 4}, /* 0xCBAE */
    {res_instruction, FIXED_PAYLOAD_ARG(5), A_ARG, 2, 2},      /* 0xCBAF */
    {res_instruction, FIXED_PAYLOAD_ARG(6), B_ARG, 2, 2},      /* 0xCBB0 */
    {res_instruction, FIXED_PAYLOAD_ARG(6), C_ARG, 2, 2},      /* 0xCBB1 */
    {res_instruction, FIXED_PAYLOAD_ARG(6), D_ARG, 2, 2},      /* 0xCBB2 */
    {res_instruction, FIXED_PAYLOAD_ARG(6), E_ARG, 2, 2},      /* 0xCBB3 */
    {res_instruction, FIXED_PAYLOAD_ARG(6), H_ARG, 2, 2},      /* 0xCBB4 */
    {res_instruction, FIXED_PAYLOAD_ARG(6), L_ARG, 2, 2},      /* 0xCBB5 */
    {res_instruction, FIXED_PAYLOAD_ARG(6), HL_PTR_ARG, 2, 4}, /* 0xCBB6 */
    {res_instruction, FIXED_PAYLOAD_ARG(6), A_ARG, 2, 2},      /* 0xCBB7 */
    {res_instruction, FIXED_PAYLOAD_ARG(7), B_ARG, 2, 2},      /* 0xCBB8 */
    {res_instruction, FIXED_PAYLOAD_ARG(7), C_ARG, 2, 2},      /* 0xCBB9 */
    {res_instruction, FIXED_PAYLOAD_ARG(7), D_ARG, 2, 2},      /* 0xCBBA */
    {res_instruction, FIXED_PAYLOAD_ARG(7), E_ARG, 2, 2},      /* 0xCBBB */
    {res_instruction, FIXED_PAYLOAD_ARG(7), H_ARG, 2, 2},      /* 0xCBBC */
    {res_instruction, FIXED_PAYLOAD_ARG(7), L_ARG, 2, 2},      /* 0xCBBD */
    {res_instruction, FIXED_PAYLOAD_ARG(7), HL_PTR_ARG, 2, 4}, /* 0xCBBE */
    {res_instruction, FIXED_PAYLOAD_ARG(7), A_ARG, 2, 2},      /* 0xCBBF */
    {set_instruction, FIXED_PAYLOAD_ARG(0), B_ARG, 2, 2},      /* 0xCBC0 */
    {set_instruction, FIXED_PAYLOAD_ARG(0), C_ARG, 2, 2},      /* 0xCBC1 */
    {set_instruction, FIXED_PAYLOAD_ARG(0), D_ARG, 2, 2},      /* 0xCBC2 */
    {set_instruction, FIXED_PAYLOAD_ARG(0), E_ARG, 2, 2},      /* 0xCBC3 */
    {set_instruction, FIXED_PAYLOAD_ARG(0), H_ARG, 2, 2},      /* 0xCBC4 */
    {set_instruction, FIXED_PAYLOAD_ARG(0), L_ARG, 2, 2},      /* 0xCBC5 */
    {set_instruction, FIXED_PAYLOAD_ARG(0), HL_PTR_ARG, 2, 4}, /* 0xCBC6 */
    {set_instruction, FIXED_PAYLOAD_ARG(0), A_ARG, 2, 2},      /* 0xCBC7 */
    {set_instruction, FIXED_PAYLOAD_ARG(1), B_ARG, 2, 2},      /* 0xCBC8 */
    {set_instruction, FIXED_PAYLOAD_ARG(1), C_ARG, 2, 2},      /* 0xCBC9 */
    {set_instruction, FIXED_PAYLOAD_ARG(1), D_ARG, 2, 2},      /* 0xCBCA */
    {set_instruction, FIXED_PAYLOAD_ARG(1), E_ARG, 2, 2},      /* 0xCBCB */
    {set_instruction, FIXED_PAYLOAD_ARG(1), H_ARG, 2, 2},      /* 0xCBCC */
    {set_instruction, FIXED_PAYLOAD_ARG(1), L_ARG, 2, 2},      /* 0xCBCD */
    {set_instruction, FIXED_PAYLOAD_ARG(1), HL_PTR_ARG, 2, 4}, /* 0xCBCE */
    {set_instruction, FIXED_PAYLOAD_ARG(1), A_ARG, 2, 2},      /* 0xCBCF */
    {set_instruction, FIXED_PAYLOAD_ARG(2), B_ARG, 2, 2},      /* 0xCBD0 */
    {set_instruction, FIXED_PAYLOAD_ARG(2), C_ARG, 2, 2},      /* 0xCBD1 */
    {set_instruction, FIXED_PAYLOAD_ARG(2), D_ARG, 2, 2},      /* 0xCBD2 */
    {set_instruction, FIXED_PAYLOAD_ARG(2), E_ARG, 2, 2},      /* 0xCBD3 */
    {set_instruction, FIXED_PAYLOAD_ARG(2), H_ARG, 2, 2},      /* 0xCBD4 */
    {set_instruction, FIXED_PAYLOAD_ARG(2), L_ARG, 2, 2},      /* 0xCBD5 */
    {set_instruction, FIXED_PAYLOAD_ARG(2), HL_PTR_ARG, 2, 4}, /* 0xCBD6 */
    {set_instruction, FIXED_PAYLOAD_ARG(2), A_ARG, 2, 2},      /* 0xCBD7 */
    {set_instruction, FIXED_PAYLOAD_ARG(3), B_ARG, 2, 2},      /* 0xCBD8 */
    {set_instruction, FIXED_PAYLOAD_ARG(3), C_ARG, 2, 2},      /* 0xCBD9 */
    {set_instruction, FIXED_PAYLOAD_ARG(3), D_ARG, 2, 2},      /* 0xCBDA */
    {set_instruction, FIXED_PAYLOAD_ARG(3), E_ARG, 2, 2},      /* 0xCBDB */
    {set_instruction, FIXED_PAYLOAD_ARG(3), H_ARG, 2, 2},      /* 0xCBDC */
    {set_instruction, FIXED_PAYLOAD_ARG(3), L_ARG, 2, 2},      /* 0xCBDD */
    {set_instruction, FIXED_PAYLOAD_ARG(3), HL_PTR_ARG, 2, 4}, /* 0xCBDE */
    {set_instruction, FIXED_PAYLOAD_ARG(3), A_ARG, 2, 2},      /* 0xCBDF */
    {set_instruction, FIXED_PAYLOAD_ARG(4), B_ARG, 2, 2},      /* 0xCBE0 */
    {set_instruction, FIXED_PAYLOAD_ARG(4), C_ARG, 2, 2},      /* 0xCBE1 */
    {set_instruction, FIXED_PAYLOAD_ARG(4), D_ARG, 2, 2},      /* 0xCBE2 */
    {set_instruction, FIXED_PAYLOAD_ARG(4), E_ARG, 2, 2},      /* 0xCBE3 */
    {set_instruction, FIXED_PAYLOAD_ARG(4), H_ARG, 2, 2},      /* 0xCBE4 */
    {set_instruction, FIXED_PAYLOAD_ARG(4), L_ARG, 2, 2},      /* 0xCBE5 */
    {set_instruction, FIXED_PAYLOAD_ARG(4), HL_PTR_ARG, 2, 4}, /* 0xCBE6 */
    {set_instruction, FIXED_PAYLOAD_ARG(4), A_ARG, 2, 2},      /* 0xCBE7 */
    {set_instruction, FIXED_PAYLOAD_ARG(5), B_ARG, 2, 2},      /* 0xCBE8 */
    {set_instruction, FIXED_PAYLOAD_ARG(5), C_ARG, 2, 2},      /* 0xCBE9 */
    {set_instruction, FIXED_PAYLOAD_ARG(5), D_ARG, 2, 2},      /* 0xCBEA */
    {set_instruction, FIXED_PAYLOAD_ARG(5), E_ARG, 2, 2},      /* 0xCBEB */
    {set_instruction, FIXED_PAYLOAD_ARG(5), H_ARG, 2, 2},      /* 0xCBEC */
    {set_instruction, FIXED_PAYLOAD_ARG(5), L_ARG, 2, 2},      /* 0xCBED */
    {set_instruction, FIXED_PAYLOAD_ARG(5), HL_PTR_ARG, 2, 4}, /* 0xCBEE */
    {set_instruction, FIXED_PAYLOAD_ARG(5), A_ARG, 2, 2},      /* 0xCBEF */
    {set_instruction, FIXED_PAYLOAD_ARG(6), B_ARG, 2, 2},      /* 0xCBF0 */
    {set_instruction, FIXED_PAYLOAD_ARG(6), C_ARG, 2, 2},      /* 0xCBF1 */
    {set_instruction, FIXED_PAYLOAD_ARG(6), D_ARG, 2, 2},      /* 0xCBF2 */
    {set_instruction, FIXED_PAYLOAD_ARG(6), E_ARG, 2, 2},      /* 0xCBF3 */
    {set_instruction, FIXED_PAYLOAD_ARG(6), H_ARG, 2, 2},      /* 0xCBF4 */
    {set_instruction, FIXED_PAYLOAD_ARG(6), L_ARG, 2, 2},      /* 0xCBF5 */
    {set_instruction, FIXED_PAYLOAD_ARG(6), HL_PTR_ARG, 2, 4}, /* 0xCBF6 */
    {set_instruction, FIXED_PAYLOAD_ARG(6), A_ARG, 2, 2},      /* 0xCBF7 */
    {set_instruction, FIXED_PAYLOAD_ARG(7), B_ARG, 2, 2},      /* 0xCBF8 */
    {set_instruction, FIXED_PAYLOAD_ARG(7), C_ARG, 2, 2},      /* 0xCBF9 */
    {set_instruction, FIXED_PAYLOAD_ARG(7), D_ARG, 2, 2},      /* 0xCBFA */
    {set_instruction, FIXED_PAYLOAD_ARG(7), E_ARG, 2, 2},      /* 0xCBFB */
    {set_instruction, FIXED_PAYLOAD_ARG(7), H_ARG, 2, 2},      /* 0xCBFC */
    {set_instruction, FIXED_PAYLOAD_ARG(7), L_ARG, 2, 2},      /* 0xCBFD */
    {set_instruction, FIXED_PAYLOAD_ARG(7), HL_PTR_ARG, 2, 4}, /* 0xCBFE */
    {set_instruction, FIXED_PAYLOAD_ARG(7), A_ARG, 2, 2},      /* 0xCBFF */
};

decoder_t decoder_new(const uint8_t *arr, uintptr_t size) {
    decoder_t d;
    d.idx = 0;
    d.arr = arr;
    d.size = size;
    return d;
}

instruction_t decoder_next(decoder_t *d) {
    instruction_t instr;
    uint8_t lhs_size;
    uint8_t rhs_size;
    const uint8_t *op = d->arr + d->idx;
    if (*op == 0xCB) {
        instr = CB_TABLE[*(op + 1) % 0x100];
    } else {
        instr = OP_TABLE[*op % 0x100];
        lhs_size = get_payload_size_argument_t(&instr.lhs);
        rhs_size = get_payload_size_argument_t(&instr.rhs);
        if (lhs_size == 1) {
            instr.lhs.p.imm_u8_p = d->arr[d->idx + 1];
        } else if (rhs_size == 1) {
            instr.rhs.p.imm_u8_p = d->arr[d->idx + 1];
        } else if (lhs_size == 2) {
            instr.lhs.p.imm_u16_p = (d->arr[d->idx + 2]) << 8 | (d->arr[d->idx + 1]);
        } else if (rhs_size == 2) {
            instr.rhs.p.imm_u16_p = (d->arr[d->idx + 2]) << 8 | (d->arr[d->idx + 1]);
        }
    }
    d->idx += instr.length;
    d->idx %= d->size;
    return instr;
}
