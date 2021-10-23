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
    const data = self.bus.getAddress(self.pc);
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
        0x0A => self.ld(Registers.A, Argument.pbc, u8),
        0x1A => self.ld(Registers.A, Argument.pde, u8),
        0x2A => self.ld(Registers.A, Argument.phlp, u8),
        0x3A => self.ld(Registers.A, Argument.phlm, u8),
        0x3E => self.ld(Registers.A, Argument{ .valU8 = self.immU8() }, u8),
        0x78 => self.ld(Registers.A, Argument.b, u8),
        0x79 => self.ld(Registers.A, Argument.c, u8),
        0x7A => self.ld(Registers.A, Argument.d, u8),
        0x7B => self.ld(Registers.A, Argument.e, u8),
        0x7C => self.ld(Registers.A, Argument.h, u8),
        0x7D => self.ld(Registers.A, Argument.l, u8),
        0x7E => self.ld(Registers.A, Argument.phl, u8),
        0x7F => self.ld(Registers.A, Argument.a, u8),
        0xF0 => self.ld(Registers.A, Argument{ .addrOffset = .{ .base = 0xFF00, .offset = self.immU8() } }, u8),
        0xF1 => self.ld(Registers.A, Argument{ .addrOffset = .{ .base = 0xFF00, .offset = self.immU8() } }, u8),
        0x06 => self.ld(Registers.B, Argument{ .valU8 = self.immU8() }, u8),
        0x40 => self.ld(Registers.B, Argument.b, u8),
        0x41 => self.ld(Registers.B, Argument.c, u8),
        0x42 => self.ld(Registers.B, Argument.d, u8),
        0x43 => self.ld(Registers.B, Argument.e, u8),
        0x44 => self.ld(Registers.B, Argument.h, u8),
        0x45 => self.ld(Registers.B, Argument.l, u8),
        0x46 => self.ld(Registers.B, Argument.phl, u8),
        0x47 => self.ld(Registers.B, Argument.a, u8),
        0x0E => self.ld(Registers.C, Argument{ .valU8 = self.immU8() }, u8),
        0x48 => self.ld(Registers.C, Argument.b, u8),
        0x49 => self.ld(Registers.C, Argument.c, u8),
        0x4A => self.ld(Registers.C, Argument.d, u8),
        0x4B => self.ld(Registers.C, Argument.e, u8),
        0x4C => self.ld(Registers.C, Argument.h, u8),
        0x4D => self.ld(Registers.C, Argument.l, u8),
        0x4E => self.ld(Registers.C, Argument.phl, u8),
        0x4F => self.ld(Registers.C, Argument.a, u8),
        0x16 => self.ld(Registers.D, Argument{ .valU8 = self.immU8() }, u8),
        0x50 => self.ld(Registers.D, Argument.b, u8),
        0x51 => self.ld(Registers.D, Argument.c, u8),
        0x52 => self.ld(Registers.D, Argument.d, u8),
        0x53 => self.ld(Registers.D, Argument.e, u8),
        0x54 => self.ld(Registers.D, Argument.h, u8),
        0x55 => self.ld(Registers.D, Argument.l, u8),
        0x56 => self.ld(Registers.D, Argument.phl, u8),
        0x57 => self.ld(Registers.D, Argument.a, u8),
        0x1E => self.ld(Registers.E, Argument{ .valU8 = self.immU8() }, u8),
        0x58 => self.ld(Registers.E, Argument.b, u8),
        0x59 => self.ld(Registers.E, Argument.c, u8),
        0x5A => self.ld(Registers.E, Argument.d, u8),
        0x5B => self.ld(Registers.E, Argument.e, u8),
        0x5C => self.ld(Registers.E, Argument.h, u8),
        0x5D => self.ld(Registers.E, Argument.l, u8),
        0x5E => self.ld(Registers.E, Argument.phl, u8),
        0x5F => self.ld(Registers.E, Argument.a, u8),
        0x26 => self.ld(Registers.H, Argument{ .valU8 = self.immU8() }, u8),
        0x60 => self.ld(Registers.H, Argument.b, u8),
        0x61 => self.ld(Registers.H, Argument.c, u8),
        0x62 => self.ld(Registers.H, Argument.d, u8),
        0x63 => self.ld(Registers.H, Argument.e, u8),
        0x64 => self.ld(Registers.H, Argument.h, u8),
        0x65 => self.ld(Registers.H, Argument.l, u8),
        0x66 => self.ld(Registers.H, Argument.phl, u8),
        0x67 => self.ld(Registers.H, Argument.a, u8),
        0x2E => self.ld(Registers.L, Argument{ .valU8 = self.immU8() }, u8),
        0x68 => self.ld(Registers.L, Argument.b, u8),
        0x69 => self.ld(Registers.L, Argument.c, u8),
        0x6A => self.ld(Registers.L, Argument.d, u8),
        0x6B => self.ld(Registers.L, Argument.e, u8),
        0x6C => self.ld(Registers.L, Argument.h, u8),
        0x6D => self.ld(Registers.L, Argument.l, u8),
        0x6E => self.ld(Registers.L, Argument.phl, u8),
        0x6F => self.ld(Registers.L, Argument.a, u8),
        0x02 => self.ld(Registers.PBC, Argument.a, u8),
        0x12 => self.ld(Registers.PDE, Argument.a, u8),
        0x70 => self.ld(Registers.PHL, Argument.b, u8),
        0x71 => self.ld(Registers.PHL, Argument.c, u8),
        0x72 => self.ld(Registers.PHL, Argument.d, u8),
        0x73 => self.ld(Registers.PHL, Argument.e, u8),
        0x74 => self.ld(Registers.PHL, Argument.h, u8),
        0x75 => self.ld(Registers.PHL, Argument.l, u8),
        0x77 => self.ld(Registers.PHL, Argument.a, u8),
        0x36 => self.ld(Registers.PHL, Argument{ .valU8 = self.immU8() }, u8),
        0x22 => self.ld(Registers.PHLP, Argument.a, u8),
        0x32 => self.ld(Registers.PHLM, Argument.a, u8),
        0xEA => {
            const b = self.immU16();
            self.writeWithArg("LD 0x{X:0>2}, A", .{b});
            self.bus.writeAddress(b, self.af.a);
        },
        0x01 => self.ld(Registers.BC, Argument{ .valU16 = self.immU16() }, u16),
        0x11 => self.ld(Registers.DE, Argument{ .valU16 = self.immU16() }, u16),
        0x21 => self.ld(Registers.HL, Argument{ .valU16 = self.immU16() }, u16),
        0x31 => self.ld(Registers.SP, Argument{ .valU16 = self.immU16() }, u16),
        0xF9 => self.ld(Registers.SP, Argument.hl, u16),
        0xE0 => {
            const b = self.immU8();
            self.writeWithArg("LD (0xFF00 + 0x{X:0>2}), A", .{b});
            self.bus.writeAddress(0xFF00 + @as(u16, b), self.af.a);
        },
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
            self.bus.writeAddress(val, @truncate(u8, self.sp >> 8));
            self.bus.writeAddress(val + 1, @truncate(u8, self.sp));
        },
        0x0B => self.dec(Registers.BC),
        0x0D => self.dec(Registers.C),
        0x0F => self.rrca(),
        0x10 => @panic("unhandled opcode: 0x10"),
        0x15 => self.dec(Registers.D),
        0x17 => self.rla(),
        0x18 => self.jr(null),
        0x1B => self.dec(Registers.DE),
        0x1D => self.dec(Registers.E),
        0x1F => self.rra(),
        0x20 => self.jr(.NZ),
        0x25 => self.dec(Registers.H),
        0x27 => @panic("unhandled opcode: 0x27"),
        0x28 => self.jr(.Z),
        0x2B => self.dec(Registers.HL),
        0x2D => self.dec(Registers.L),
        0x2F => @panic("unhandled opcode: 0x2F"),
        0x30 => self.jr(.NC),
        0x35 => self.dec(Registers.PHL),
        0x37 => @panic("unhandled opcode: 0x37"),
        0x38 => self.jr(.C),
        0x09 => self.add(Registers.HL, Argument.bc, u16),
        0x19 => self.add(Registers.HL, Argument.de, u16),
        0x29 => self.add(Registers.HL, Argument.hl, u16),
        0x39 => self.add(Registers.HL, Argument.sp, u16),
        0x3B => self.dec(Registers.SP),
        0x3D => self.dec(Registers.A),
        0x3F => @panic("unhandled opcode: 0x3F"),
        0x76 => self.halt(),
        0x80 => self.add(Registers.A, Argument.b, u8),
        0x81 => self.add(Registers.A, Argument.c, u8),
        0x82 => self.add(Registers.A, Argument.d, u8),
        0x83 => self.add(Registers.A, Argument.e, u8),
        0x84 => self.add(Registers.A, Argument.h, u8),
        0x85 => self.add(Registers.A, Argument.l, u8),
        0x86 => self.add(Registers.A, Argument.phl, u8),
        0x87 => self.add(Registers.A, Argument.a, u8),
        0xC6 => self.add(Registers.A, Argument{ .valU8 = self.immU8() }, u8),
        0x88 => self.adc(Argument.b),
        0x89 => self.adc(Argument.c),
        0x8A => self.adc(Argument.d),
        0x8B => self.adc(Argument.e),
        0x8C => self.adc(Argument.h),
        0x8D => self.adc(Argument.l),
        0x8E => self.adc(Argument.phl),
        0x8F => self.adc(Argument.a),
        0xCE => self.adc(Argument{ .valU8 = self.immU8() }),
        0x90 => self.sub(Argument.b),
        0x91 => self.sub(Argument.c),
        0x92 => self.sub(Argument.d),
        0x93 => self.sub(Argument.e),
        0x94 => self.sub(Argument.h),
        0x95 => self.sub(Argument.l),
        0x96 => self.sub(Argument.phl),
        0x97 => self.sub(Argument.a),
        0xD6 => self.sub(Argument{ .valU8 = self.immU8() }),
        0x98 => @panic("unhandled opcode: 0x98"),
        0x99 => @panic("unhandled opcode: 0x99"),
        0x9A => @panic("unhandled opcode: 0x9A"),
        0x9B => @panic("unhandled opcode: 0x9B"),
        0x9C => @panic("unhandled opcode: 0x9C"),
        0x9D => @panic("unhandled opcode: 0x9D"),
        0x9E => @panic("unhandled opcode: 0x9E"),
        0x9F => @panic("unhandled opcode: 0x9F"),
        0xA0 => self.andReg(Argument.b),
        0xA1 => self.andReg(Argument.c),
        0xA2 => self.andReg(Argument.d),
        0xA3 => self.andReg(Argument.e),
        0xA4 => self.andReg(Argument.h),
        0xA5 => self.andReg(Argument.l),
        0xA6 => self.andReg(Argument.phl),
        0xA7 => self.andReg(Argument.a),
        0xE6 => self.andReg(Argument{ .valU8 = self.immU8() }),
        0xA8 => self.xor(Argument.b),
        0xA9 => self.xor(Argument.c),
        0xAA => self.xor(Argument.d),
        0xAB => self.xor(Argument.e),
        0xAC => self.xor(Argument.h),
        0xAD => self.xor(Argument.l),
        0xAE => self.xor(Argument.phl),
        0xAF => self.xor(Argument.a),
        0xEE => self.xor(Argument{ .valU8 = self.immU8() }),
        0xB0 => self.orReg(Argument.b),
        0xB1 => self.orReg(Argument.c),
        0xB2 => self.orReg(Argument.d),
        0xB3 => self.orReg(Argument.e),
        0xB4 => self.orReg(Argument.h),
        0xB5 => self.orReg(Argument.l),
        0xB6 => self.orReg(Argument.phl),
        0xB7 => self.orReg(Argument.a),
        0xF6 => self.orReg(Argument{ .valU8 = self.immU8() }),
        0xB8 => self.cp(Argument.b),
        0xB9 => self.cp(Argument.c),
        0xBA => self.cp(Argument.d),
        0xBB => self.cp(Argument.e),
        0xBC => self.cp(Argument.h),
        0xBD => self.cp(Argument.l),
        0xBE => self.cp(Argument.phl),
        0xBF => self.cp(Argument.a),
        0xFE => self.cp(Argument{ .valU8 = self.immU8() }),
        0xC1 => @panic("unhandled opcode: 0xC1"),
        0xC4 => self.call(.NZ),
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
        0xC7 => self.rst(0x00),
        0xCF => self.rst(0x08),
        0xD7 => self.rst(0x10),
        0xDF => self.rst(0x18),
        0xE7 => self.rst(0x20),
        0xEF => self.rst(0x28),
        0xF7 => self.rst(0x30),
        0xFF => self.rst(0x38),
        0xD0 => @panic("unhandled opcode: 0xD0"),
        0xD4 => self.call(.NC),
        0xC0 => self.ret(Optional.NZ),
        0xC8 => self.ret(Optional.Z),
        0xC9 => self.ret(null),
        0xD1 => self.ret(Optional.NC),
        0xD8 => self.ret(Optional.C),
        0xD9 => @panic("unhandled opcode: 0xD9"),
        0xDC => self.call(.C),
        0xDE => @panic("unhandled opcode: 0xDE"),
        0xE1 => @panic("unhandled opcode: 0xE1"),
        0xE2 => {
            self.write("LD (0xFF00 + C), A");
            self.bus.writeAddress(0xFF00 + @as(u16, self.bc.b), self.af.a);
        },
        0xE8 => {
            const a = self.sp;
            const b = self.immU8();
            self.sp +%= b;
            self.af.f.z = false;
            self.af.f.n = false;
            self.halfCarry(@truncate(u8, a), @truncate(u8, b));
            self.carry(a, b);
        },
        0xC2 => self.jp(Argument{ .valU16 = self.immU16() }, .NZ),
        0xC3 => self.jp(Argument{ .valU16 = self.immU16() }, null),
        0xCA => self.jp(Argument{ .valU16 = self.immU16() }, .Z),
        0xD2 => self.jp(Argument{ .valU16 = self.immU16() }, .NC),
        0xDA => self.jp(Argument{ .valU16 = self.immU16() }, .C),
        0xE9 => self.jp(Argument.hl, null),
        0xF2 => @panic("unhandled opcode: 0xF2"),
        0xF3 => self.di(),
        0xC5 => self.push(Argument.bc),
        0xD5 => self.push(Argument.de),
        0xE5 => self.push(Argument.hl),
        0xF5 => self.push(Argument.af),
        0xF8 => self.ei(),
        0xFA => @panic("unhandled opcode: 0xFA"),
        0xFB => @panic("unhandled opcode: 0xFB"),
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

