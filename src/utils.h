#ifndef UTILS_H
#define UTILS_H

#define bool int
#define true 1
#define false 0
#include <stdint.h>

#define UNIMPLEMENTED(fn)                                                                          \
    {                                                                                              \
        fflush(stdout);                                                                            \
        fprintf(stderr, "Unimplemented function: %s\n", (fn));                                     \
        abort();                                                                                   \
    }
#define PANIC(str)                                                                                 \
    {                                                                                              \
        fflush(stdout);                                                                            \
        fprintf(stderr, "%s\n", (str));                                                            \
        abort();                                                                                   \
    }

void panic_handler(int sig);
void sdl_panic();

#endif
