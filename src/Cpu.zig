const std = @import("std");
const debug = @import("utils.zig").debugAddr;
const utils = @import("utils.zig");
const File = std.fs.File;
const WriteError = std.os.WriteError;
const Bufw = std.io.BufferedWriter(4096, std.io.Writer(File, WriteError, File.write));
const Registers = utils.Registers;
const Optional = utils.Optional;
const Argument = utils.Argument;
const Bus = @import("Bus.zig");
const Ppu = @import("Ppu.zig");
const Self = @This();

const regu16 = packed struct {
    a: u8 align(@alignOf(u16)) = 0x00,
    b: u8 align(@alignOf(u16)) = 0x00,

    fn add(self: *regu16, rhs: u16) void {
        self.* = @bitCast(regu16, @bitCast(u16, self.*) +% rhs);
    }
    fn sub(self: *regu16, rhs: u16) void {
        self.* = @bitCast(regu16, @bitCast(u16, self.*) -% rhs);
    }
};
const afreg = packed struct {
    a: u8 align(@alignOf(u16)) = 0x00,
    f: packed struct { _: u4 = 0x0, c: bool = false, h: bool = false, n: bool = false, z: bool = false } = .{},

    fn add(self: *afreg, rhs: u16) void {
        self.* = @bitCast(afreg, @bitCast(u16, self.*) +% rhs);
    }
    fn sub(self: *afreg, rhs: u16) void {
        self.* = @bitCast(afreg, @bitCast(u16, self.*) -% rhs);
    }
};
af: afreg = .{},
bc: regu16 = .{},
de: regu16 = .{},
hl: regu16 = .{},
pc: u16 = 0,
sp: u16 = 0xFFFE,

bus: *Bus,
ppu: *Ppu,

interrupts_enabled: enum { Set, SetPending, SetStart, UnsetStart, UnsetPending, Unset } = .Unset,

halted: bool = false,

_writer: *Bufw,

pub fn init(bus: *Bus, ppu: *Ppu, _w: *Bufw) Self {
    return .{
        .bus = bus,
        .ppu = ppu,
        ._writer = _w,
    };
}

fn nextInstruction(self: *Self) u8 {
    const data = self.bus.getAddress(self.pc).*;
    self.pc += 1;
    return data;
}

fn immU8(self: *Self) u8 {
    return self.nextInstruction();
}

fn immU16(self: *Self) u16 {
    const b1 = self.nextInstruction();
    const b2 = self.nextInstruction();
    return @bitCast(u16, [2]u8{ b1, b2 });
}

