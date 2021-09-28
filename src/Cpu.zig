const std = @import("std");
const Self = @This();
const Bus = @import("Bus.zig");
const Ppu = @import("Ppu.zig");

af: packed struct { a: u8 = 0x00, f: packed struct { _: u4 = 0x0, c: u1 = 0, h: u1 = 0, n: u1 = 0, z: u1 = 0 } = .{} } = .{},
bc: packed struct { b: u8 = 0x00, c: u8 = 0x00 } = .{},
de: packed struct { d: u8 = 0x00, e: u8 = 0x00 } = .{},
hl: packed struct { h: u8 = 0x00, l: u8 = 0x00 } = .{},

bus: *Bus,
ppu: *Ppu,

interrupts_enabled: enum { Set, SetPending, SetStart, UnsetStart, UnsetPending, Unset } = .Unset,

pub fn init(bus: *Bus, ppu: *Ppu) Self {
    return .{
        .bus = bus,
        .ppu = ppu,
    };
}

const Register = enum {
    A,
    F,
    AF,
    B,
    C,
    BC,
    D,
    E,
    DE,
    H,
    L,
    HL,
};

test {
    std.testing.refAllDecls(@This());
}

test "cpu registers" {
    const testing = std.testing;

    var bus = Bus.init();
    var ppu = Ppu.init();
    var cpu = Self.init(&bus, &ppu);
    cpu.af.f.z = 1;

    const val: u8 = 0b10000000;
    try testing.expectEqual(val, @bitCast(u8, cpu.af.f));
}