fn add(self: *Self, comptime field: Registers, comptime arg: Argument, comptime T: type) void {
    self.write("ADD ");
    const reg = self.getReg(field, T);
    const val = self.getArg(arg, T);
    self.setReg(T, field, reg +% val);
    if (T == u8)
        self.af.f.z = self.getReg(field, T) == 0
    else if (field == Registers.SP)
        self.af.a.z = false;
    self.af.f.n = false;
    self.halfCarry(@truncate(u8, reg), @truncate(u8, val));
    self.carry(reg, val);
}

fn andReg(self: *Self, comptime arg: Argument) void {
    self.write("AND A, ");
    const val = self.getArg(arg, u8);
    self.af.a &= val;
    self.af.f.z = self.af.a == 0;
    self.af.f.n = false;
    self.af.f.h = true;
    self.af.f.c = false;
}

fn adc(self: *Self, comptime arg: Argument) void {
    self.write("ADC A, ");
    const val = self.getArg(arg, u8);
    const old_a = self.af.a;
    self.af.a +%= val +% @boolToInt(self.af.f.c);
    self.af.f.z = self.af.a == 0;
    self.af.f.n = false;
    self.halfCarry(old_a, val);
    self.carry(old_a, val);
}

fn bit(self: *Self, comptime b: u8, comptime arg: Argument) void {
    self.writeWithArg("BIT {}, ", .{b});
    const r = self.getArg(arg, u8);
    self.af.f.z = ((r >> b) & 0x01) == 0;
    self.af.f.n = false;
    self.af.f.h = true;
}