pub fn clock(self: *Self) !void {
    const opcode = self.nextInstruction();

    debug("Matching opcode:", .{opcode});
    switch (opcode) {
        0x00 => self.noop(),
        0x0A => self.ldU8(Registers.A, Argument.pbc),
        0x1A => self.ldU8(Registers.A, Argument.pde),
        0x2A => self.ldU8(Registers.A, Argument.phlp),
        0x3A => self.ldU8(Registers.A, Argument.phlm),
        0x3E => self.ldU8(Registers.A, Argument.immU8),
        0x78 => self.ldU8(Registers.A, Argument.b),
        0x79 => self.ldU8(Registers.A, Argument.c),
        0x7A => self.ldU8(Registers.A, Argument.d),
        0x7B => self.ldU8(Registers.A, Argument.e),
        0x7C => self.ldU8(Registers.A, Argument.h),
        0x7D => self.ldU8(Registers.A, Argument.l),
        0x7E => self.ldU8(Registers.A, Argument.phl),
        0x7F => self.ldU8(Registers.A, Argument.a),
        0xF0 => self.ldU8(Registers.A, Argument{ .offsetImmU8 = 0xFF00 }),
        0x06 => self.ldU8(Registers.B, Argument.immU8),
        0x40 => self.ldU8(Registers.B, Argument.b),
        0x41 => self.ldU8(Registers.B, Argument.c),
        0x42 => self.ldU8(Registers.B, Argument.d),
        0x43 => self.ldU8(Registers.B, Argument.e),
        0x44 => self.ldU8(Registers.B, Argument.h),
        0x45 => self.ldU8(Registers.B, Argument.l),
        0x46 => self.ldU8(Registers.B, Argument.phl),
        0x47 => self.ldU8(Registers.B, Argument.a),
        0x0E => self.ldU8(Registers.C, Argument.immU8),
        0x48 => self.ldU8(Registers.C, Argument.b),
        0x49 => self.ldU8(Registers.C, Argument.c),
        0x4A => self.ldU8(Registers.C, Argument.d),
        0x4B => self.ldU8(Registers.C, Argument.e),
        0x4C => self.ldU8(Registers.C, Argument.h),
        0x4D => self.ldU8(Registers.C, Argument.l),
        0x4E => self.ldU8(Registers.C, Argument.phl),
        0x4F => self.ldU8(Registers.C, Argument.a),
        0x16 => self.ldU8(Registers.D, Argument.immU8),
        0x50 => self.ldU8(Registers.D, Argument.b),
        0x51 => self.ldU8(Registers.D, Argument.c),
        0x52 => self.ldU8(Registers.D, Argument.d),
        0x53 => self.ldU8(Registers.D, Argument.e),
        0x54 => self.ldU8(Registers.D, Argument.h),
        0x55 => self.ldU8(Registers.D, Argument.l),
        0x56 => self.ldU8(Registers.D, Argument.phl),
        0x57 => self.ldU8(Registers.D, Argument.a),
        0x1E => self.ldU8(Registers.E, Argument.immU8),
        0x58 => self.ldU8(Registers.E, Argument.b),
        0x59 => self.ldU8(Registers.E, Argument.c),
        0x5A => self.ldU8(Registers.E, Argument.d),
        0x5B => self.ldU8(Registers.E, Argument.e),
        0x5C => self.ldU8(Registers.E, Argument.h),
        0x5D => self.ldU8(Registers.E, Argument.l),
        0x5E => self.ldU8(Registers.E, Argument.phl),
        0x5F => self.ldU8(Registers.E, Argument.a),
        0x26 => self.ldU8(Registers.H, Argument.immU8),
        0x60 => self.ldU8(Registers.H, Argument.b),
        0x61 => self.ldU8(Registers.H, Argument.c),
        0x62 => self.ldU8(Registers.H, Argument.d),
        0x63 => self.ldU8(Registers.H, Argument.e),
        0x64 => self.ldU8(Registers.H, Argument.h),
        0x65 => self.ldU8(Registers.H, Argument.l),
        0x66 => self.ldU8(Registers.H, Argument.phl),
        0x67 => self.ldU8(Registers.H, Argument.a),
        0x2E => self.ldU8(Registers.L, Argument.immU8),
        0x68 => self.ldU8(Registers.L, Argument.b),
        0x69 => self.ldU8(Registers.L, Argument.c),
        0x6A => self.ldU8(Registers.L, Argument.d),
        0x6B => self.ldU8(Registers.L, Argument.e),
        0x6C => self.ldU8(Registers.L, Argument.h),
        0x6D => self.ldU8(Registers.L, Argument.l),
        0x6E => self.ldU8(Registers.L, Argument.phl),
        0x6F => self.ldU8(Registers.L, Argument.a),
        0x02 => self.ldU8(Registers.PBC, Argument.a),
        0x12 => self.ldU8(Registers.PDE, Argument.a),
        0x70 => self.ldU8(Registers.PHL, Argument.b),
        0x71 => self.ldU8(Registers.PHL, Argument.c),
        0x72 => self.ldU8(Registers.PHL, Argument.d),
        0x73 => self.ldU8(Registers.PHL, Argument.e),
        0x74 => self.ldU8(Registers.PHL, Argument.h),
        0x75 => self.ldU8(Registers.PHL, Argument.l),
        0x77 => self.ldU8(Registers.PHL, Argument.a),
        0x22 => self.ldU8(Registers.PHLP, Argument.a),
        0x32 => self.ldU8(Registers.PHLM, Argument.a),
        0x01 => self.ldU16(Registers.BC, Argument.immU16),
        0x11 => self.ldU16(Registers.DE, Argument.immU16),
        0x21 => self.ldU16(Registers.HL, Argument.immU16),
        0x31 => self.ldU16(Registers.SP, Argument.immU16),
        0xE0 => self.bus.getAddress(0xFF00 + @as(u16, self.immU8())).* = self.af.a,
        0x03 => self.inc(Registers.BC),
        0x04 => self.inc(Registers.B),
        0x0C => self.inc(Registers.C),
        0x13 => self.inc(Registers.DE),
        0x14 => self.inc(Registers.D),
        0x1C => self.inc(Registers.E),
        0x23 => self.inc(Registers.HL),
        0x24 => self.inc(Registers.H),
        0x2C => self.inc(Registers.L),
        0x33 => self.inc(Registers.SP),
        0x34 => self.inc(Registers.PHL),
        0x3C => self.inc(Registers.A),
        0x05 => self.dec(Registers.B),
        0x07 => self.rlca(),
        0x08 => {
            const val = self.immU16();
            self.bus.getAddress(val).* = @truncate(u8, self.sp >> 8);
            self.bus.getAddress(val + 1).* = @truncate(u8, self.sp);
        },
        0x09 => self.addU16(@bitCast(u16, self.bc)),
        0x0B => self.dec(Registers.BC),
        0x0D => self.dec(Registers.C),
        0x0F => self.rrca(),
        0x10 => @panic("unhandled opcode: 0x10"),
        0x15 => self.dec(Registers.D),
        0x17 => self.rla(),
        0x18 => self.jr(null),
        0x19 => self.addU16(@bitCast(u16, self.de)),
        0x1B => self.dec(Registers.DE),
        0x1D => self.dec(Registers.E),
        0x1F => self.rra(),
        0x20 => self.jr(.NZ),
        0x25 => self.dec(Registers.H),
        0x27 => @panic("unhandled opcode: 0x27"),
        0x28 => self.jr(.Z),
        0x29 => self.addU16(@bitCast(u16, self.hl)),
        0x2B => self.dec(Registers.HL),
        0x2D => self.dec(Registers.L),
        0x2F => @panic("unhandled opcode: 0x2F"),
        0x30 => self.jr(.NC),
        0x35 => self.dec(Registers.PHL),
        0x36 => self.bus.getAddress(@bitCast(u16, self.hl)).* = self.immU8(),
        0x37 => @panic("unhandled opcode: 0x37"),
        0x38 => self.jr(.C),
        0x39 => self.addU16(self.sp),
        0x3B => self.dec(Registers.SP),
        0x3D => self.dec(Registers.A),
        0x3F => @panic("unhandled opcode: 0x3F"),
        0x76 => self.halt(),
        0x80 => self.addU8(Argument.b),
        0x81 => self.addU8(Argument.c),
        0x82 => self.addU8(Argument.d),
        0x83 => self.addU8(Argument.e),
        0x84 => self.addU8(Argument.h),
        0x85 => self.addU8(Argument.l),
        0x86 => self.addU8(Argument.phl),
        0x87 => self.addU8(Argument.a),
        0x88 => @panic("unhandled opcode: 0x88"),
        0x89 => @panic("unhandled opcode: 0x89"),
        0x8A => @panic("unhandled opcode: 0x8A"),
        0x8B => @panic("unhandled opcode: 0x8B"),
        0x8C => @panic("unhandled opcode: 0x8C"),
        0x8D => @panic("unhandled opcode: 0x8D"),
        0x8E => @panic("unhandled opcode: 0x8E"),
        0x8F => @panic("unhandled opcode: 0x8F"),
        0x90 => self.sub(self.bc.a),
        0x91 => self.sub(self.bc.b),
        0x92 => self.sub(self.de.a),
        0x93 => self.sub(self.de.b),
        0x94 => self.sub(self.hl.a),
        0x95 => self.sub(self.hl.b),
        0x96 => self.sub(self.bus.getAddress(@bitCast(u16, self.hl)).*),
        0x97 => self.sub(self.af.a),
        0x98 => @panic("unhandled opcode: 0x98"),
        0x99 => @panic("unhandled opcode: 0x99"),
        0x9A => @panic("unhandled opcode: 0x9A"),
        0x9B => @panic("unhandled opcode: 0x9B"),
        0x9C => @panic("unhandled opcode: 0x9C"),
        0x9D => @panic("unhandled opcode: 0x9D"),
        0x9E => @panic("unhandled opcode: 0x9E"),
        0x9F => @panic("unhandled opcode: 0x9F"),
        0xA0 => self.andReg(self.bc.a),
        0xA1 => self.andReg(self.bc.b),
        0xA2 => self.andReg(self.de.a),
        0xA3 => self.andReg(self.de.b),
        0xA4 => self.andReg(self.hl.a),
        0xA5 => self.andReg(self.hl.b),
        0xA6 => self.andReg(self.bus.getAddress(@bitCast(u16, self.hl)).*),
        0xA7 => self.andReg(self.af.a),
        0xA8 => self.xor(Argument.b),
        0xA9 => self.xor(Argument.c),
        0xAA => self.xor(Argument.d),
        0xAB => self.xor(Argument.e),
        0xAC => self.xor(Argument.h),
        0xAD => self.xor(Argument.l),
        0xAE => self.xor(Argument.phl),
        0xAF => self.xor(Argument.a),
        0xB0 => self.orReg(self.bc.a),
        0xB1 => self.orReg(self.bc.b),
        0xB2 => self.orReg(self.de.a),
        0xB3 => self.orReg(self.de.b),
        0xB4 => self.orReg(self.hl.a),
        0xB5 => self.orReg(self.hl.b),
        0xB6 => self.orReg(self.bus.getAddress(@bitCast(u16, self.hl)).*),
        0xB7 => self.orReg(self.af.a),
        0xB8 => @panic("unhandled opcode: 0xB8"),
        0xB9 => @panic("unhandled opcode: 0xB9"),
        0xBA => @panic("unhandled opcode: 0xBA"),
        0xBB => @panic("unhandled opcode: 0xBB"),
        0xBC => @panic("unhandled opcode: 0xBC"),
        0xBD => @panic("unhandled opcode: 0xBD"),
        0xBE => @panic("unhandled opcode: 0xBE"),
        0xBF => @panic("unhandled opcode: 0xBF"),
        0xC0 => @panic("unhandled opcode: 0xC0"),
        0xC1 => @panic("unhandled opcode: 0xC1"),
        0xC2 => self.jp(self.immU16(), .NZ),
        0xC3 => self.jp(self.immU16(), null),
        0xC4 => self.call(.NZ),
        0xC6 => self.addU8(Argument.immU8),
        0xC7 => @panic("unhandled opcode: 0xC7"),
        0xC8 => @panic("unhandled opcode: 0xC8"),
        0xC9 => @panic("unhandled opcode: 0xC9"),
        0xCA => self.jp(self.immU16(), .Z),
        0xCB => {
            const nopcode = self.nextInstruction();
            switch (nopcode) {
                0x00 => self.rlc(Registers.B),
                0x01 => self.rlc(Registers.C),
                0x02 => self.rlc(Registers.D),
                0x03 => self.rlc(Registers.E),
                0x04 => self.rlc(Registers.H),
                0x05 => self.rlc(Registers.L),
                0x06 => self.rlc(Registers.PHL),
                0x07 => self.rlc(Registers.A),
                0x08 => self.rrc(Registers.B),
                0x09 => self.rrc(Registers.C),
                0x0A => self.rrc(Registers.D),
                0x0B => self.rrc(Registers.E),
                0x0C => self.rrc(Registers.H),
                0x0D => self.rrc(Registers.L),
                0x0E => self.rrc(Registers.PHL),
                0x0F => self.rrc(Registers.A),
                0x10 => self.rl(Registers.B),
                0x11 => self.rl(Registers.C),
                0x12 => self.rl(Registers.D),
                0x13 => self.rl(Registers.E),
                0x14 => self.rl(Registers.H),
                0x15 => self.rl(Registers.L),
                0x16 => self.rl(Registers.PHL),
                0x17 => self.rl(Registers.A),
                0x18 => self.rr(Registers.B),
                0x19 => self.rr(Registers.C),
                0x1A => self.rr(Registers.D),
                0x1B => self.rr(Registers.E),
                0x1C => self.rr(Registers.H),
                0x1D => self.rr(Registers.L),
                0x1E => self.rr(Registers.PHL),
                0x1F => self.rr(Registers.A),
                0x20 => self.sla(Registers.B),
                0x21 => self.sla(Registers.C),
                0x22 => self.sla(Registers.D),
                0x23 => self.sla(Registers.E),
                0x24 => self.sla(Registers.H),
                0x25 => self.sla(Registers.L),
                0x26 => self.sla(Registers.PHL),
                0x27 => self.sla(Registers.A),
                0x28 => self.sra(Registers.B),
                0x29 => self.sra(Registers.C),
                0x2A => self.sra(Registers.D),
                0x2B => self.sra(Registers.E),
                0x2C => self.sra(Registers.H),
                0x2D => self.sra(Registers.L),
                0x2E => self.sra(Registers.PHL),
                0x2F => self.sra(Registers.A),
                0x30 => self.swap(Registers.B),
                0x31 => self.swap(Registers.C),
                0x32 => self.swap(Registers.D),
                0x33 => self.swap(Registers.E),
                0x34 => self.swap(Registers.H),
                0x35 => self.swap(Registers.L),
                0x36 => self.swap(Registers.PHL),
                0x37 => self.swap(Registers.A),
                0x38 => self.srl(Registers.B),
                0x39 => self.srl(Registers.C),
                0x3A => self.srl(Registers.D),
                0x3B => self.srl(Registers.E),
                0x3C => self.srl(Registers.H),
                0x3D => self.srl(Registers.L),
                0x3E => self.srl(Registers.PHL),
                0x3F => self.srl(Registers.A),
                0x40 => self.bit(0, Argument.b),
                0x41 => self.bit(0, Argument.c),
                0x42 => self.bit(0, Argument.d),
                0x43 => self.bit(0, Argument.e),
                0x44 => self.bit(0, Argument.h),
                0x45 => self.bit(0, Argument.l),
                0x46 => self.bit(0, Argument.phl),
                0x47 => self.bit(0, Argument.a),
                0x48 => self.bit(1, Argument.b),
                0x49 => self.bit(1, Argument.c),
                0x4A => self.bit(1, Argument.d),
                0x4B => self.bit(1, Argument.e),
                0x4C => self.bit(1, Argument.h),
                0x4D => self.bit(1, Argument.l),
                0x4E => self.bit(1, Argument.phl),
                0x4F => self.bit(1, Argument.a),
                0x50 => self.bit(2, Argument.b),
                0x51 => self.bit(2, Argument.c),
                0x52 => self.bit(2, Argument.d),
                0x53 => self.bit(2, Argument.e),
                0x54 => self.bit(2, Argument.h),
                0x55 => self.bit(2, Argument.l),
                0x56 => self.bit(2, Argument.phl),
                0x57 => self.bit(2, Argument.a),
                0x58 => self.bit(3, Argument.b),
                0x59 => self.bit(3, Argument.c),
                0x5A => self.bit(3, Argument.d),
                0x5B => self.bit(3, Argument.e),
                0x5C => self.bit(3, Argument.h),
                0x5D => self.bit(3, Argument.l),
                0x5E => self.bit(3, Argument.phl),
                0x5F => self.bit(3, Argument.a),
                0x60 => self.bit(4, Argument.b),
                0x61 => self.bit(4, Argument.c),
                0x62 => self.bit(4, Argument.d),
                0x63 => self.bit(4, Argument.e),
                0x64 => self.bit(4, Argument.h),
                0x65 => self.bit(4, Argument.l),
                0x66 => self.bit(4, Argument.phl),
                0x67 => self.bit(4, Argument.a),
                0x68 => self.bit(5, Argument.b),
                0x69 => self.bit(5, Argument.c),
                0x6A => self.bit(5, Argument.d),
                0x6B => self.bit(5, Argument.e),
                0x6C => self.bit(5, Argument.h),
                0x6D => self.bit(5, Argument.l),
                0x6E => self.bit(5, Argument.phl),
                0x6F => self.bit(5, Argument.a),
                0x70 => self.bit(6, Argument.b),
                0x71 => self.bit(6, Argument.c),
                0x72 => self.bit(6, Argument.d),
                0x73 => self.bit(6, Argument.e),
                0x74 => self.bit(6, Argument.h),
                0x75 => self.bit(6, Argument.l),
                0x76 => self.bit(6, Argument.phl),
                0x77 => self.bit(6, Argument.a),
                0x78 => self.bit(7, Argument.b),
                0x79 => self.bit(7, Argument.c),
                0x7A => self.bit(7, Argument.d),
                0x7B => self.bit(7, Argument.e),
                0x7C => self.bit(7, Argument.h),
                0x7D => self.bit(7, Argument.l),
                0x7E => self.bit(7, Argument.phl),
                0x7F => self.bit(7, Argument.a),
                0x80 => self.res(0, Registers.B),
                0x81 => self.res(0, Registers.C),
                0x82 => self.res(0, Registers.D),
                0x83 => self.res(0, Registers.E),
                0x84 => self.res(0, Registers.H),
                0x85 => self.res(0, Registers.L),
                0x86 => self.res(0, Registers.PHL),
                0x87 => self.res(0, Registers.A),
                0x88 => self.res(1, Registers.B),
                0x89 => self.res(1, Registers.C),
                0x8A => self.res(1, Registers.D),
                0x8B => self.res(1, Registers.E),
                0x8C => self.res(1, Registers.H),
                0x8D => self.res(1, Registers.L),
                0x8E => self.res(1, Registers.PHL),
                0x8F => self.res(1, Registers.A),
                0x90 => self.res(2, Registers.B),
                0x91 => self.res(2, Registers.C),
                0x92 => self.res(2, Registers.D),
                0x93 => self.res(2, Registers.E),
                0x94 => self.res(2, Registers.H),
                0x95 => self.res(2, Registers.L),
                0x96 => self.res(2, Registers.PHL),
                0x97 => self.res(2, Registers.A),
                0x98 => self.res(3, Registers.B),
                0x99 => self.res(3, Registers.C),
                0x9A => self.res(3, Registers.D),
                0x9B => self.res(3, Registers.E),
                0x9C => self.res(3, Registers.H),
                0x9D => self.res(3, Registers.L),
                0x9E => self.res(3, Registers.PHL),
                0x9F => self.res(3, Registers.A),
                0xA0 => self.res(4, Registers.B),
                0xA1 => self.res(4, Registers.C),
                0xA2 => self.res(4, Registers.D),
                0xA3 => self.res(4, Registers.E),
                0xA4 => self.res(4, Registers.H),
                0xA5 => self.res(4, Registers.L),
                0xA6 => self.res(4, Registers.PHL),
                0xA7 => self.res(4, Registers.A),
                0xA8 => self.res(5, Registers.B),
                0xA9 => self.res(5, Registers.C),
                0xAA => self.res(5, Registers.D),
                0xAB => self.res(5, Registers.E),
                0xAC => self.res(5, Registers.H),
                0xAD => self.res(5, Registers.L),
                0xAE => self.res(5, Registers.PHL),
                0xAF => self.res(5, Registers.A),
                0xB0 => self.res(6, Registers.B),
                0xB1 => self.res(6, Registers.C),
                0xB2 => self.res(6, Registers.D),
                0xB3 => self.res(6, Registers.E),
                0xB4 => self.res(6, Registers.H),
                0xB5 => self.res(6, Registers.L),
                0xB6 => self.res(6, Registers.PHL),
                0xB7 => self.res(6, Registers.A),
                0xB8 => self.res(7, Registers.B),
                0xB9 => self.res(7, Registers.C),
                0xBA => self.res(7, Registers.D),
                0xBB => self.res(7, Registers.E),
                0xBC => self.res(7, Registers.H),
                0xBD => self.res(7, Registers.L),
                0xBE => self.res(7, Registers.PHL),
                0xBF => self.res(7, Registers.A),
                0xC0 => self.set(0, Registers.B),
                0xC1 => self.set(0, Registers.C),
                0xC2 => self.set(0, Registers.D),
                0xC3 => self.set(0, Registers.E),
                0xC4 => self.set(0, Registers.H),
                0xC5 => self.set(0, Registers.L),
                0xC6 => self.set(0, Registers.PHL),
                0xC7 => self.set(0, Registers.A),
                0xC8 => self.set(1, Registers.B),
                0xC9 => self.set(1, Registers.C),
                0xCA => self.set(1, Registers.D),
                0xCB => self.set(1, Registers.E),
                0xCC => self.set(1, Registers.H),
                0xCD => self.set(1, Registers.L),
                0xCE => self.set(1, Registers.PHL),
                0xCF => self.set(1, Registers.A),
                0xD0 => self.set(2, Registers.B),
                0xD1 => self.set(2, Registers.C),
                0xD2 => self.set(2, Registers.D),
                0xD3 => self.set(2, Registers.E),
                0xD4 => self.set(2, Registers.H),
                0xD5 => self.set(2, Registers.L),
                0xD6 => self.set(2, Registers.PHL),
                0xD7 => self.set(2, Registers.A),
                0xD8 => self.set(3, Registers.B),
                0xD9 => self.set(3, Registers.C),
                0xDA => self.set(3, Registers.D),
                0xDB => self.set(3, Registers.E),
                0xDC => self.set(3, Registers.H),
                0xDD => self.set(3, Registers.L),
                0xDE => self.set(3, Registers.PHL),
                0xDF => self.set(3, Registers.A),
                0xE0 => self.set(4, Registers.B),
                0xE1 => self.set(4, Registers.C),
                0xE2 => self.set(4, Registers.D),
                0xE3 => self.set(4, Registers.E),
                0xE4 => self.set(4, Registers.H),
                0xE5 => self.set(4, Registers.L),
                0xE6 => self.set(4, Registers.PHL),
                0xE7 => self.set(4, Registers.A),
                0xE8 => self.set(5, Registers.B),
                0xE9 => self.set(5, Registers.C),
                0xEA => self.set(5, Registers.D),
                0xEB => self.set(5, Registers.E),
                0xEC => self.set(5, Registers.H),
                0xED => self.set(5, Registers.L),
                0xEE => self.set(5, Registers.PHL),
                0xEF => self.set(5, Registers.A),
                0xF0 => self.set(6, Registers.B),
                0xF1 => self.set(6, Registers.C),
                0xF2 => self.set(6, Registers.D),
                0xF3 => self.set(6, Registers.E),
                0xF4 => self.set(6, Registers.H),
                0xF5 => self.set(6, Registers.L),
                0xF6 => self.set(6, Registers.PHL),
                0xF7 => self.set(6, Registers.A),
                0xF8 => self.set(7, Registers.B),
                0xF9 => self.set(7, Registers.C),
                0xFA => self.set(7, Registers.D),
                0xFB => self.set(7, Registers.E),
                0xFC => self.set(7, Registers.H),
                0xFD => self.set(7, Registers.L),
                0xFE => self.set(7, Registers.PHL),
                0xFF => self.set(7, Registers.A),
            }
        },
        0xCC => self.call(.C),
        0xCD => self.call(null),
        0xCE => @panic("unhandled opcode: 0xCE"),
        0xCF => @panic("unhandled opcode: 0xCF"),
        0xD0 => @panic("unhandled opcode: 0xD0"),
        0xD1 => @panic("unhandled opcode: 0xD1"),
        0xD2 => self.jp(self.immU16(), .NC),
        0xD4 => self.call(.NC),
        0xD6 => self.sub(self.immU8()),
        0xD7 => @panic("unhandled opcode: 0xD7"),
        0xD8 => @panic("unhandled opcode: 0xD8"),
        0xD9 => @panic("unhandled opcode: 0xD9"),
        0xDA => self.jp(self.immU16(), .C),
        0xDC => self.call(.C),
        0xDE => @panic("unhandled opcode: 0xDE"),
        0xDF => @panic("unhandled opcode: 0xDF"),
        0xE1 => @panic("unhandled opcode: 0xE1"),
        0xE2 => self.bus.getAddress(0xFF00 + @as(u16, self.bc.b)).* = self.af.a,
        0xE6 => @panic("unhandled opcode: 0xE6"),
        0xE7 => @panic("unhandled opcode: 0xE7"),
        0xE8 => {
            const a = self.sp;
            const b = self.immU8();
            self.sp +%= b;
            self.af.f.z = false;
            self.af.f.n = false;
            self.halfCarry(@truncate(u8, a), @truncate(u8, b));
            self.carry(a, b);
        },
        0xE9 => self.jp(@bitCast(u16, self.hl), null),
        0xEA => @panic("unhandled opcode: 0xEA"),
        0xEE => self.xor(Argument.immU8),
        0xEF => @panic("unhandled opcode: 0xEF"),
        0xF1 => @panic("unhandled opcode: 0xF1"),
        0xF2 => @panic("unhandled opcode: 0xF2"),
        0xF3 => self.di(),
        0xC5 => self.push(Argument.bc),
        0xD5 => self.push(Argument.de),
        0xE5 => self.push(Argument.hl),
        0xF5 => self.push(Argument.af),
        0xF6 => self.orReg(self.immU8()),
        0xF7 => @panic("unhandled opcode: 0xF7"),
        0xF8 => self.ei(),
        0xF9 => @panic("unhandled opcode: 0xF9"),
        0xFA => @panic("unhandled opcode: 0xFA"),
        0xFB => @panic("unhandled opcode: 0xFB"),
        0xFE => @panic("unhandled opcode: 0xFE"),
        0xFF => @panic("unhandled opcode: 0xFF"),
        else => unreachable,
    }

    self.interrupts_enabled = switch (self.interrupts_enabled) {
        .UnsetStart => .UnsetPending,
        .SetStart => .SetPending,
        .UnsetPending => .Unset,
        .SetPending => .Set,
        else => self.interrupts_enabled,
    };

    self.write("\n");
}

