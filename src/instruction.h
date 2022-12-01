#ifndef INSTRUCTION_H
#define INSTRUCTION_H
#include "utils.h"

typedef struct {
    union {
        enum {
            none_condition_e,
            nzero_condition_e,
            zero_condition_e,
            ncarry_condition_e,
            carry_condition_e
        } condition_p;
        enum {
            a_register_p,
            f_register_p,
            b_register_p,
            c_register_p,
            d_register_p,
            e_register_p,
            h_register_p,
            l_register_p,
            af_register_p,
            bc_register_p,
            de_register_p,
            hl_register_p,
            sp_register_p
        } register_p;
        uint8_t io_offset_u8_p;
        enum { hl_ptr_inc_e, hl_ptr_dec_e } hl_ptr_p;
        enum { bc_register_ptr_e, de_register_ptr_e, hl_register_ptr_e } register_ptr_p;
        uint8_t none_p;
        uint8_t imm_u8_p;
        int8_t imm_i8_p;
        uint16_t imm_u16_p;
        uint16_t imm_u16_ptr_p;
        uint8_t fixed_payload_p;
        int8_t sp_offset_p;
    } p;
    enum {
        none_e,
        condition_e,
        register_e,
        io_offset_u8_e,
        io_offset_c_e,
        hl_ptr_e,
        register_ptr_e,
        imm_u8_e,
        imm_i8_e,
        imm_u16_e,
        imm_u16_ptr_e,
        fixed_payload_e,
        sp_offset_e
    } e;
} argument_t;

uint8_t get_payload_size_argument_t(argument_t *arg);
uint8_t get_raw_size_argument_t(argument_t *arg);

typedef struct {
    enum {
        adc_instruction,
        add_instruction,
        and_instruction,
        bit_instruction,
        call_instruction,
        ccf_instruction,
        cpl_instruction,
        cp_instruction,
        daa_instruction,
        dec_instruction,
        di_instruction,
        ei_instruction,
        halt_instruction,
        illegal_instruction,
        inc_instruction,
        jp_instruction,
        jr_instruction,
        ld_instruction,
        noop_instruction,
        or_instruction,
        pop_instruction,
        push_instruction,
        res_instruction,
        reti_instruction,
        ret_instruction,
        rla_instruction,
        rlca_instruction,
        rlc_instruction,
        rl_instruction,
        rra_instruction,
        rrca_instruction,
        rrc_instruction,
        rr_instruction,
        rst_instruction,
        sbc_instruction,
        scf_instruction,
        set_instruction,
        sla_instruction,
        sra_instruction,
        srl_instruction,
        stop_instruction,
        sub_instruction,
        swap_instruction,
        xor_instruction
    } instruction_type;
    argument_t lhs;
    argument_t rhs;
    uint8_t length;
    uint8_t clocks;
} instruction_t;

/* Caller is responsible for freeing memory after call. */
char *print_instruction(instruction_t *instr);

#endif