fn call(self: *Self, comptime opt: ?Optional) void {
    self.write("CALL ");
    const addr = self.immU16();
    if (opt) |o| {
        self.writeWithArg("{}, ", .{o});
        if (self.check(o)) {
            self.bus.writeAddress(self.sp, @truncate(u8, self.pc & 0x00FF));
            self.sp -%= 1;
            self.bus.writeAddress(self.sp, @truncate(u8, (self.pc & 0xFF00) >> 8));
            self.sp -%= 1;

            self.pc = addr;
        }
    }
    self.writeWithArg("0x{X:0>2}", .{addr});
}

fn cp(self: *Self, comptime arg: Argument) void {
    self.write("CP A, ");
    const n = self.getArg(arg, u8);
    self.halfCarry(self.af.a, n);
    self.af.f.c = self.af.a < n;
    const val = self.af.a -% n;
    self.af.f.z = val == 0;
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
            const r = self.getReg(field, u8);
            self.halfCarry(r, 1);
            self.setReg(u8, field, r -% 1);
            self.af.f.z = self.getReg(field, u8) == 0;
            self.af.f.n = true;
        },
    }
}

fn ei(self: *Self) void {
    self.interrupts_enabled = .SetStart;
}

fn halt(self: *Self) void {
    self.halted = true;
}