fn addU8(self: *Self, comptime arg: Argument) void {
    self.write("ADD A, ");
    const val = self.getArg(arg, u8);
    const old_a = self.af.a;
    self.af.a +%= val;
    self.af.f.z = self.af.a == 0;
    self.af.f.n = false;
    self.halfCarry(old_a, val);
    self.carry(old_a, val);
}

fn andReg(self: *Self, val: u8) void {
    self.af.a &= val;
    self.af.f.z = self.af.a == 0;
    self.af.f.n = false;
    self.af.f.h = true;
    self.af.f.c = false;
}

fn addU16(self: *Self, val: u16) void {
    var hl = @bitCast(u16, self.hl);

    self.af.f.n = false;
    self.halfCarry(@truncate(u8, hl), @truncate(u8, val));
    self.carry(hl, val);

    hl +%= val;
}

fn bit(self: *Self, comptime b: u8, comptime arg: Argument) void {
    self.writeWithArg("BIT {}, ", .{b});
    const r = self.getArg(arg, u8);
    self.af.f.z = ((r >> b) & 0x01) == 0;
    self.af.f.n = false;
    self.af.f.h = true;
}

fn call(self: *Self, comptime opt: ?Optional) void {
    if (opt) |o| if (!self.check(o))
        return;

    const addr = self.immU16();
    self.bus.getAddress(self.sp).* = @truncate(u8, self.pc & 0x0F);
    self.sp -%= 1;
    self.bus.getAddress(self.sp).* = @truncate(u8, (self.pc & 0xF0) >> 8);
    self.sp -%= 1;

    self.pc = addr;
}

