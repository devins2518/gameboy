const std = @import("std");
const debug = std.log.debug;

pub fn debug_addr(comptime header: []const u8, args: anytype) void {
    const format = " 0x{x:0>4}";
    debug(header ++ format, args);
}

pub const Registers = enum { AF, BC, DE, HL, A, F, B, C, D, E, H, L, SP, PC, PHL, PBC, PDE };