// TODO: Prints , after register
fn inc(self: *Self, comptime field: Registers) void {
    self.writeWithArg("INC {}", .{field});
    switch (field) {
        .AF, .BC, .DE, .HL, .PC, .SP => {
            const reg = self.getReg(field, u16);
            self.setReg(u16, field, reg +% 1);
        },
        else => {
            const r = self.getReg(field, u8);
            self.halfCarry(r, 1);
            self.setReg(u8, field, r +% 1);
            self.af.f.z = self.getReg(field, u8) == 0;
            self.af.f.n = false;
        },
    }
}

fn jp(self: *Self, comptime arg: Argument, comptime opt: ?Optional) void {
    const addr = self.getArg(arg, u16);
    self.write("JP ");
    if (opt) |o| {
        self.writeWithArg("{}, ", .{o});
        if (self.check(o))
            self.pc = addr;
    }
    self.writeWithArg("0x{X:0>2}", .{addr});
}

fn jr(self: *Self, comptime opt: ?Optional) void {
    const r = @bitCast(i8, self.immU8());
    const addr = @bitCast(u16, @bitCast(i16, self.pc) +% @as(i16, r));
    self.write("JR ");
    if (opt) |o| {
        self.writeWithArg("{}, 0x{X:0>2}", .{ o, addr });
        if (self.check(o))
            self.pc = addr;
    } else {
        self.writeWithArg("{}", .{addr});
        self.pc = addr;
    }
}

