const std = @import("std");
const cpu = @import("Cpu.zig");

var arena = std.heap.ArenaAllocator.init(std.heap.page_allocator);

pub fn main() anyerror!void {
    // const path = getPath();

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
