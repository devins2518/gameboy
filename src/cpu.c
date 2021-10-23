#include "cpu.h"
#include "utils.h"

cpu cpu_init() {
    cpu c = {
        .af = {0},
        .bc = {0},
        .de = {0},
        .hl = {0},
        .pc = 0x0000,
        .sp = 0xFFFE,
    };
    return c;
}