fn ld(self: *Self, comptime field: Registers, comptime arg: Argument, comptime T: type) void {
    self.writeWithArg("LD {}, {}", .{ field, arg });
    self.setReg(T, field, self.getArg(arg, T));
}

fn noop(_: *Self) void {}

fn orReg(self: *Self, comptime arg: Argument) void {
    self.write("OR A, ");
    const val = self.getArg(arg, u8);
    self.af.a |= val;
    self.af.f.z = self.af.a == 0;
    self.af.f.n = false;
    self.af.f.h = false;
    self.af.f.c = false;
}

fn res(self: *Self, comptime b: u8, comptime field: Registers) void {
    const r = self.getReg(field, u8);
    self.setReg(u8, field, r | ~@bitCast(u8, @as(u8, 1) << b));
}

fn ret(self: *Self, comptime opt: ?Optional) void {
    self.write("RET");
    if (opt) |o| {
        self.writeWithArg(" {}", .{o});
        if (!self.check(o))
            return;
    }
    const lsb = self.bus.getAddress(self.sp);
    self.sp -%= 1;
    const msb = self.bus.getAddress(self.sp);
    self.sp -%= 1;
    self.pc = @bitCast(u16, [2]u8{ msb, lsb });
}

fn rl(self: *Self, comptime field: Registers) void {
    const reg = self.getReg(field, u8);
    const c = @boolToInt(self.af.f.c);
    self.af.f.c = (reg >> 7 == 1);
    self.af.f.n = false;
    self.af.f.h = false;
    self.setReg(u8, field, reg << 1 | c);
    self.af.f.z = self.getReg(field, u8) == 0;
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
    self.af.f.c = (reg >> 7 == 1);
    self.af.f.n = false;
    self.af.f.h = false;
    self.setReg(u8, field, std.math.rotl(u8, 1, reg));
    self.af.f.z = self.getReg(field, u8) == 0;
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
    self.af.f.c = (reg & 0x01 == 1);
    self.af.f.n = false;
    self.af.f.h = false;
    self.setReg(u8, field, @as(u8, c) << 7 | reg >> 1);
    self.af.f.z = self.getReg(field, u8) == 0;
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
    self.af.f.c = (reg & 0x01 == 1);
    self.af.f.n = false;
    self.af.f.h = false;
    self.setReg(u8, field, std.math.rotr(u8, 1, reg));
    self.af.f.z = self.getReg(field, u8) == 0;
}

