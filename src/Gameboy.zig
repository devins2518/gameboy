const std = @import("std");
const main = @import("main.zig");
const Bus = @import("Bus.zig");
const Cpu = @import("Cpu.zig");
const Ppu = @import("Ppu.zig");
const Cartridge = @import("Cartridge.zig");
const utils = @import("utils.zig");
const bufw = std.io.bufferedWriter;
const Self = @This();
const SDL = @import("sdl2");
const sdl_native = SDL.c;

auto: bool = false,
clocks: u64 = 0,
bus: Bus,
ppu: Ppu,
cpu: Cpu,

var writer = bufw(std.io.getStdOut().writer());

pub fn init(cart: Cartridge) !Self {
    var bus = Bus.init(cart);
    var ppu = try Ppu.init(&bus);
    var cpu = Cpu.init(&bus, &ppu, &writer);
    return Self{
        .bus = bus,
        .ppu = ppu,
        .cpu = cpu,
    };
}

pub fn deinit(self: Self) void {
    self.bus.deinit();
    self.ppu.deinit();
    SDL.quit();
}

fn step(self: *Self) void {
    self.cpu.clock() catch {};
    self.ppu.clock() catch {};
    self.clocks += 1;
}

pub fn run(self: *Self) !void {
    mainLoop: while (true) {
        while (SDL.pollEvent()) |ev| {
            switch (ev) {
                .quit => break :mainLoop,
                .key_down => {
                    switch (ev.key_down.keysym.scancode) {
                        sdl_native.SDL_SCANCODE_A => self.auto = !self.auto,
                        sdl_native.SDL_SCANCODE_G => self.step(),
                        else => {},
                    }
                },
                else => {},
            }
        }

        // if self.auto && frame_delta > clock_interval
        if (self.auto)
            self.step();
    }
}

test {
    std.testing.refAllDecls(@This());
}
