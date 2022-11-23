#ifndef CPU_H
#define CPU_H

#include "bus.h"
#include "utils.h"
#include <stdint.h>
#ifdef __APPLE__
#include <machine/endian.h>
#elif __linux__
#include <endian.h>
#endif

typedef struct cpu {
    union __attribute((packed)) {
        struct __attribute((packed)) {
            uint8_t a;
            union {
                struct __attribute((packed)) {
                    uint8_t z : 1;
                    uint8_t n : 1;
                    uint8_t h : 1;
                    uint8_t c : 1;
                    uint8_t _ : 4;
                } bits;
                uint8_t u8;
            } f;
        } u8;
        uint16_t u16;
    } af;
    union __attribute((packed)) {
        struct __attribute((packed)) {
#if BIG_ENDIAN == 1
            uint8_t b;
            uint8_t c;
#else
            uint8_t c;
            uint8_t b;
#endif
        } u8;
        uint16_t u16;
    } bc;
    union __attribute((packed)) {
        struct __attribute((packed)) {
#if BIG_ENDIAN == 1
            uint8_t d;
            uint8_t e;
#else
            uint8_t e;
            uint8_t d;
#endif
        } u8;
        uint16_t u16;
    } de;
    union __attribute((packed)) {
        struct __attribute((packed)) {
#if BIG_ENDIAN == 1
            uint8_t h;
            uint8_t l;
#else
            uint8_t l;
            uint8_t h;
#endif
        } u8;
        uint16_t u16;
    } hl;
    uint16_t pc;
    uint16_t sp;
    enum { cpu_running_mode_e, cpu_halted_mode_e, cpu_stop_mode_e } mode;
    bus *bus;
    uintptr_t clocks;
} cpu;

cpu cpu_new(bus *bus);

uintptr_t cpu_clock(cpu *self);

uint16_t get_sp(cpu *self);
uint8_t cpu_get_imm_u8(cpu *self);
uint16_t cpu_get_imm_u16(cpu *self);
uint8_t get_flag_z(cpu *self);
uint8_t get_flag_n(cpu *self);
uint8_t get_flag_h(cpu *self);
uint8_t get_flag_c(cpu *self);

#endif