fn rrca(self: *Self) void {
    const reg = &self.af.a;
    self.af.f.c = (reg.* & 0x01 == 1);
    self.af.f.n = false;
    self.af.f.h = false;
    reg.* = std.math.rotr(u8, 1, reg.*);
    self.af.f.z = reg.* == 0;
}

fn rst(self: *Self, comptime addr: u16) void {
    self.writeWithArg("RST 0x{X:0>2}", .{addr});
    const b = @bitCast([2]u8, self.pc);
    self.bus.writeAddress(self.sp, b[0]);
    self.sp -%= 1;
    self.bus.writeAddress(self.sp, b[1]);
    self.sp -%= 1;
}

fn set(self: *Self, comptime b: u8, comptime field: Registers) void {
    const a = self.getReg(field, u8);
    self.setReg(u8, field, a | (1 << b));
}

fn sla(self: *Self, comptime field: Registers) void {
    const r = self.getReg(field, u8);
    self.af.f.c = (r >> 7) == 1;
    self.af.f.n = false;
    self.af.f.h = false;
    self.setReg(u8, field, r << 1);
    self.af.f.z = self.getReg(field, u8) == 0;
}

fn sra(self: *Self, comptime field: Registers) void {
    const r = self.getReg(field, u8);
    self.af.f.c = (r & 0x01) == 1;
    self.af.f.n = false;
    self.af.f.h = false;
    self.setReg(u8, field, (r >> 1) | (r & 0x80));
    self.af.f.z = self.getReg(field, u8) == 0;
}

fn srl(self: *Self, comptime field: Registers) void {
    const r = self.getReg(field, u8);
    self.af.f.c = (r & 0x01) == 1;
    self.af.f.n = false;
    self.af.f.h = false;
    self.setReg(u8, field, r >> 1);
    self.af.f.z = self.getReg(field, u8) == 0;
}

fn sub(self: *Self, comptime arg: Argument) void {
    self.write("SUB A, ");
    const val = self.getArg(arg, u8);
    self.af.f.z = self.af.a == 0;
    self.af.f.n = true;
    self.halfCarry(self.af.a, val);
    self.carry(self.af.a, val);

    self.af.a -%= val;
}

fn swap(self: *Self, comptime field: Registers) void {
    self.writeWithArg("SWAP {}", .{field});
    const r = self.getReg(field, u8);
    self.setReg(u8, field, (r & 0x0F) << 4 | (r & 0xF0) >> 4);
    self.af.f.c = false;
    self.af.f.n = false;
    self.af.f.h = false;
    self.af.f.z = self.getReg(field, u8) == 0;
}

