#include "instruction.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define INSTR_MAX_LEN 50
#define ARG_MAX_LEN 20

uint8_t get_payload_size_argument_t(argument_t *arg) {
    switch (arg->e) {
    case io_offset_u8_e:
    case imm_u8_e:
    case imm_i8_e:
    case sp_offset_e:
        return 1;
    case imm_u16_e:
    case imm_u16_ptr_e:
        return 2;
    default:
        return 0;
    }
}

char *print_argument_t(argument_t *arg) {
    char *str = malloc(ARG_MAX_LEN);
    switch (arg->e) {
    case none_e:
        strncat(str, "", ARG_MAX_LEN - 1);
        break;
    case condition_e:
        switch (arg->p.condition_p) {
        case none_condition_e:
            strncat(str, "", ARG_MAX_LEN - 1);
            break;
        case nzero_condition_e:
            strncat(str, "nz", ARG_MAX_LEN - 1);
            break;
        case zero_condition_e:
            strncat(str, "z", ARG_MAX_LEN - 1);
            break;
        case ncarry_condition_e:
            strncat(str, "nc", ARG_MAX_LEN - 1);
            break;
        case carry_condition_e:
            strncat(str, "c", ARG_MAX_LEN - 1);
            break;
        }
        break;
    case register_e:
        switch (arg->p.register_p) {
        case a_register_p:
            strncat(str, "a", ARG_MAX_LEN - 1);
            break;
        case f_register_p:
            strncat(str, "f", ARG_MAX_LEN - 1);
            break;
        case b_register_p:
            strncat(str, "b", ARG_MAX_LEN - 1);
            break;
        case c_register_p:
            strncat(str, "c", ARG_MAX_LEN - 1);
            break;
        case d_register_p:
            strncat(str, "d", ARG_MAX_LEN - 1);
            break;
        case e_register_p:
            strncat(str, "e", ARG_MAX_LEN - 1);
            break;
        case h_register_p:
            strncat(str, "h", ARG_MAX_LEN - 1);
            break;
        case l_register_p:
            strncat(str, "l", ARG_MAX_LEN - 1);
            break;
        case af_register_p:
            strncat(str, "af", ARG_MAX_LEN - 1);
            break;
        case bc_register_p:
            strncat(str, "bc", ARG_MAX_LEN - 1);
            break;
        case de_register_p:
            strncat(str, "de", ARG_MAX_LEN - 1);
            break;
        case hl_register_p:
            strncat(str, "hl", ARG_MAX_LEN - 1);
            break;
        case sp_register_p:
            strncat(str, "sp", ARG_MAX_LEN - 1);
            break;
        }
        break;
    case io_offset_u8_e:
        snprintf(str, ARG_MAX_LEN - 1, "(0xff00+0x%02x)", arg->p.io_offset_u8_p);
        break;
    case io_offset_c_e:
        strncat(str, "(0xff00+c)", ARG_MAX_LEN - 1);
        break;
    case hl_ptr_e:
        switch (arg->p.hl_ptr_p) {
        case hl_ptr_dec_e:
            strncat(str, "(hl-)", ARG_MAX_LEN - 1);
            break;
        case hl_ptr_inc_e:
            strncat(str, "(hl+)", ARG_MAX_LEN - 1);
            break;
        }
        break;
    case register_ptr_e:
        switch (arg->p.register_ptr_p) {
        case bc_register_ptr_e:
            strncat(str, "(bc)", ARG_MAX_LEN - 1);
            break;
        case de_register_ptr_e:
            strncat(str, "(de)", ARG_MAX_LEN - 1);
            break;
        case hl_register_ptr_e:
            strncat(str, "(hl)", ARG_MAX_LEN - 1);
            break;
        }
        break;
    case imm_u8_e:
    case imm_i8_e:
        snprintf(str, ARG_MAX_LEN - 1, "0x%02x", arg->p.imm_u8_p);
        break;
    case imm_u16_e:
        snprintf(str, ARG_MAX_LEN - 1, "0x%04x", arg->p.imm_u16_p);
        break;
    case imm_u16_ptr_e:
        snprintf(str, ARG_MAX_LEN - 1, "(0x%04x)", arg->p.imm_u16_ptr_p);
        break;
    case fixed_payload_e:
        snprintf(str, ARG_MAX_LEN - 1, "0x%02x", arg->p.fixed_payload_p);
        break;
    case sp_offset_e:
        snprintf(str, ARG_MAX_LEN - 1, "sp+0x%02x", arg->p.imm_u8_p);
        break;
    }
    return str;
}

