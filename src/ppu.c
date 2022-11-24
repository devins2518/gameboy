#include "ppu.h"
#include "SDL_render.h"

#define HEIGHT 144
#define WIDTH 160

#define CLOCKS_PER_HBLANK 51
#define CLOCKS_PER_DRAW 43
#define CLOCKS_PER_OAM 20
#define CLOCKS_PER_VBLANK (CLOCKS_PER_OAM + CLOCKS_PER_DRAW + CLOCKS_PER_HBLANK)

/* TODO: SDL_SetPaletteColor doesn't work */
const SDL_Color GB_PALETTE[3] = {
    {0xFF, 0xFF, 0xFF, 0xFF},
    {0xCC, 0xCC, 0xCC, 0xFF},
    {0x77, 0x77, 0x77, 0xFF},
};

ppu ppu_new(bus *bus) {
    ppu ppu;
    ppu.bus = bus;
    ppu.lcdc = (void *)bus_read_ptr(bus, 0xFF40);
    ppu.lcds = (void *)bus_read_ptr(bus, 0xFF41);
    ppu.scroll_y = (void *)bus_read_ptr(bus, 0xFF42);
    ppu.scroll_x = (void *)bus_read_ptr(bus, 0xFF43);
    ppu.ly = (void *)bus_read_ptr(bus, 0xFF44);
    ppu.lyc = (void *)bus_read_ptr(bus, 0xFF45);
    ppu.palette = (void *)bus_read_ptr(bus, 0xFF47);
    ppu.window_y = (void *)bus_read_ptr(bus, 0xFF4A);
    ppu.window_x = (void *)bus_read_ptr(bus, 0xFF4B);
    ppu.objs = (void *)bus_read_ptr(bus, SAT_START);
#ifndef DEBUG
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
        sdl_panic();
    if (SDL_CreateWindowAndRenderer(WIDTH, HEIGHT, 0, &ppu.window, &ppu.renderer))
        sdl_panic();
#endif
    ppu.clocks = 0;
    ppu.mode_clocks = 0;

    return ppu;
}

uint8_t ppu_get_color(ppu *ppu, uint16_t addr, uint8_t idx) {
    struct palette *pal = (void *)bus_read_ptr(ppu->bus, addr);
    uint8_t res;
    switch (idx) {
    case 0:
        res = pal->index_0;
        break;
    case 1:
        res = pal->index_1;
        break;
    case 2:
        res = pal->index_2;
        break;
    case 3:
        res = pal->index_3;
        break;
    default:
        res = 0; /* Ignore in case of panic */
        PANIC("invalid color index");
    }
    return res;
}

void ppu_render_obj(ppu *ppu) {
    bool tall = ppu->lcdc->obj_size == tall_obj_size_e;
    uint8_t i;

    for (i = 0; i < 40; i++) {
        struct sprite sprite = ppu->objs[i];
        uint8_t ysize = (tall) ? 16 : 8;

        if ((*ppu->ly >= sprite.ypos) && (*ppu->ly < (sprite.ypos + ysize))) {
            int8_t line = *ppu->ly - sprite.ypos;
            uint16_t obj_addr;
            uint8_t obj_a;
            uint8_t obj_b;
            uint8_t obj_pixel;

            if (sprite.attrs.yflip) {
                line -= ysize;
                line *= -1;
            }

            line *= 2;

            obj_addr = (VRAM_START + (sprite.tile_idx * 16)) + line;
            obj_a = bus_read(ppu->bus, obj_addr);
            obj_b = bus_read(ppu->bus, obj_addr + 1);

            for (obj_pixel = 7; obj_pixel >= 0; obj_pixel--) {
                int8_t color_bit = obj_pixel;
                int8_t color_idx = obj_pixel;
                uint16_t palette_addr;
                uint8_t color;
                int8_t x_pixel;
                uint8_t pixel;

                if (sprite.attrs.xflip) {
                    color_bit -= 7;
                    color_bit *= -1;
                }

                color_idx = (obj_b >> color_bit) & 0x01;
                color_idx <<= 1;
                color_idx |= (obj_a >> color_bit) & 0x01;

                palette_addr = (sprite.attrs.palette == obp0_palette_e) ? 0xFF48 : 0xFF49;

                color = ppu_get_color(ppu, palette_addr, color_idx);

                if (color == 0) {
                    continue;
                }

                x_pixel = 0 - obj_pixel;
                x_pixel += 7;

                pixel = x_pixel + sprite.xpos;
                if ((*ppu->ly < 0) || (*ppu->ly > 143) || (pixel < 0) || (pixel > 159)) {
                    continue;
                }
                SDL_SetRenderDrawColor(ppu->renderer, GB_PALETTE[color].r, GB_PALETTE[color].g,
                                       GB_PALETTE[color].b, GB_PALETTE[color].a);
                SDL_RenderDrawPoint(ppu->renderer, pixel, *ppu->ly);
            }
        }
    }
}