fn di(self: *Self) void {
    self.interrupts_enabled = .UnsetStart;
}

fn dec(self: *Self, comptime field: Registers) void {
    switch (field) {
        .AF => self.af.sub(1),
        .BC => self.bc.sub(1),
        .DE => self.de.sub(1),
        .HL => self.hl.sub(1),
        .PC => self.pc -%= 1,
        .SP => self.sp -%= 1,
        else => {
            const r = blk: {
                switch (field) {
                    .A => {
                        self.af.a -%= 1;
                        break :blk self.af.a;
                    },
                    .F => {
                        self.af.f -%= 1;
                        break :blk self.af.f;
                    },
                    .B => {
                        self.bc.a -%= 1;
                        break :blk self.bc.a;
                    },
                    .C => {
                        self.bc.b -%= 1;
                        break :blk self.bc.b;
                    },
                    .D => {
                        self.de.a -%= 1;
                        break :blk self.de.a;
                    },
                    .E => {
                        self.de.b -%= 1;
                        break :blk self.de.b;
                    },
                    .H => {
                        self.hl.a -%= 1;
                        break :blk self.hl.a;
                    },
                    .L => {
                        self.hl.b -%= 1;
                        break :blk self.hl.b;
                    },
                    .PHL => {
                        const val = self.bus.getAddress(@bitCast(u16, self.hl));
                        val.* -%= 1;
                        break :blk val.*;
                    },
                    else => @compileError("Invalid reg"),
                }
            };

            self.af.f.z = r == 0;
            self.af.f.n = true;
            self.halfCarry(r +% 1, 1);
        },
    }
}