fn push(self: *Self, comptime arg: Argument) void {
    self.write("PUSH ");
    const v = self.getArg(arg, u16);
    self.bus.writeAddress(self.sp, @truncate(u8, v & 0xFF00));
    self.sp -%= 1;
    self.bus.writeAddress(self.sp, @truncate(u8, v & 0x00FF));
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

fn getReg(self: *Self, comptime field: Registers, comptime T: type) T {
    return blk: {
        switch (field) {
            .A => break :blk self.af.a,
            .F => break :blk self.af.f,
            .B => break :blk self.bc.a,
            .C => break :blk self.bc.b,
            .D => break :blk self.de.a,
            .E => break :blk self.de.b,
            .H => break :blk self.hl.a,
            .L => break :blk self.hl.b,
            .PBC => break :blk self.bus.getAddress(@bitCast(u16, self.bc)),
            .PDE => break :blk self.bus.getAddress(@bitCast(u16, self.de)),
            .PHL => break :blk self.bus.getAddress(@bitCast(u16, self.hl)),
            .PHLP => {
                self.hl.add(1);
                break :blk self.bus.getAddress(@bitCast(u16, self.hl) -% 1);
            },
            .PHLM => {
                self.hl.sub(1);
                break :blk self.bus.getAddress(@bitCast(u16, self.hl) +% 1);
            },
            .AF => break :blk self.af,
            .BC => break :blk @bitCast(u16, self.bc),
            .DE => break :blk @bitCast(u16, self.de),
            .HL => break :blk @bitCast(u16, self.hl),
            .SP => break :blk @bitCast(u16, self.sp),
            .PC => break :blk @bitCast(u16, self.pc),
        }
    };
}

fn setReg(self: *Self, comptime T: type, comptime field: Registers, n: T) void {
    switch (field) {
        .A => self.af.a = n,
        .F => self.af.f = n,
        .B => self.bc.a = n,
        .C => self.bc.b = n,
        .D => self.de.a = n,
        .E => self.de.b = n,
        .H => self.hl.a = n,
        .L => self.hl.b = n,
        .PBC => self.bus.writeAddress(@bitCast(u16, self.bc), n),
        .PDE => self.bus.writeAddress(@bitCast(u16, self.de), n),
        .PHL => self.bus.writeAddress(@bitCast(u16, self.hl), n),
        .PHLP => {
            self.hl.add(1);
            self.bus.writeAddress(@bitCast(u16, self.hl) -% 1, n);
        },
        .PHLM => {
            self.hl.sub(1);
            self.bus.writeAddress(@bitCast(u16, self.hl) +% 1, n);
        },
        .AF => self.af = @bitCast(afreg, n),
        .BC => self.bc = @bitCast(regu16, n),
        .DE => self.de = @bitCast(regu16, n),
        .HL => self.hl = @bitCast(regu16, n),
        .SP => self.sp = n,
        .PC => self.pc = n,
    }
}

fn getArg(self: *Self, comptime arg: Argument, comptime T: type) T {
    return blk: {
        switch (arg) {
            .a => break :blk self.af.a,
            .f => break :blk self.af.f,
            .b => break :blk self.bc.a,
            .c => break :blk self.bc.b,
            .d => break :blk self.de.a,
            .e => break :blk self.de.b,
            .h => break :blk self.hl.a,
            .l => break :blk self.hl.b,
            .phl => break :blk self.bus.getAddress(@bitCast(u16, self.hl)),
            .pbc => break :blk self.bus.getAddress(@bitCast(u16, self.bc)),
            .pde => break :blk self.bus.getAddress(@bitCast(u16, self.de)),
            .phlp => {
                self.hl.add(1);
                break :blk self.bus.getAddress(@bitCast(u16, self.hl) -% 1);
            },
            .phlm => {
                self.hl.sub(1);
                break :blk self.bus.getAddress(@bitCast(u16, self.hl) +% 1);
            },
            .p => |p| break :blk self.bus.getAddress(p),
            .valU8 => |v| break :blk v,
            .af => break :blk @bitCast(u16, self.af),
            .bc => break :blk @bitCast(u16, self.bc),
            .de => break :blk @bitCast(u16, self.de),
            .hl => break :blk @bitCast(u16, self.hl),
            .sp => break :blk self.sp,
            .pc => break :blk self.pc,
            .valU16 => |v| break :blk v,
            .addrOffset => |v| break :blk self.bus.getAddress(v.base +% v.offset),
            .spOffset => break :blk self.sp + @as(u16, @bitCast(i8, self.immU8())),
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
    var ppu = try Ppu.init(&bus);
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
