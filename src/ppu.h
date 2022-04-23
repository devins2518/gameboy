#ifndef PPU_H
#define PPU_H

#include "bus.h"
#include "utils.h"
#include <SDL.h>

typedef struct {
    bus *bus;
    struct __attribute__((packed)) {
        bool enable : 1;
        enum {
            first_wmap_e, /* 0x9800 - 0x9BFF */
            sec_wmap_e    /* 0x9C00 - 0x9FFF */
        } wmap : 1;
        bool window_enable : 1;
        enum {
            first_wdata_e, /* 0x8800 - 0x97FF */
            sec_wdata_e    /* 0x8000 - 0x8FFF */
        } wdata : 1;
        enum {
            first_bgmap_e, /* 0x9800 - 0x9BFF */
            sec_bgmap_e    /* 0x9C00 - 0x9FFF */
        } bgmap : 1;
        enum {
            short_obj_size_e, /* 8x8 */
            tall_obj_size_e   /* 8x16 */
        } obj_size : 1;
        bool obj_enable : 1;
        bool bg_win_enable : 1;
    } * lcdc;
    struct __attribute((packed)) {
        uint8_t _ : 1;
        uint8_t lyc_eq_ly_int : 1;
        uint8_t mode2_int : 1;
        uint8_t mode1_int : 1;
        uint8_t mode0_int : 1;
        uint8_t lyc_eq_ly : 1;
        enum { hblank_state_e, vblank_state_e, oam_state_e, draw_state_e } state : 1;
    } * lcds;
    SDL_Window *window;
    SDL_Renderer *renderer;
} ppu;

ppu ppu_new(bus *bus);
void ppu_free(ppu ppu);
#endif