fn ei(self: *Self) void {
    self.interrupts_enabled = .SetStart;
}

fn halt(self: *Self) void {
    self.halted = true;
}

fn inc(self: *Self, comptime field: Registers) void {
    switch (field) {
        .AF => self.af.add(1),
        .BC => self.bc.add(1),
        .DE => self.de.add(1),
        .HL => self.hl.add(1),
        .PC => self.pc +%= 1,
        .SP => self.sp +%= 1,
        else => {
            const r = blk: {
                switch (field) {
                    .A => {
                        self.af.a +%= 1;
                        break :blk self.af.a;
                    },
                    .F => {
                        self.af.f +%= 1;
                        break :blk self.af.f;
                    },
                    .B => {
                        self.bc.a +%= 1;
                        break :blk self.bc.a;
                    },
                    .C => {
                        self.bc.b +%= 1;
                        break :blk self.bc.b;
                    },
                    .D => {
                        self.de.a +%= 1;
                        break :blk self.de.a;
                    },
                    .E => {
                        self.de.b +%= 1;
                        break :blk self.de.b;
                    },
                    .H => {
                        self.hl.a +%= 1;
                        break :blk self.hl.a;
                    },
                    .L => {
                        self.hl.b +%= 1;
                        break :blk self.hl.b;
                    },
                    .PHL => {
                        const val = self.bus.getAddress(@bitCast(u16, self.hl));
                        val.* +%= 1;
                        break :blk val.*;
                    },
                    else => @compileError("Invalid reg"),
                }
            };
            self.af.f.z = r == 0;
            self.af.f.n = false;
            self.halfCarry(r -% 1, 1);
        },
    }
}

