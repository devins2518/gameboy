#include "cartridge.h"
#include "rom.h"
#include "utils.h"
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>

cartridge_t cartridge_new(char *path) {
    cartridge_t c;

    c.path = path;
    /* Use embedded file */
    if (path == NULL) {
        c.size = rom_len;
        c.data = malloc(c.size);
        if (c.data == NULL)
            PANIC("mapping rom file failed");
        memcpy(c.data, rom_gb, c.size);
    } else {
        int fd;
        struct stat s;

        fd = open(path, O_RDONLY);
        fstat(fd, &s);
        c.data = mmap(NULL, s.st_size, PROT_WRITE, MAP_PRIVATE, fd, 0);
        c.size = s.st_size;
        if (c.data == MAP_FAILED)
            PANIC("mmap failed");
    }

    return c;
}

void cartridge_free(cartridge_t self) {
    if (self.path == NULL)
        free(self.data);
    else
        munmap(self.data, self.size);
}

uint8_t get_cartridge_addr(cartridge_t *self, uint16_t addr) {
    return self->data[addr];
}
