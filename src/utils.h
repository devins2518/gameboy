#ifndef UTILS_H
#define UTILS_H

#include <stdint.h>
#define TRUE 1
#define FALSE 0

#define UNIMPLEMENTED(fn)                                                      \
    {                                                                          \
        fflush(stdout);                                                        \
        fprintf(stderr, "Unimplemented function: %s\n", (fn));                 \
        abort();                                                               \
    }
#define PANIC(str)                                                             \
    {                                                                          \
        fflush(stdout);                                                        \
        fprintf(stderr, "%s\n", (str));                                        \
        abort();                                                               \
    }

void panic_handler(int sig);

/* Caller is responsible for ensuring that payload does not contain invalid
   data. Only valid values with payloads is pointer, immediate values, IO
   offset, and stack pointer offset. CALLING WITH OTHER DISCRIMINANTS WILL
   RETURN GARBAGE. */
typedef struct argument_t {
    enum type {
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
        sp_offset  /* Offset from stack pointer address */
    } type;
    uint16_t payload;
    enum cond { zero, nzero, carry, ncarry, none } cond;
} argument_t;

const char *ARGUMENT_NAME[19];

#endif