fn jp(self: *Self, addr: u16, comptime opt: ?Optional) void {
    if (opt) |o| if (self.check(o))
        return;

    self.pc = addr;
}

fn jr(self: *Self, comptime opt: ?Optional) void {
    const r = @bitCast(i8, self.immU8());
    const addr = @bitCast(u16, @bitCast(i16, self.pc) +% @as(i16, r));
    self.write("JR ");
    if (opt) |o| {
        self.writeWithArg("{}, 0x{X:0>2}", .{ o, addr });
        if (!self.check(o))
            return
        else
            self.pc = addr;
    } else {
        self.writeWithArg("{}", .{addr});
        self.pc = addr;
    }
}

fn ldU16(self: *Self, comptime field: Registers, comptime arg: Argument) void {
    self.write("LD ");
    const f = self.getReg(field, u16);
    const a = self.getArg(arg, u16);
    f.* = a;
}

fn ldU8(self: *Self, comptime field: Registers, comptime arg: Argument) void {
    self.write("LD ");
    const f = self.getReg(field, u8);
    const a = self.getArg(arg, u8);
    f.* = a;
}

fn noop(_: *Self) void {}

fn orReg(self: *Self, val: u8) void {
    self.af.a |= val;
    self.af.f.z = self.af.a == 0;
    self.af.f.n = false;
    self.af.f.h = false;
    self.af.f.c = false;
}

