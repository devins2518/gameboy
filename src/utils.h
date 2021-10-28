#ifndef UTILS_H
#define UTILS_H

#include <stdlib.h>

#define TRUE 1
#define FALSE 0

#define PANIC() abort()

enum register_t {
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
    PC,
};

#endif
