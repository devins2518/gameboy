#include "utils.h"
#include <execinfo.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

void panic_handler(int sig) {
    void *array[10];
    size_t size;

    // get void*'s for all entries on the stack
    size = backtrace(array, 10);

    // print out all the frames to stderr
    fprintf(stderr, "Error: signal %s\n", strsignal(sig));
    backtrace_symbols_fd(array, size, STDERR_FILENO);
    exit(1);
}

const char *ARGUMENT_NAME[] = {
    "a", "f",      "b",       "c",         "d",        "e",  "h",
    "l", "af",     "bc",      "de",        "hl",       "sp", "pc",
    "p", "imm_u8", "imm_u16", "io_offset", "sp_offset"};
