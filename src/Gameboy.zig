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

var writer = bufw(std.io.getStdOut().writer());

pub fn init(cart: *Cartridge) Self {
    var bus = Bus.init(cart);
    var ppu = Ppu.init();
    var cpu = Cpu.init(&bus, &ppu, &writer);
    return Self{ .bus = bus, .ppu = ppu, .cpu = cpu };
}

pub fn deinit(self: Self) void {
    self.bus.cart.deinit();
}

pub fn step(self: *Self) void {
    self.cpu.clock() catch {};
    self.clocks += 1;
}
