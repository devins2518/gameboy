#ifndef BUS_H
#define BUS_H

#include <stdint.h>

#define BOOTROM_SIZE 0x0100

typedef struct bus {
    uint8_t bootrom[BOOTROM_SIZE];
} bus;

bus bus_init();
uint8_t get_address(bus *self, uint16_t addr);

#endif
