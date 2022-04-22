#ifndef UTILS_H
#define UTILS_H

#include <stdint.h>
#define TRUE 1
#define FALSE 0

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

#endif