fn res(self: *Self, comptime b: u8, comptime field: Registers) void {
    const r = self.getReg(field, u8);
    r.* |= ~@intCast(u8, 1 << b);
}

fn rl(self: *Self, comptime field: Registers) void {
    const reg = self.getReg(field, u8);
    const c = @boolToInt(self.af.f.c);
    self.af.f.c = (reg.* >> 7 == 1);
    self.af.f.n = false;
    self.af.f.h = false;
    reg.* = reg.* << 1 | c;
    self.af.f.z = reg.* == 0;
}

fn rla(self: *Self) void {
    const reg = &self.af.a;
    const c = @boolToInt(self.af.f.c);
    self.af.f.c = (reg.* >> 7 == 1);
    self.af.f.n = false;
    self.af.f.h = false;
    reg.* = reg.* << 1 | c;
    self.af.f.z = reg.* == 0;
}

fn rlc(self: *Self, comptime field: Registers) void {
    const reg = self.getReg(field, u8);
    self.af.f.c = (reg.* >> 7 == 1);
    self.af.f.n = false;
    self.af.f.h = false;
    reg.* = std.math.rotl(u8, 1, reg.*);
    self.af.f.z = reg.* == 0;
}

fn rlca(self: *Self) void {
    const reg = &self.af.a;
    self.af.f.c = (reg.* >> 7 == 1);
    self.af.f.n = false;
    self.af.f.h = false;
    reg.* = std.math.rotl(u8, 1, reg.*);
    self.af.f.z = reg.* == 0;
}

fn rr(self: *Self, comptime field: Registers) void {
    const reg = self.getReg(field, u8);
    const c = @boolToInt(self.af.f.c);
    self.af.f.c = (reg.* & 0x01 == 1);
    self.af.f.n = false;
    self.af.f.h = false;
    reg.* = @as(u8, c) << 7 | reg.* >> 1;
    self.af.f.z = reg.* == 0;
}

fn rra(self: *Self) void {
    const reg = &self.af.a;
    const c = @boolToInt(self.af.f.c);
    self.af.f.c = (reg.* & 0x01 == 1);
    self.af.f.n = false;
    self.af.f.h = false;
    reg.* = @as(u8, c) << 7 | reg.* >> 1;
    self.af.f.z = reg.* == 0;
}

fn rrc(self: *Self, comptime field: Registers) void {
    const reg = self.getReg(field, u8);
    self.af.f.c = (reg.* & 0x01 == 1);
    self.af.f.n = false;
    self.af.f.h = false;
    reg.* = std.math.rotr(u8, 1, reg.*);
    self.af.f.z = reg.* == 0;
}

fn rrca(self: *Self) void {
    const reg = &self.af.a;
    self.af.f.c = (reg.* & 0x01 == 1);
    self.af.f.n = false;
    self.af.f.h = false;
    reg.* = std.math.rotr(u8, 1, reg.*);
    self.af.f.z = reg.* == 0;
}

fn set(self: *Self, comptime b: u8, comptime field: Registers) void {
    self.getReg(field, u8).* |= (1 << b);
}

fn sla(self: *Self, comptime field: Registers) void {
    const r = self.getReg(field, u8);
    self.af.f.c = (r.* >> 7) == 1;
    self.af.f.n = false;
    self.af.f.h = false;
    r.* <<= 1;
    self.af.f.z = r.* == 0;
}

fn sra(self: *Self, comptime field: Registers) void {
    const r = self.getReg(field, u8);
    self.af.f.c = (r.* & 0x01) == 1;
    self.af.f.n = false;
    self.af.f.h = false;
    r.* = (r.* >> 1) | (r.* & 0x80);
    self.af.f.z = r.* == 0;
}

fn srl(self: *Self, comptime field: Registers) void {
    const r = self.getReg(field, u8);
    self.af.f.c = (r.* & 0x01) == 1;
    self.af.f.n = false;
    self.af.f.h = false;
    r.* = r.* >> 1;
    self.af.f.z = r.* == 0;
}

fn sub(self: *Self, val: u8) void {
    self.af.f.z = self.af.a == 0;
    self.af.f.n = true;
    self.halfCarry(self.af.a, val);
    self.carry(self.af.a, val);

    self.af.a -%= val;
}

fn swap(self: *Self, comptime field: Registers) void {
    const r = self.getReg(field, u8);
    r.* = (r.* & 0x0F) << 4 | (r.* & 0xF0) >> 4;
    self.af.f.c = false;
    self.af.f.n = false;
    self.af.f.h = false;
    self.af.f.z = r.* == 0;
}

fn push(self: *Self, comptime arg: Argument) void {
    self.write("PUSH ");
    const v = self.getArg(arg, u16);
    self.bus.getAddress(self.sp).* = @truncate(u8, v & 0xFF00);
    self.sp -%= 1;
    self.bus.getAddress(self.sp).* = @truncate(u8, v & 0x00FF);
    self.sp -%= 1;
}

fn xor(self: *Self, comptime arg: Argument) void {
    self.write("XOR ");
    const val = self.getArg(arg, u8);
    self.af.a ^= val;
    self.af.f.z = self.af.a == 0;
    self.af.f.n = false;
    self.af.f.h = false;
    self.af.f.c = false;
}

fn halfCarry(self: *Self, a: u8, b: u8) void {
    self.af.f.h = (((a & 0xf) + (b & 0xf)) & 0x10) == 0x10;
}

fn carry(self: *Self, a: u16, b: u16) void {
    self.af.f.c = (((a & 0xf0) + (b & 0xf0)) & 0x100) == 0x100;
}

fn check(self: *Self, comptime opt: Optional) bool {
    return switch (opt) {
        .C => self.af.f.c,
        .NC => !self.af.f.c,
        .Z => self.af.f.z,
        .NZ => !self.af.f.z,
    };
}

