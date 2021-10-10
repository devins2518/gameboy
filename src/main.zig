const std = @import("std");
const Cpu = @import("Cpu.zig");
const Bus = @import("Bus.zig");
const Ppu = @import("Ppu.zig");
const Cartridge = @import("Cartridge.zig");
const SDL = @import("sdl2");
const sdl_native = SDL.c;

var arena = std.heap.ArenaAllocator.init(std.heap.page_allocator);
const blargg = getPath();

pub fn main() !void {
    var args = std.process.args();
    _ = args.skip();
    const path = if (args.next(arena.child_allocator)) |file|
        file catch blargg
    else
        blargg;

    std.log.info("{s}", .{path});
    var cart = Cartridge.init(path, arena.child_allocator) catch unreachable;
    defer cart.deinit();
    var bus = Bus.init(&cart);
    var ppu = Ppu.init();
    var cpu = Cpu.init(&bus, &ppu);

    var state = struct {
        auto: bool = false,
        clocks: u64 = 0,
        bus: Bus,
        ppu: Ppu,
        cpu: Cpu,

        fn step(self: *@This()) void {
            self.cpu.clock();
            self.clocks += 1;
        }
    }{ .bus = bus, .ppu = ppu, .cpu = cpu };

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
                        sdl_native.SDL_SCANCODE_A => state.auto = true,
                        sdl_native.SDL_SCANCODE_G => state.step(),
                        else => {},
                    }
                },
                else => {},
            }
        }

        try renderer.setColorRGB(0xF7, 0xA4, 0x1D);
        try renderer.clear();

        // if self.auto && frame_delta > clock_interval {
        if (state.auto)
            state.step();

        renderer.present();
    }

    arena.deinit();
}

pub fn getPath() []const u8 {
    comptime {
        const dn = std.fs.path.dirname;
        const root = @src().file;

        return dn(dn(root).?).? ++ "/roms/blargg/cpu_instrs/cpu_instrs.gb";
    }
}

fn sdlPanic() noreturn {
    const str = @as(?[*:0]const u8, SDL.SDL_GetError()) orelse "unknown error";
    @panic(std.mem.sliceTo(str, 0));
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
