#include "gamegirl.h"
#include "utils.h"
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define FRAMES_PER_SEC 60
#define CYCLES_PER_SEC 4194304
#define CYCLES_PER_FRAME (CYCLES_PER_SEC / FRAMES_PER_SEC)

int main(int argc, char **argv) {
    gamegirl *gg;
    struct timespec req;
    char *path;
    SDL_Event e;
    bool quit = false;

    signal(SIGSEGV, panic_handler);
    signal(SIGABRT, panic_handler);

    if (argc > 0) {
        path = argv[1];
    } else {
        path = NULL;
    }
    gg = gamegirl_init(path);

    req.tv_sec = 0;
    req.tv_nsec = CYCLES_PER_FRAME;
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
                quit = true;
                break;
            default:
                break;
            }
        }
        if (!gg->step) {
            gamegirl_clock(gg);
        }
        nanosleep(&req, NULL);
    }

    return 0;
}
