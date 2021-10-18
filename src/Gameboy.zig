const Bus = @import("Bus.zig");
const Cpu = @import("Cpu.zig");
const Ppu = @import("Ppu.zig");
const Self = @This();

auto: bool = false,
clocks: u64 = 0,
bus: Bus,
ppu: Ppu,
cpu: Cpu,

pub fn init(bus: Bus, ppu: Ppu, cpu: Cpu) Self {
    return Self{ .bus = bus, .ppu = ppu, .cpu = cpu };
}

pub fn step(self: *Self) void {
    self.cpu.clock() catch {};
    self.clocks += 1;
}
