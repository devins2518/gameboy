const std = @import("std");
const io = std.io;
const Bus = @import("Bus.zig");
const Cartridge = @import("Cartridge.zig");
const Cpu = @import("Cpu.zig");
const Ppu = @import("Ppu.zig");
const Gameboy = @import("Gameboy.zig");
const utils = @import("utils.zig");
const SDL = @import("sdl2");

var arena = std.heap.ArenaAllocator.init(std.heap.page_allocator);
const blargg = utils.getPath();

pub fn main() !void {
    defer arena.deinit();
    var args = std.process.args();
    defer args.deinit();
    _ = args.skip();
    const path = if (args.next(arena.child_allocator)) |file|
        file catch blargg
    else
        blargg;
    std.debug.print("{s}\n", .{path});
    var cart = Cartridge.init(path, arena.child_allocator) catch @panic("");

    var gb = try Gameboy.init(cart);
    defer gb.deinit();

    try gb.run();
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
