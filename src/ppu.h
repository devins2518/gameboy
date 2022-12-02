#ifndef PPU_H
#define PPU_H

#include "bus.h"
#include "utils.h"
#include <SDL.h>

typedef struct {
    bus *bus;
    struct __attribute__((packed)) {
        bool bg_win_enable : 1;
        bool obj_enable : 1;
        enum {
            short_obj_size_e, /* 8x8 */
            tall_obj_size_e   /* 8x16 */
        } obj_size : 1;
        enum {
            first_bgmap_e, /* 0x9800 - 0x9BFF */
            sec_bgmap_e    /* 0x9C00 - 0x9FFF */
        } bgmap : 1;
        enum {
            first_wdata_e, /* 0x8800 - 0x97FF */
            sec_wdata_e    /* 0x8000 - 0x8FFF */
        } wdata : 1;
        bool window_enable : 1;
        enum {
            first_wmap_e, /* 0x9800 - 0x9BFF */
            sec_wmap_e    /* 0x9C00 - 0x9FFF */
        } wmap : 1;
        bool enable : 1;
    } * lcdc;
    struct __attribute((packed)) {
        enum { hblank_state_e, vblank_state_e, oam_state_e, draw_state_e } state : 2;
        uint8_t lyc_eq_ly : 1;
        uint8_t mode0_int : 1;
        uint8_t mode1_int : 1;
        uint8_t mode2_int : 1;
        uint8_t lyc_eq_ly_int : 1;
        uint8_t _ : 1;
    } * lcds;
    struct __attribute((packed)) palette {
        uint8_t index_0 : 2;
        uint8_t index_1 : 2;
        uint8_t index_2 : 2;
        uint8_t index_3 : 2;
    } * palette;
    uint8_t *scroll_y;
    uint8_t *scroll_x;
    uint8_t *ly;
    uint8_t *lyc;
    uint8_t *window_y;
    uint8_t *window_x;
    struct __attribute((packed)) sprite {
#if BIG_ENDIAN
        uint8_t ypos;
        uint8_t xpos;
        uint8_t tile_idx;
        struct __attribute((packed)) {
            uint8_t bg_over : 1;
            uint8_t yflip : 1;
            uint8_t xflip : 1;
            enum {
                obp0_palette_e, /* 0xFF48 */
                obp1_palette_e  /* 0xFF49 */
            } palette : 1;
            uint8_t _ : 4;
        } attrs;
#else
        struct __attribute((packed)) {
            uint8_t bg_over : 1;
            uint8_t yflip : 1;
            uint8_t xflip : 1;
            enum {
                obp0_palette_e, /* 0xFF48 */
                obp1_palette_e  /* 0xFF49 */
            } palette : 1;
            uint8_t _ : 4;
        } attrs;
        uint8_t tile_idx;
        uint8_t xpos;
        uint8_t ypos;
#endif
    } * objs;
    SDL_Window *window;
    SDL_Renderer *renderer;
    uintptr_t clocks;
    uintptr_t mode_clocks;
} ppu;

ppu ppu_new(bus *bus);
uintptr_t ppu_clock(ppu *ppu);
void ppu_free(ppu ppu);
#endif
