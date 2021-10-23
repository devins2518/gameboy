#ifndef UTILS_H
#define UTILS_H

#include <stdint.h>

typedef struct __attribute__((packed)) reg {
    uint8_t lo;
    uint8_t hi;
} reg;

typedef struct __attribute__((packed)) f {
    unsigned _ : 4;
    unsigned c : 1;
    unsigned n : 1;
    unsigned h : 1;
    unsigned z : 1;
} f;

typedef struct __attribute__((packed)) afreg {
    uint8_t a;
    f f;
} afreg;

#endif
