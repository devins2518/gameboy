#include "cartridge.h"
#include "utils.h"
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>

cartridge_t cartridge_new(char *path) {
    cartridge_t c;
    int fd;
    struct stat s;

    fd = open(path, O_RDONLY);
    fstat(fd, &s);
    c.data = mmap(NULL, s.st_size, PROT_WRITE, MAP_PRIVATE, fd, 0);
    if (c.data == MAP_FAILED)
        PANIC("mmap failed");

    return c;
}

void cartridge_free(cartridge_t self) { munmap(self.data, self.size); }
