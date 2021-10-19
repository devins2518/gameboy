const std = @import("std");
const main = @import("main.zig");
const Bus = @import("Bus.zig");
const Cpu = @import("Cpu.zig");
const Ppu = @import("Ppu.zig");
const Cartridge = @import("Cartridge.zig");
const utils = @import("utils.zig");
const bufw = std.io.bufferedWriter;
const Self = @This();

auto: bool = false,
clocks: u64 = 0,
bus: Bus,
ppu: Ppu,
cpu: Cpu,

var arena = std.heap.ArenaAllocator.init(std.heap.page_allocator);
var writer = bufw(std.io.getStdOut().writer());
const blargg = utils.getPath();

pub fn init() Self {
    var args = std.process.args();
    _ = args.skip();
    const path = if (args.next(arena.child_allocator)) |file|
        file catch blargg
    else
        blargg;
    std.debug.print("{s}\n", .{path});
    var cart = Cartridge.init(path, arena.child_allocator) catch @panic("");
    var bus = Bus.init(&cart);
    var ppu = Ppu.init();
    var cpu = Cpu.init(&bus, &ppu, &writer);
    return Self{ .bus = bus, .ppu = ppu, .cpu = cpu };
}

pub fn deinit(self: Self) void {
    self.bus.cart.deinit();
    arena.deinit();
}

pub fn step(self: *Self) void {
    self.cpu.clock() catch {};
    self.clocks += 1;
}
