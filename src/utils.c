#include "utils.h"
#include <SDL.h>
#include <execinfo.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

const char *siglist[] = {
    "SIGHUP",  "SIGINT",    "SIGQUIT", "SIGILL",  "SIGTRAP",   "SIGABRT",  "SIGIOT",   "SIGBUS",
    "SIGEMT",  "SIGFPE",    "SIGKILL", "SIGUSR1", "SIGSEGV",   "SIGUSR2",  "SIGPIPE",  "SIGALRM",
    "SIGTERM", "SIGSTKFLT", "SIGCHLD", "SIGCLD",  "SIGCONT",   "SIGSTOP",  "SIGTSTP",  "SIGTTIN",
    "SIGTTOU", "SIGURG",    "SIGXCPU", "SIGXFSZ", "SIGVTALRM", "SIGPROF",  "SIGWINCH", "SIGIO",
    "SIGPOLL", "SIGPWR",    "SIGINFO", "SIGLOST", "SIGSYS",    "SIGUNUSED"};

void panic_handler(int sig) {
    void *array[10];
    size_t size;

    /* get void*'s for all entries on the stack */
    size = backtrace(array, 10);

    /* print out all the frames to stderr */
    fprintf(stderr, "Error: signal %s\n", siglist[sig - 1]);
    backtrace_symbols_fd(array, size, STDERR_FILENO);
    exit(EXIT_FAILURE);
}

void sdl_panic() {
    printf("SDL ERROR: %s", SDL_GetError());
    exit(EXIT_FAILURE);
}

const char *ARGUMENT_NAME[] = {"a", "f",      "b",       "c",         "d",        "e",  "h",
                               "l", "af",     "bc",      "de",        "hl",       "sp", "pc",
                               "p", "imm_u8", "imm_u16", "io_offset", "sp_offset"};
