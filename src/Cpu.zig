const std = @import("std");
const Self = @This();

af: packed struct { a: u8, f: packed struct { _: u4, c: bool, h: bool, n: bool, z: bool } },
bc: packed struct { b: u8, c: u8 },
de: packed struct { d: u8, e: u8 },
hl: packed struct { h: u8, l: u8 },

fn init() Self {
    return .{
        .af = @bitCast(@TypeOf(.{}), 0u16),
        .bc = @bitCast(@TypeOf(.{}), 0x0000),
        .de = @bitCast(@TypeOf(.{}), 0x0000),
        .hl = @bitCast(@TypeOf(.{}), 0x0000),
    };
}

test {
    std.testing.refAllDecls(@This());
}

test "cpu registers" {}