char *print_instruction(instruction_t *instr) {
    char *str = malloc(INSTR_MAX_LEN);
    char *lhs = print_argument_t(&instr->lhs);
    char *rhs = print_argument_t(&instr->rhs);
    switch (instr->instruction_type) {
    case adc_instruction:
        snprintf(str, INSTR_MAX_LEN - 1, "adc %s, %s", lhs, rhs);
        break;
    case add_instruction:
        snprintf(str, INSTR_MAX_LEN - 1, "add %s, %s", lhs, rhs);
        break;
    case and_instruction:
        snprintf(str, INSTR_MAX_LEN - 1, "and %s, %s", lhs, rhs);
        break;
    case bit_instruction:
        snprintf(str, INSTR_MAX_LEN - 1, "bit %s, %s", lhs, rhs);
        break;
    case call_instruction:
        if (instr->lhs.p.condition_p == none_condition_e)
            snprintf(str, INSTR_MAX_LEN - 1, "call %s", rhs);
        else
            snprintf(str, INSTR_MAX_LEN - 1, "call %s, %s", lhs, rhs);
        break;
    case ccf_instruction:
        strncat(str, "ccf", INSTR_MAX_LEN - 1);
        break;
    case cpl_instruction:
        strncat(str, "cpl", INSTR_MAX_LEN - 1);
        break;
    case cp_instruction:
        snprintf(str, INSTR_MAX_LEN - 1, "cp %s, %s", lhs, rhs);
        break;
    case daa_instruction:
        strncat(str, "daa", INSTR_MAX_LEN - 1);
        break;
    case dec_instruction:
        snprintf(str, INSTR_MAX_LEN - 1, "dec %s", lhs);
        break;
    case di_instruction:
        strncat(str, "di", INSTR_MAX_LEN - 1);
        break;
    case ei_instruction:
        strncat(str, "ei", INSTR_MAX_LEN - 1);
        break;
    case halt_instruction:
        strncat(str, "halt", INSTR_MAX_LEN - 1);
        break;
    case illegal_instruction:
        strncat(str, "illegal", INSTR_MAX_LEN - 1);
        break;
    case inc_instruction:
        snprintf(str, INSTR_MAX_LEN - 1, "inc %s", lhs);
        break;
    case jp_instruction:
        if (instr->lhs.p.condition_p == none_condition_e)
            snprintf(str, INSTR_MAX_LEN - 1, "jp %s", rhs);
        else
            snprintf(str, INSTR_MAX_LEN - 1, "jp %s, %s", lhs, rhs);
        break;
    case jr_instruction:
        if (instr->lhs.p.condition_p == none_condition_e)
            snprintf(str, INSTR_MAX_LEN - 1, "jr %s", rhs);
        else
            snprintf(str, INSTR_MAX_LEN - 1, "jr %s, %s", lhs, rhs);
        break;
    case ld_instruction:
        snprintf(str, INSTR_MAX_LEN - 1, "ld %s, %s", lhs, rhs);
        break;
    case noop_instruction:
        strncat(str, "noop", INSTR_MAX_LEN - 1);
        break;
    case or_instruction:
        snprintf(str, INSTR_MAX_LEN - 1, "or %s, %s", lhs, rhs);
        break;
    case pop_instruction:
        snprintf(str, INSTR_MAX_LEN - 1, "pop %s", lhs);
        break;
    case push_instruction:
        snprintf(str, INSTR_MAX_LEN - 1, "push %s", lhs);
        break;
    case res_instruction:
        snprintf(str, INSTR_MAX_LEN - 1, "res %s, %s", lhs, rhs);
        break;
    case reti_instruction:
        snprintf(str, INSTR_MAX_LEN - 1, "reti");
        break;
    case ret_instruction:
        if (instr->lhs.p.condition_p == none_condition_e)
            snprintf(str, INSTR_MAX_LEN - 1, "ret");
        else
            snprintf(str, INSTR_MAX_LEN - 1, "ret %s", lhs);
        break;
    case rla_instruction:
        strncat(str, "rla", INSTR_MAX_LEN - 1);
        break;
    case rlca_instruction:
        strncat(str, "rlca", INSTR_MAX_LEN - 1);
        break;
    case rlc_instruction:
        snprintf(str, INSTR_MAX_LEN - 1, "rlc %s", lhs);
        break;
    case rl_instruction:
        snprintf(str, INSTR_MAX_LEN - 1, "rl %s", lhs);
        break;
    case rra_instruction:
        strncat(str, "rra", INSTR_MAX_LEN - 1);
        break;
    case rrca_instruction:
        strncat(str, "rrca", INSTR_MAX_LEN - 1);
        break;
    case rrc_instruction:
        snprintf(str, INSTR_MAX_LEN - 1, "rrc %s", lhs);
        break;
    case rr_instruction:
        snprintf(str, INSTR_MAX_LEN - 1, "rr %s", lhs);
        break;
    case rst_instruction:
        snprintf(str, INSTR_MAX_LEN - 1, "rst %s", lhs);
        break;
    case sbc_instruction:
        snprintf(str, INSTR_MAX_LEN - 1, "sbc %s, %s", lhs, rhs);
        break;
    case scf_instruction:
        strncat(str, "scf", INSTR_MAX_LEN - 1);
        break;
    case set_instruction:
        snprintf(str, INSTR_MAX_LEN - 1, "set %s, %s", lhs, rhs);
        break;
    case sla_instruction:
        snprintf(str, INSTR_MAX_LEN - 1, "sla %s", lhs);
        break;
    case sra_instruction:
        snprintf(str, INSTR_MAX_LEN - 1, "sra %s", lhs);
        break;
    case srl_instruction:
        snprintf(str, INSTR_MAX_LEN - 1, "srl %s", lhs);
        break;
    case stop_instruction:
        strncat(str, "stop", INSTR_MAX_LEN - 1);
        break;
    case sub_instruction:
        snprintf(str, INSTR_MAX_LEN - 1, "sub %s, %s", lhs, rhs);
        break;
    case swap_instruction:
        snprintf(str, INSTR_MAX_LEN - 1, "swap %s", lhs);
        break;
    case xor_instruction:
        snprintf(str, INSTR_MAX_LEN - 1, "xor %s, %s", lhs, rhs);
        break;
    }
    free(lhs);
    free(rhs);
    return str;
}
