#ifndef CARTRIDGE_H
#define CARTRIDGE_H

#include <stddef.h>
#include <stdint.h>

typedef struct cartridge_t {
    uint8_t *data;
    size_t size;
} cartridge_t;

cartridge_t cartridge_new(char *path);
void cartridge_free(cartridge_t self);

#endif
