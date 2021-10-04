const std = @import("std");
const debug = std.log.debug;

pub fn debugAddr(comptime header: []const u8, args: anytype) void {
    const format = " 0x{x:0>2}";
    debug(header ++ format, args);
}

pub const Registers = enum { AF, BC, DE, HL, A, F, B, C, D, E, H, L, SP, PC, PHL, PBC, PDE };

pub const Optional = enum { C, NC, Z, NZ };
