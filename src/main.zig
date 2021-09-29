const std = @import("std");
const Cpu = @import("Cpu.zig");
const Bus = @import("Bus.zig");
const Ppu = @import("Ppu.zig");

var arena = std.heap.ArenaAllocator.init(std.heap.page_allocator);

pub fn main() anyerror!void {
    _ = getPath();

    var bus = Bus.init(undefined);
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

fn getPath() []const u8 {
    var args = std.process.args();
    _ = args.skip();
    // TODO: Is there a way to get the path of build.zig?
    const path = args.next(arena.child_allocator) orelse "/home/devin/Repos/ziggyboy/roms/blargg/cpu_instrs/cpu_instrs.gb" catch unreachable;

    return path;
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
