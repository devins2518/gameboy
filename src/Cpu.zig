const std = @import("std");
const Self = @This();

af: packed struct { a: u8, f: u8 },
bc: packed struct { b: u8, c: u8 },
de: packed struct { d: u8, e: u8 },
hl: packed struct { h: u8, l: u8 },

test {
    std.testing.refAllDecls(@This());
}
