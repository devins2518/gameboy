const std = @import("std");
const Cpu = @import("Cpu.zig");
const Bus = @import("Bus.zig");
const Ppu = @import("Ppu.zig");

var arena = std.heap.ArenaAllocator.init(std.heap.page_allocator);

pub fn main() anyerror!void {
    const path = getPath();
    defer arena.allocator.free(path);
    std.log.info("{s}", .{path});

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
    const dirname = std.fs.path.dirname;
    const join = std.fs.path.join;
    const root = @src().file;

    var args = std.process.args();

    // Skip name
    _ = args.skip();
    const path = blk: {
        if (args.next(arena.child_allocator)) |file| {
            const path = file catch root;
            break :blk path;
        } else {
            break :blk root;
        }
    };
    return join(arena.child_allocator, &.{ dirname(dirname(path).?).?, "/roms/blargg/cpu_instrs/cpu_instrs.gb" }) catch root;
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
