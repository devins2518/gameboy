#ifndef UTILS_H
#define UTILS_H

#include <stdint.h>
#define TRUE 1
#define FALSE 0

#define UNIMPLEMENTED(fn)                                                      \
    fflush(stdout);                                                            \
    fprintf(stderr, "Unimplemented function: %s\n", (fn));                     \
    abort();

void panic_handler(int sig);

/* typedef enum t_argument {
    a,
    f,
    b,
    c,
    d,
    e,
    h,
    l,
    af,
    bc,
    de,
    hl,
    sp,
    pc,
    p,
    imm_u8,
    imm_u16,
    io_offset,
    sp_offset,
} t_argument; */

// Caller is responsible for ensuring that payload does not contain invalid
// data. Only valid values with payloads is pointer, immediate values, IO
// offset, and stack pointer offset. CALLING WITH OTHER DISCRIMINANTS WILL
// RETURN GARBAGE.
typedef struct argument_t {
    enum t_argument {
        a,         /* A Register */
        f,         /* F Register */
        b,         /* B Register */
        c,         /* C Register */
        d,         /* D Register */
        e,         /* E Register */
        h,         /* H Register */
        l,         /* L Register */
        af,        /* AF Register */
        bc,        /* BC Register */
        de,        /* DE Register */
        hl,        /* HL Register */
        sp,        /* SP Register */
        pc,        /* PC Register */
        p,         /* Pointer */
        imm_u8,    /* Immediate 8 bit value */
        imm_u16,   /* Immediate 16 bit value */
        io_offset, /* Offset from IO base address 0xFF00 */
        sp_offset, /* Offset from stack pointer address */
    } type;
    uint16_t payload;
} argument_t;

#endif
