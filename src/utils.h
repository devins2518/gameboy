#ifndef UTILS_H
#define UTILS_H

#define TRUE 1
#define FALSE 0

void p();

typedef enum register_t {
    A,
    F,
    B,
    C,
    D,
    E,
    H,
    L,
    AF,
    BC,
    DE,
    HL,
    SP,
    PC
} register_t;

#endif
