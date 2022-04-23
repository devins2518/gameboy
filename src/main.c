#include "gamegirl.h"
#include "utils.h"
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main(int argc, char **argv) {
    gamegirl *gg;
    struct timespec req;
    char *path;
    SDL_Event e;
    bool quit = FALSE;

    signal(SIGSEGV, panic_handler);
    signal(SIGABRT, panic_handler);

    if (argc > 0) {
        path = argv[1];
    } else {
        path = NULL;
    }
    gg = gamegirl_init(path);

    req.tv_sec = 0;
    req.tv_nsec = 16670000;
    while (!quit) {
        while (SDL_PollEvent(&e)) {
            switch (e.type) {
            case SDL_KEYDOWN:
                switch (e.key.keysym.scancode) {
                case SDL_SCANCODE_J:
                    if (gg->step)
                        gamegirl_clock(gg);
                    break;
                case SDL_SCANCODE_G:
                    gg->step = !gg->step;
                    break;
                default:
                    break;
                }
                break;
            case SDL_QUIT:
                quit = TRUE;
                break;
            default:
                break;
            }
        }
        if (!gg->step) {
            gamegirl_clock(gg);
        }
        nanosleep(&req, NULL);
        /* printf("gg.clocks %d\n", gg.cpu.clocks); */
    }

    return 0;
}
