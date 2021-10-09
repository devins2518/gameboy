const std = @import("std");
const Cpu = @import("Cpu.zig");
const Bus = @import("Bus.zig");
const Ppu = @import("Ppu.zig");
const Cartridge = @import("Cartridge.zig");
const SDL = @import("sdl2");

var arena = std.heap.ArenaAllocator.init(std.heap.page_allocator);
const blargg = getPath();

pub fn main() void {
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
            nosuspend self.cpu.clock();
            self.clocks += 1;
        }
    }{ .bus = bus, .ppu = ppu, .cpu = cpu };

    if (SDL.SDL_Init(SDL.SDL_INIT_VIDEO | SDL.SDL_INIT_EVENTS | SDL.SDL_INIT_AUDIO) < 0)
        sdlPanic();
    defer SDL.SDL_Quit();

    var window = SDL.SDL_CreateWindow(
        "ziggyboy",
        SDL.SDL_WINDOWPOS_CENTERED,
        SDL.SDL_WINDOWPOS_CENTERED,
        640,
        480,
        SDL.SDL_WINDOW_SHOWN,
    ) orelse sdlPanic();
    defer _ = SDL.SDL_DestroyWindow(window);

    var renderer = SDL.SDL_CreateRenderer(window, -1, SDL.SDL_RENDERER_ACCELERATED) orelse sdlPanic();
    defer _ = SDL.SDL_DestroyRenderer(renderer);

    mainLoop: while (true) {
        var ev: SDL.SDL_Event = undefined;
        while (SDL.SDL_PollEvent(&ev) != 0) {
            if ((ev.type == SDL.SDL_QUIT) or (ev.key.keysym.scancode == SDL.SDL_SCANCODE_ESCAPE))
                break :mainLoop;
            if (ev.key.keysym.scancode == SDL.SDL_SCANCODE_A)
                state.auto = true;
            if (ev.key.keysym.scancode == SDL.SDL_SCANCODE_G)
                state.step();
        }

        _ = SDL.SDL_SetRenderDrawColor(renderer, 0xF7, 0xA4, 0x1D, 0xFF);
        _ = SDL.SDL_RenderClear(renderer);

        // if self.auto && frame_delta > clock_interval {
        if (state.auto)
            state.step();

        SDL.SDL_RenderPresent(renderer);
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
