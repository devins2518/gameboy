#include "cpu.h"
#include <stdio.h>

int main(int argc, char **argv) {
    (void)argv;
    cpu c = cpu_init();
    if (argc != 1) {
        printf("%d takes no arguments.\n", c.i);
        return 1;
    }
    /* printf("This is project %s.\n", PROJECT_NAME); */
    return 0;
}
