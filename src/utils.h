#ifndef UTILS_H
#define UTILS_H

#define TRUE 1
#define FALSE 0

#define UNIMPLEMENTED(fn)                                                      \
    fflush(stdout);                                                            \
    fprintf(stderr, "Unimplemented function: %s\n", (fn));                     \
    abort();

void panic_handler(int sig);

typedef enum argument_t {
    A,
    F,
    B,
    C,
    D,
    E,
    H,
    L,
    AF,
    BC,
    DE,
    HL,
    SP,
    PC
} argument_t;

#endif
