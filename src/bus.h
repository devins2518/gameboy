#ifndef BUS_H
#define BUS_H

#include "cartridge.h"
#include <stdint.h>

#define BOOTROM_SIZE 0x0100
#define VRAM_SIZE 0x8000
#define VRAM_START 0x8000
#define RAM_SIZE 0x8000
#define RAM_START 0xC000
#define SAT_SIZE 0x00A0
#define SAT_START 0xFE00
#define IO_SIZE 0x0080
#define IO_START 0xFF00
#define HRAM_SIZE 0x0080
#define HRAM_START 0xFF80

typedef struct bus {
    uint8_t bootrom[BOOTROM_SIZE];
    uint8_t vram[VRAM_SIZE];
    uint8_t ram[RAM_START];
    uint8_t sat[SAT_START];
    uint8_t io[IO_SIZE];
    uint8_t hram[HRAM_SIZE];
    uint8_t ie_reg;
    uint8_t _finished_boot;
    cartridge_t cart;
} bus;

bus bus_new(cartridge_t cart);
uint8_t get_address(bus *self, uint16_t addr);
void write_address(bus *self, uint16_t addr, uint8_t n);
void bus_free(bus self);

#endif
