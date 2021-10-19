const std = @import("std");
const io = std.io;
const Bus = @import("Bus.zig");
const Cartridge = @import("Cartridge.zig");
const Cpu = @import("Cpu.zig");
const Ppu = @import("Ppu.zig");
const Gameboy = @import("Gameboy.zig");
const SDL = @import("sdl2");
const sdl_native = SDL.c;

pub fn main() !void {
    var gb = Gameboy.init();
    defer gb.deinit();

    try SDL.init(.{
        .video = true,
        .events = true,
        .audio = true,
    });
    defer SDL.quit();

    var window = try SDL.createWindow(
        "Ziggyboy",
        .{ .centered = {} },
        .{ .centered = {} },
        640,
        480,
        .{ .shown = true },
    );
    defer window.destroy();

    var renderer = try SDL.createRenderer(window, null, .{ .accelerated = true });
    defer renderer.destroy();

    mainLoop: while (true) {
        while (SDL.pollEvent()) |ev| {
            switch (ev) {
                .quit => break :mainLoop,
                .key_down => {
                    switch (ev.key_down.keysym.scancode) {
                        sdl_native.SDL_SCANCODE_A => gb.auto = !gb.auto,
                        sdl_native.SDL_SCANCODE_G => gb.step(),
                        else => {},
                    }
                },
                else => {},
            }
        }

        try renderer.setColorRGB(0xF7, 0xA4, 0x1D);
        try renderer.clear();

        // if self.auto && frame_delta > clock_interval {
        if (gb.auto)
            gb.step();

        renderer.present();
    }
}

test {
    std.testing.refAllDecls(@This());
}

// ----- time start
// |---- init stuff
// |---- cpu.clock()
// |---- fetch_opcode();
// |---- suspend for 1 byte read
// |
// |
// |----
