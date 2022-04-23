#include "ppu.h"

#define HEIGHT 144
#define WIDTH 160

ppu ppu_new(bus *bus) {
    ppu pp;
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
        sdl_panic();
    pp.bus = bus;
    pp.lcdc = (void *)get_address_ptr(bus, 0xFF40);
    pp.lcds = (void *)get_address_ptr(bus, 0xFF41);
    if (SDL_CreateWindowAndRenderer(WIDTH, HEIGHT, 0, &pp.window, &pp.renderer))
        sdl_panic();

    return pp;
}

void ppu_free(ppu ppu) {
    SDL_DestroyRenderer(ppu.renderer);
    SDL_DestroyWindow(ppu.window);
    SDL_Quit();
}
