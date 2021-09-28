const std = @import("std");
const Self = @This();

af: packed struct { a: u8 = 0x00, f: packed struct { _: u4 = 0x0, c: u1 = 0, h: u1 = 0, n: u1 = 0, z: u1 = 0 } = .{} } = .{},
bc: packed struct { b: u8 = 0x00, c: u8 = 0x00 } = .{},
de: packed struct { d: u8 = 0x00, e: u8 = 0x00 } = .{},
hl: packed struct { h: u8 = 0x00, l: u8 = 0x00 } = .{},

fn init() Self {
    return .{};
}

test {
    std.testing.refAllDecls(@This());
}

test "cpu registers" {}