void ppu_render_bg(ppu *ppu) {
    bool use_window = false;
    bool signed_tile;
    uint16_t tile_mem_idx;
    uint16_t bg_mem_idx;
    uint8_t ypos;
    uint16_t tile_row;
    uint8_t pixel;

    if (ppu->lcdc->window_enable && ppu->window_y <= ppu->ly)
        use_window = true;
    if (ppu->lcdc->wdata == first_wdata_e) {
        tile_mem_idx = 0x8800;
        signed_tile = false;
    } else {
        tile_mem_idx = 0x8000;
        signed_tile = true;
    }

    /* Determine which bank to use */
    if (!use_window) {
        if (ppu->lcdc->bgmap == first_bgmap_e)
            bg_mem_idx = 0x9800;
        else
            bg_mem_idx = 0x9C00;
    } else {
        if (ppu->lcdc->wmap == first_wmap_e)
            bg_mem_idx = 0x9800;
        else
            bg_mem_idx = 0x9C00;
    }

    if (!use_window)
        ypos = *ppu->scroll_y + *ppu->ly;
    else
        ypos = *ppu->ly - *ppu->window_y;

    tile_row = (ypos / 8) * 32;

    for (pixel = 0; pixel < WIDTH; pixel++) {
        uint8_t xpos;
        uint16_t tile_column;
        int16_t tile_num;
        uint16_t tile_addr;
        uint16_t tile_loc;
        uint8_t line;
        uint8_t tile_a;
        uint8_t tile_b;
        int8_t color_idx;
        uint8_t palette_idx;
        uint8_t color;

        if ((use_window) && (pixel >= *ppu->window_x))
            xpos = pixel - *ppu->window_x;
        else
            xpos = pixel + *ppu->scroll_x;

        tile_column = xpos / 8;
        tile_addr = bg_mem_idx + tile_row + tile_column;

        if (signed_tile) {
            tile_num = (int16_t)bus_read(ppu->bus, tile_addr);
        }

        tile_loc = tile_mem_idx;

        if (signed_tile)
            tile_loc += (tile_num + 128) * 16;
        else
            tile_loc += tile_num * 16;

        line = ypos % 8;
        line *= 2;

        tile_a = bus_read(ppu->bus, tile_loc + line);
        tile_b = bus_read(ppu->bus, tile_loc + line + 1);

        color_idx = xpos % 8;
        color_idx -= 7;
        color_idx *= -1;

        palette_idx = (tile_b >> color_idx) & 0x01;
        palette_idx <<= 1;
        palette_idx |= (tile_a >> color_idx) & 0x01;

        color = ppu_get_color(ppu, 0xFF47, palette_idx);

        if ((*ppu->ly < 0) || (*ppu->ly > 143) || (pixel < 0) || (pixel > 159)) {
            continue;
        }
        SDL_SetRenderDrawColor(ppu->renderer, GB_PALETTE[color].r, GB_PALETTE[color].g,
                               GB_PALETTE[color].b, GB_PALETTE[color].a);
        SDL_RenderDrawPoint(ppu->renderer, pixel, pixel);
    }

    UNIMPLEMENTED("ppu_render_bg")
}

void ppu_draw_scanline(ppu *ppu) {
    /* LOG("PPU", "Drawing scanline"); */
    /* LOG("PPU", "BG Enabled: %d", ppu->lcdc->bg_win_enable); */
    if (ppu->lcdc->bg_win_enable)
        ppu_render_bg(ppu);
    /* LOG("PPU", "OBJ Enabled: %d", ppu->lcdc->obj_enable); */
    if (ppu->lcdc->obj_enable)
        ppu_render_obj(ppu);
}

uintptr_t ppu_clock(ppu *ppu) {
    uintptr_t old_clocks = ppu->clocks++;
    ppu->mode_clocks++;
    switch (ppu->lcds->state) {
    case oam_state_e:
        if (ppu->mode_clocks >= CLOCKS_PER_OAM) {
            ppu->mode_clocks %= CLOCKS_PER_OAM;
            ppu->lcds->state = draw_state_e;
            /* LOG("PPU", "Switched to draw state from oam"); */
        }
        break;
    case hblank_state_e:
        if (ppu->mode_clocks >= CLOCKS_PER_HBLANK) {
            ppu->mode_clocks %= CLOCKS_PER_HBLANK;
            (*ppu->ly)++;
            if (*ppu->ly == HEIGHT - 1) {
                ppu->lcds->state = vblank_state_e;
                /* LOG("PPU", "Switched to vblank state from hblank"); */
                SDL_RenderPresent(ppu->renderer);
            } else {
                ppu->lcds->state = oam_state_e;
                /* LOG("PPU", "Switched to oam state from hblank"); */
            }
        }
        break;
    case vblank_state_e:
        if (ppu->mode_clocks >= CLOCKS_PER_VBLANK) {
            ppu->mode_clocks %= CLOCKS_PER_VBLANK;
            (*ppu->ly)++;

            if (*ppu->ly > 153) {
                ppu->lcds->state = oam_state_e;
                *ppu->ly = 0;
                /* LOG("PPU", "Switched to oam state from vblank"); */
            }
        }
        break;
    case draw_state_e:
        if (ppu->mode_clocks >= CLOCKS_PER_DRAW) {
            ppu->mode_clocks %= CLOCKS_PER_DRAW;
            ppu->lcds->state = hblank_state_e;
            /* LOG("PPU", "Switched to hblank state from draw"); */

            ppu_draw_scanline(ppu);
        }
        break;
    }
    return ppu->clocks - old_clocks;
}

void ppu_free(ppu ppu) {
    SDL_DestroyRenderer(ppu.renderer);
    SDL_DestroyWindow(ppu.window);
    SDL_Quit();
}
