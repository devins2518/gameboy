const std = @import("std");
const debug = std.log.debug;

pub fn debugAddr(comptime header: []const u8, args: anytype) void {
    const format = " 0x{X:0>2}";
    debug(header ++ format, args);
}

pub const Registers = enum { AF, BC, DE, HL, A, F, B, C, D, E, H, L, SP, PC, PHL, PHLP, PHLM, PBC, PDE };

pub const Optional = enum { C, NC, Z, NZ };

pub const Argument = union(enum) {
    a,
    f,
    b,
    c,
    d,
    e,
    h,
    l,
    af,
    bc,
    de,
    hl,
    pbc,
    pde,
    phl,
    phlm,
    phlp,
    p: u16,
    immU8,
    valU8: u8,
    immU16,
    valU16: u16,
    // Pass in base address
    offsetImmU8: u16,
    spOffsetImmI8,
};