fn getReg(self: *Self, comptime field: Registers, comptime T: type) *T {
    return blk: {
        switch (field) {
            .A => {
                self.write("A, ");
                break :blk &self.af.a;
            },
            .F => {
                self.write("A, ");
                break :blk &self.af.f;
            },
            .B => {
                self.write("B, ");
                break :blk &self.bc.a;
            },
            .C => {
                self.write("C, ");
                break :blk &self.bc.b;
            },
            .D => {
                self.write("D, ");
                break :blk &self.de.a;
            },
            .E => {
                self.write("E, ");
                break :blk &self.de.b;
            },
            .H => {
                self.write("H, ");
                break :blk &self.hl.a;
            },
            .L => {
                self.write("L, ");
                break :blk &self.hl.b;
            },
            .PBC => {
                self.write("(BC), ");
                break :blk self.bus.getAddress(@bitCast(u16, self.bc));
            },
            .PDE => {
                self.write("(DE), ");
                break :blk self.bus.getAddress(@bitCast(u16, self.de));
            },
            .PHL => {
                self.write("(HL), ");
                break :blk self.bus.getAddress(@bitCast(u16, self.hl));
            },
            .PHLP => {
                self.write("(HL+), ");
                self.hl.add(1);
                break :blk self.bus.getAddress(@bitCast(u16, self.hl) -% 1);
            },
            .PHLM => {
                self.write("(HL-), ");
                self.hl.sub(1);
                break :blk self.bus.getAddress(@bitCast(u16, self.hl) +% 1);
            },
            .AF => {
                self.write("AF, ");
                break :blk @ptrCast(*u16, &self.af);
            },
            .BC => {
                self.write("BC, ");
                break :blk @ptrCast(*u16, &self.bc);
            },
            .DE => {
                self.write("DE, ");
                break :blk @ptrCast(*u16, &self.de);
            },
            .HL => {
                self.write("HL, ");
                break :blk @ptrCast(*u16, &self.hl);
            },
            .SP => {
                self.write("SP, ");
                break :blk &self.sp;
            },
            .PC => {
                self.write("PC, ");
                break :blk &self.pc;
            },
        }
    };
}

fn getArg(self: *Self, comptime arg: Argument, comptime T: type) T {
    return blk: {
        switch (arg) {
            .a => {
                self.write("A");
                break :blk self.af.a;
            },
            .f => {
                self.write("F");
                break :blk self.af.f;
            },
            .b => {
                self.write("B");
                break :blk self.bc.a;
            },
            .c => {
                self.write("C");
                break :blk self.bc.b;
            },
            .d => {
                self.write("D");
                break :blk self.de.a;
            },
            .e => {
                self.write("E");
                break :blk self.de.b;
            },
            .h => {
                self.write("H");
                break :blk self.hl.a;
            },
            .l => {
                self.write("L");
                break :blk self.hl.b;
            },
            .phl => {
                self.write("(HL)");
                break :blk self.bus.getAddress(@bitCast(u16, self.hl)).*;
            },
            .pbc => {
                self.write("(BC)");
                break :blk self.bus.getAddress(@bitCast(u16, self.bc)).*;
            },
            .pde => {
                self.write("(DE)");
                break :blk self.bus.getAddress(@bitCast(u16, self.de)).*;
            },
            .phlp => {
                self.write("(HL+)");
                self.hl.add(1);
                break :blk self.bus.getAddress(@bitCast(u16, self.hl) -% 1).*;
            },
            .phlm => {
                self.write("(HL-)");
                self.hl.sub(1);
                break :blk self.bus.getAddress(@bitCast(u16, self.hl) +% 1).*;
            },
            .p => |p| {
                self.writeWithArg("0x{X:0>2}", .{p});
                break :blk self.bus.getAddress(p).*;
            },
            .immU8 => {
                const b = self.immU8();
                self.writeWithArg("0x{X:0>2}", .{b});
                break :blk b;
            },
            .valU8 => |v| {
                self.writeWithArg("0x{X:0>2}", .{v});
                break :blk v;
            },
            .af => {
                self.write("AF");
                break :blk @bitCast(u16, self.af);
            },
            .bc => {
                self.write("BC");
                break :blk @bitCast(u16, self.bc);
            },
            .de => {
                self.write("CD");
                break :blk @bitCast(u16, self.de);
            },
            .hl => {
                self.write("DE");
                break :blk @bitCast(u16, self.hl);
            },
            .immU16 => {
                const b = self.immU16();
                self.writeWithArg("0x{X:0>4}", .{b});
                break :blk b;
            },
            .valU16 => |v| {
                self.writeWithArg("0x{X:0>4}", .{v});
                break :blk v;
            },
            .offsetImmU8 => |v| {
                const val = self.immU8();
                self.writeWithArg("(0x{X:0>4} 0x{X:0>4} )", .{ v, v + @as(u16, val) });
                const p = v + @as(u16, val);
                break :blk self.bus.getAddress(p).*;
            },
            .spOffsetImmI8 => {
                const v = @bitCast(i8, self.immU8());
                const d = self.sp + @as(u16, v);
                self.writeWithArg("SP+0x{X:0>4}", .{d});
                break :blk d;
            },
        }
    };
}

fn write(self: *Self, comptime msg: []const u8) void {
    _ = self._writer.write(msg) catch {};
    self._writer.flush() catch std.debug.print("Failed to flush self._writer", .{});
}

fn writeWithArg(self: *Self, comptime format: []const u8, args: anytype) void {
    std.fmt.format(self._writer.unbuffered_writer, format, args) catch {};
    self._writer.flush() catch std.debug.print("Failed to flush self._writer", .{});
}

test "cpu registers" {
    const testing = std.testing;
    var writer = std.io.bufferedWriter(std.io.getStdOut().writer());

    var bus = Bus.init(undefined);
    var ppu = Ppu.init();
    var cpu = Self.init(&bus, &ppu, &writer);
    cpu.af.f.z = true;

    const val: u8 = 0b10000000;
    try testing.expectEqual(val, @bitCast(u8, cpu.af.f));
}

test {
    std.testing.refAllDecls(@This());
}

test "compare afreg and regu16 typeinfo" {
    std.debug.assert(@typeInfo(regu16).Struct.fields[0].alignment == @alignOf(u16));
    std.debug.assert(@typeInfo(regu16).Struct.fields[1].alignment == @alignOf(u16));
}
