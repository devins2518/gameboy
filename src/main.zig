const std = @import("std");
const cpu = @import("Cpu.zig");

var arena = std.heap.ArenaAllocator.init(std.heap.page_allocator);

pub fn main() anyerror!void {
    const path = getPath();
    std.log.info("{s}", .{path});

    arena.deinit();
}

fn getPath() [:0]const u8 {
    var args = std.process.args();

    const cwd = std.process.getCwdAlloc(arena.child_allocator);
    _ = args.skip();
    const path = args.next(arena.child_allocator) orelse "../roms/blargg/cpu_instrs/cpu_instrs.gb" catch unreachable;

    return path;
}

test {
    std.testing.refAllDecls(@This());
}
