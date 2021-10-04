const std = @import("std");
const Cpu = @import("Cpu.zig");
const Bus = @import("Bus.zig");
const Ppu = @import("Ppu.zig");
const Cartridge = @import("Cartridge.zig");

var arena = std.heap.ArenaAllocator.init(std.heap.page_allocator);
const blargg = getPath();

pub fn main() anyerror!void {
    var args = std.process.args();
    _ = args.skip();
    const path = if (args.next(arena.child_allocator)) |file|
        file catch blargg
    else
        blargg;

    std.log.info("{s}", .{path});
    var cart = Cartridge.init(path);
    var bus = Bus.init(&cart);
    var ppu = Ppu.init();
    var cpu = Cpu.init(&bus, &ppu);

    var state = struct {
        clocks: u64 = 0,
        bus: Bus,
        ppu: Ppu,
        cpu: Cpu,
    }{ .bus = bus, .ppu = ppu, .cpu = cpu };

    while (true) {
        nosuspend cpu.clock();
        state.clocks += 1;
    }

    arena.deinit();
}

pub fn getPath() []const u8 {
    comptime {
        const root = @src().file;

        return root ++ "/roms/blargg/cpu_instrs/cpu_instrs.gb";
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
