#ifndef DECODER_H
#define DECODER_H
#include "instruction.h"
#include <stdint.h>

typedef struct {
    uintptr_t idx;
    const uint8_t *arr;
    uintptr_t size;
} decoder_t;

decoder_t decoder_new(const uint8_t *arr, uintptr_t size);
instruction_t decoder_next(decoder_t *d);

#endif
