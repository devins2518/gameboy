#ifndef CARTRIDGE_H
#define CARTRIDGE_H

#include <stddef.h>
#include <stdint.h>

typedef struct cartridge_t {
    uint8_t *data;
    size_t size;
    char *path;
} cartridge_t;

cartridge_t cartridge_new(char *path);
void cartridge_free(cartridge_t self);
uint8_t cartridge_read(cartridge_t *self, uint16_t addr);
void cartridge_write(cartridge_t *self, uint16_t addr, uint8_t n);

#endif
