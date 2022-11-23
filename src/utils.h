#ifndef UTILS_H
#define UTILS_H

#define bool uint8_t
#define true 1
#define false 0
#include <stdint.h>

#define UNIMPLEMENTED(fn)                                                                          \
    {                                                                                              \
        fflush(stdout);                                                                            \
        fprintf(stderr, "Unimplemented function: %s\n", (fn));                                     \
        abort();                                                                                   \
    }

void PANIC(char *msg, ...);
void panic_handler(int sig);
void sdl_panic();
void LOG(char *name, char *msg, ...);

#endif
