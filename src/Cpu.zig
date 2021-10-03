const std = @import("std");
const debug = @import("utils.zig").debugAddr;
const Self = @This();
const Utils = @import("utils.zig");
const Registers = Utils.Registers;
const Bus = @import("Bus.zig");
const Ppu = @import("Ppu.zig");

const regu16 = packed struct {
    a: u8 = 0x00,
    b: u8 = 0x00,

    fn add(self: *regu16, rhs: u16) void {
        self.* = @bitCast(regu16, @bitCast(u16, self.*) +% rhs);
    }
    fn sub(self: *regu16, rhs: u16) void {
        self.* = @bitCast(regu16, @bitCast(u16, self.*) -% rhs);
    }
};
const afreg = packed struct {
    a: u8 = 0x00,
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

current_frame: ?anyframe->void = null,

interrupts_enabled: enum { Set, SetPending, SetStart, UnsetStart, UnsetPending, Unset } = .Unset,

halted: bool = false,

pub fn init(bus: *Bus, ppu: *Ppu) Self {
    return .{
        .bus = bus,
        .ppu = ppu,
    };
}

fn nextInstruction(self: *Self) u8 {
    const val = self.pc;
    self.pc += 1;
    return self.bus.getAddress(val).*;
}

inline fn imm_u8(self: *Self) u8 {
    return self.nextInstruction();
}

inline fn imm_u16(self: *Self) u16 {
    const b1 = self.nextInstruction();
    const b2 = self.nextInstruction();
    return @bitCast(u16, [2]u8{ b2, b1 });
}

pub fn clock(self: *Self) void {
    if (self.current_frame) |frame| {
        resume frame;
    } else {
        const opcode = self.nextInstruction();

        debug("Matching opcode:", .{opcode});
        switch (opcode) {
            0x00 => self.noop(),
            0x01 => self.ldU16(Registers.BC, self.imm_u16()),
            0x02 => self.ldU8(Registers.PBC, self.af.a),
            0x03 => self.inc(Registers.BC),
            0x04 => self.inc(Registers.B),
            0x05 => self.dec(Registers.B),
            0x06 => self.ldU8(Registers.B, self.imm_u8()),
            0x07 => @panic("unhandled opcode: 0x07"),
            0x08 => {
                const val = self.imm_u16();
                self.bus.getAddress(val).* = @truncate(u8, self.sp >> 8);
                self.bus.getAddress(val + 1).* = @truncate(u8, self.sp);
            },
            0x09 => self.addU16(@bitCast(u16, self.bc)),
            0x0A => self.ldU8(Registers.A, self.bus.getAddress(@bitCast(u16, self.bc)).*),
            0x0B => self.dec(Registers.BC),
            0x0C => self.inc(Registers.C),
            0x0D => self.dec(Registers.C),
            0x0E => self.ldU8(Registers.C, self.imm_u8()),
            0x0F => @panic("unhandled opcode: 0x0F"),
            0x10 => @panic("unhandled opcode: 0x10"),
            0x11 => self.ldU16(Registers.DE, self.imm_u16()),
            0x12 => @panic("unhandled opcode: 0x12"),
            0x13 => self.inc(Registers.DE),
            0x14 => self.inc(Registers.D),
            0x15 => self.dec(Registers.D),
            0x16 => @panic("unhandled opcode: 0x16"),
            0x17 => @panic("unhandled opcode: 0x17"),
            0x18 => @panic("unhandled opcode: 0x18"),
            0x19 => self.addU16(@bitCast(u16, self.de)),
            0x1A => self.ldU8(Registers.A, self.bus.getAddress(@bitCast(u16, self.de)).*),
            0x1B => self.dec(Registers.DE),
            0x1C => self.inc(Registers.E),
            0x1D => self.dec(Registers.E),
            0x1E => self.ldU8(Registers.E, self.imm_u8()),
            0x1F => @panic("unhandled opcode: 0x1F"),
            0x20 => @panic("unhandled opcode: 0x20"),
            0x21 => self.ldU16(Registers.HL, self.imm_u16()),
            0x22 => {
                self.ldU8(Registers.PHL, self.af.a);
                self.hl.add(1);
            },
            0x23 => self.inc(Registers.HL),
            0x24 => self.inc(Registers.H),
            0x25 => self.dec(Registers.H),
            0x26 => @panic("unhandled opcode: 0x26"),
            0x27 => @panic("unhandled opcode: 0x27"),
            0x28 => @panic("unhandled opcode: 0x28"),
            0x29 => self.addU16(@bitCast(u16, self.hl)),
            0x2A => {
                self.ldU8(Registers.A, self.bus.getAddress(@bitCast(u16, self.hl)).*);
                self.hl.add(1);
            },
            0x2B => self.dec(Registers.HL),
            0x2C => self.inc(Registers.L),
            0x2D => self.dec(Registers.L),
            0x2E => self.ldU8(Registers.L, self.imm_u8()),
            0x2F => @panic("unhandled opcode: 0x2F"),
            0x30 => @panic("unhandled opcode: 0x30"),
            0x31 => self.ldU16(Registers.SP, self.imm_u16()),
            0x32 => {
                self.ldU8(Registers.PHL, self.af.a);
                self.hl.sub(1);
            },
            0x33 => self.inc(Registers.SP),
            0x34 => self.inc(Registers.PHL),
            0x35 => self.dec(Registers.PHL),
            0x36 => @panic("unhandled opcode: 0x36"),
            0x37 => @panic("unhandled opcode: 0x37"),
            0x38 => @panic("unhandled opcode: 0x38"),
            0x39 => self.addU16(self.sp),
            0x3A => {
                self.ldU8(Registers.A, self.bus.getAddress(@bitCast(u16, self.hl)).*);
                self.hl.sub(1);
            },
            0x3B => self.dec(Registers.SP),
            0x3C => self.inc(Registers.A),
            0x3D => self.dec(Registers.A),
            0x3E => self.ldU8(Registers.A, self.imm_u8()),
            0x3F => @panic("unhandled opcode: 0x3F"),
            0x40 => self.ldU8(Registers.B, self.bc.a),
            0x41 => self.ldU8(Registers.B, self.bc.b),
            0x42 => self.ldU8(Registers.B, self.de.a),
            0x43 => self.ldU8(Registers.B, self.de.b),
            0x44 => self.ldU8(Registers.B, self.hl.a),
            0x45 => self.ldU8(Registers.B, self.hl.b),
            0x46 => self.ldU8(Registers.B, self.bus.getAddress(@bitCast(u16, self.hl)).*),
            0x47 => self.ldU8(Registers.B, self.af.a),
            0x48 => self.ldU8(Registers.C, self.bc.a),
            0x49 => self.ldU8(Registers.C, self.bc.b),
            0x4A => self.ldU8(Registers.C, self.de.a),
            0x4B => self.ldU8(Registers.C, self.de.b),
            0x4C => self.ldU8(Registers.C, self.hl.a),
            0x4D => self.ldU8(Registers.C, self.hl.b),
            0x4E => self.ldU8(Registers.C, self.bus.getAddress(@bitCast(u16, self.hl)).*),
            0x4F => self.ldU8(Registers.C, self.af.a),
            0x50 => self.ldU8(Registers.D, self.bc.a),
            0x51 => self.ldU8(Registers.D, self.bc.b),
            0x52 => self.ldU8(Registers.D, self.de.a),
            0x53 => self.ldU8(Registers.D, self.de.b),
            0x54 => self.ldU8(Registers.D, self.hl.a),
            0x55 => self.ldU8(Registers.D, self.hl.b),
            0x56 => self.ldU8(Registers.D, self.bus.getAddress(@bitCast(u16, self.hl)).*),
            0x57 => self.ldU8(Registers.D, self.af.a),
            0x58 => self.ldU8(Registers.E, self.bc.a),
            0x59 => self.ldU8(Registers.E, self.bc.b),
            0x5A => self.ldU8(Registers.E, self.de.a),
            0x5B => self.ldU8(Registers.E, self.de.b),
            0x5C => self.ldU8(Registers.E, self.hl.a),
            0x5D => self.ldU8(Registers.E, self.hl.b),
            0x5E => self.ldU8(Registers.E, self.bus.getAddress(@bitCast(u16, self.hl)).*),
            0x5F => self.ldU8(Registers.E, self.af.a),
            0x60 => self.ldU8(Registers.H, self.bc.a),
            0x61 => self.ldU8(Registers.H, self.bc.b),
            0x62 => self.ldU8(Registers.H, self.de.a),
            0x63 => self.ldU8(Registers.H, self.de.b),
            0x64 => self.ldU8(Registers.H, self.hl.a),
            0x65 => self.ldU8(Registers.H, self.hl.b),
            0x66 => self.ldU8(Registers.H, self.bus.getAddress(@bitCast(u16, self.hl)).*),
            0x67 => self.ldU8(Registers.H, self.af.a),
            0x68 => self.ldU8(Registers.L, self.bc.a),
            0x69 => self.ldU8(Registers.L, self.bc.b),
            0x6A => self.ldU8(Registers.L, self.de.a),
            0x6B => self.ldU8(Registers.L, self.de.b),
            0x6C => self.ldU8(Registers.L, self.hl.a),
            0x6D => self.ldU8(Registers.L, self.hl.b),
            0x6E => self.ldU8(Registers.L, self.bus.getAddress(@bitCast(u16, self.hl)).*),
            0x6F => self.ldU8(Registers.L, self.af.a),
            0x70 => self.bus.getAddress(@bitCast(u16, self.hl)).* = self.bc.a,
            0x71 => self.bus.getAddress(@bitCast(u16, self.hl)).* = self.bc.b,
            0x72 => self.bus.getAddress(@bitCast(u16, self.hl)).* = self.de.a,
            0x73 => self.bus.getAddress(@bitCast(u16, self.hl)).* = self.de.b,
            0x74 => self.bus.getAddress(@bitCast(u16, self.hl)).* = self.hl.a,
            0x75 => self.bus.getAddress(@bitCast(u16, self.hl)).* = self.hl.b,
            0x76 => self.halt(),
            0x77 => self.bus.getAddress(@bitCast(u16, self.hl)).* = self.af.a,
            0x78 => self.ldU8(Registers.L, self.bc.a),
            0x79 => self.ldU8(Registers.L, self.bc.b),
            0x7A => self.ldU8(Registers.L, self.de.a),
            0x7B => self.ldU8(Registers.L, self.de.b),
            0x7C => self.ldU8(Registers.L, self.hl.a),
            0x7D => self.ldU8(Registers.L, self.hl.b),
            0x7E => self.ldU8(Registers.L, self.bus.getAddress(@bitCast(u16, self.hl)).*),
            0x7F => self.ldU8(Registers.L, self.af.a),
            0x80 => self.addU8(self.bc.a),
            0x81 => self.addU8(self.bc.b),
            0x82 => self.addU8(self.de.a),
            0x83 => self.addU8(self.de.b),
            0x84 => self.addU8(self.hl.a),
            0x85 => self.addU8(self.hl.b),
            0x86 => self.addU8(self.bus.getAddress(@bitCast(u16, self.hl)).*),
            0x87 => self.addU8(self.af.a),
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
            0xA8 => self.xor(self.bc.a),
            0xA9 => self.xor(self.bc.b),
            0xAA => self.xor(self.de.a),
            0xAB => self.xor(self.de.b),
            0xAC => self.xor(self.hl.a),
            0xAD => self.xor(self.hl.b),
            0xAE => self.xor(self.bus.getAddress(@bitCast(u16, self.hl)).*),
            0xAF => self.xor(self.af.a),
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
            0xC2 => @panic("unhandled opcode: 0xC2"),
            0xC3 => @panic("unhandled opcode: 0xC3"),
            0xC4 => @panic("unhandled opcode: 0xC4"),
            0xC5 => @panic("unhandled opcode: 0xC5"),
            0xC6 => self.addU8(self.imm_u8()),
            0xC7 => @panic("unhandled opcode: 0xC7"),
            0xC8 => @panic("unhandled opcode: 0xC8"),
            0xC9 => @panic("unhandled opcode: 0xC9"),
            0xCA => @panic("unhandled opcode: 0xCA"),
            0xCB => {
                const nopcode = self.nextInstruction();
                switch (nopcode) {
                    0x00 => @panic("unhandled opcode: 0xCB00"),
                    0x01 => @panic("unhandled opcode: 0xCB01"),
                    0x02 => @panic("unhandled opcode: 0xCB02"),
                    0x03 => @panic("unhandled opcode: 0xCB03"),
                    0x04 => @panic("unhandled opcode: 0xCB04"),
                    0x05 => @panic("unhandled opcode: 0xCB05"),
                    0x06 => @panic("unhandled opcode: 0xCB06"),
                    0x07 => @panic("unhandled opcode: 0xCB07"),
                    0x08 => @panic("unhandled opcode: 0xCB08"),
                    0x09 => @panic("unhandled opcode: 0xCB09"),
                    0x0A => @panic("unhandled opcode: 0xCB0A"),
                    0x0B => @panic("unhandled opcode: 0xCB0B"),
                    0x0C => @panic("unhandled opcode: 0xCB0C"),
                    0x0D => @panic("unhandled opcode: 0xCB0D"),
                    0x0E => @panic("unhandled opcode: 0xCB0E"),
                    0x0F => @panic("unhandled opcode: 0xCB0F"),
                    0x10 => @panic("unhandled opcode: 0xCB10"),
                    0x11 => @panic("unhandled opcode: 0xCB11"),
                    0x12 => @panic("unhandled opcode: 0xCB12"),
                    0x13 => @panic("unhandled opcode: 0xCB13"),
                    0x14 => @panic("unhandled opcode: 0xCB14"),
                    0x15 => @panic("unhandled opcode: 0xCB15"),
                    0x16 => @panic("unhandled opcode: 0xCB16"),
                    0x17 => @panic("unhandled opcode: 0xCB17"),
                    0x18 => @panic("unhandled opcode: 0xCB18"),
                    0x19 => @panic("unhandled opcode: 0xCB19"),
                    0x1A => @panic("unhandled opcode: 0xCB1A"),
                    0x1B => @panic("unhandled opcode: 0xCB1B"),
                    0x1C => @panic("unhandled opcode: 0xCB1C"),
                    0x1D => @panic("unhandled opcode: 0xCB1D"),
                    0x1E => @panic("unhandled opcode: 0xCB1E"),
                    0x1F => @panic("unhandled opcode: 0xCB1F"),
                    0x20 => @panic("unhandled opcode: 0xCB20"),
                    0x21 => @panic("unhandled opcode: 0xCB21"),
                    0x22 => @panic("unhandled opcode: 0xCB22"),
                    0x23 => @panic("unhandled opcode: 0xCB23"),
                    0x24 => @panic("unhandled opcode: 0xCB24"),
                    0x25 => @panic("unhandled opcode: 0xCB25"),
                    0x26 => @panic("unhandled opcode: 0xCB26"),
                    0x27 => @panic("unhandled opcode: 0xCB27"),
                    0x28 => @panic("unhandled opcode: 0xCB28"),
                    0x29 => @panic("unhandled opcode: 0xCB29"),
                    0x2A => @panic("unhandled opcode: 0xCB2A"),
                    0x2B => @panic("unhandled opcode: 0xCB2B"),
                    0x2C => @panic("unhandled opcode: 0xCB2C"),
                    0x2D => @panic("unhandled opcode: 0xCB2D"),
                    0x2E => @panic("unhandled opcode: 0xCB2E"),
                    0x2F => @panic("unhandled opcode: 0xCB2F"),
                    0x30 => @panic("unhandled opcode: 0xCB30"),
                    0x31 => @panic("unhandled opcode: 0xCB31"),
                    0x32 => @panic("unhandled opcode: 0xCB32"),
                    0x33 => @panic("unhandled opcode: 0xCB33"),
                    0x34 => @panic("unhandled opcode: 0xCB34"),
                    0x35 => @panic("unhandled opcode: 0xCB35"),
                    0x36 => @panic("unhandled opcode: 0xCB36"),
                    0x37 => @panic("unhandled opcode: 0xCB37"),
                    0x38 => @panic("unhandled opcode: 0xCB38"),
                    0x39 => @panic("unhandled opcode: 0xCB39"),
                    0x3A => @panic("unhandled opcode: 0xCB3A"),
                    0x3B => @panic("unhandled opcode: 0xCB3B"),
                    0x3C => @panic("unhandled opcode: 0xCB3C"),
                    0x3D => @panic("unhandled opcode: 0xCB3D"),
                    0x3E => @panic("unhandled opcode: 0xCB3E"),
                    0x3F => @panic("unhandled opcode: 0xCB3F"),
                    0x40 => @panic("unhandled opcode: 0xCB40"),
                    0x41 => @panic("unhandled opcode: 0xCB41"),
                    0x42 => @panic("unhandled opcode: 0xCB42"),
                    0x43 => @panic("unhandled opcode: 0xCB43"),
                    0x44 => @panic("unhandled opcode: 0xCB44"),
                    0x45 => @panic("unhandled opcode: 0xCB45"),
                    0x46 => @panic("unhandled opcode: 0xCB46"),
                    0x47 => @panic("unhandled opcode: 0xCB47"),
                    0x48 => @panic("unhandled opcode: 0xCB48"),
                    0x49 => @panic("unhandled opcode: 0xCB49"),
                    0x4A => @panic("unhandled opcode: 0xCB4A"),
                    0x4B => @panic("unhandled opcode: 0xCB4B"),
                    0x4C => @panic("unhandled opcode: 0xCB4C"),
                    0x4D => @panic("unhandled opcode: 0xCB4D"),
                    0x4E => @panic("unhandled opcode: 0xCB4E"),
                    0x4F => @panic("unhandled opcode: 0xCB4F"),
                    0x50 => @panic("unhandled opcode: 0xCB50"),
                    0x51 => @panic("unhandled opcode: 0xCB51"),
                    0x52 => @panic("unhandled opcode: 0xCB52"),
                    0x53 => @panic("unhandled opcode: 0xCB53"),
                    0x54 => @panic("unhandled opcode: 0xCB54"),
                    0x55 => @panic("unhandled opcode: 0xCB55"),
                    0x56 => @panic("unhandled opcode: 0xCB56"),
                    0x57 => @panic("unhandled opcode: 0xCB57"),
                    0x58 => @panic("unhandled opcode: 0xCB58"),
                    0x59 => @panic("unhandled opcode: 0xCB59"),
                    0x5A => @panic("unhandled opcode: 0xCB5A"),
                    0x5B => @panic("unhandled opcode: 0xCB5B"),
                    0x5C => @panic("unhandled opcode: 0xCB5C"),
                    0x5D => @panic("unhandled opcode: 0xCB5D"),
                    0x5E => @panic("unhandled opcode: 0xCB5E"),
                    0x5F => @panic("unhandled opcode: 0xCB5F"),
                    0x60 => @panic("unhandled opcode: 0xCB60"),
                    0x61 => @panic("unhandled opcode: 0xCB61"),
                    0x62 => @panic("unhandled opcode: 0xCB62"),
                    0x63 => @panic("unhandled opcode: 0xCB63"),
                    0x64 => @panic("unhandled opcode: 0xCB64"),
                    0x65 => @panic("unhandled opcode: 0xCB65"),
                    0x66 => @panic("unhandled opcode: 0xCB66"),
                    0x67 => @panic("unhandled opcode: 0xCB67"),
                    0x68 => @panic("unhandled opcode: 0xCB68"),
                    0x69 => @panic("unhandled opcode: 0xCB69"),
                    0x6A => @panic("unhandled opcode: 0xCB6A"),
                    0x6B => @panic("unhandled opcode: 0xCB6B"),
                    0x6C => @panic("unhandled opcode: 0xCB6C"),
                    0x6D => @panic("unhandled opcode: 0xCB6D"),
                    0x6E => @panic("unhandled opcode: 0xCB6E"),
                    0x6F => @panic("unhandled opcode: 0xCB6F"),
                    0x70 => @panic("unhandled opcode: 0xCB70"),
                    0x71 => @panic("unhandled opcode: 0xCB71"),
                    0x72 => @panic("unhandled opcode: 0xCB72"),
                    0x73 => @panic("unhandled opcode: 0xCB73"),
                    0x74 => @panic("unhandled opcode: 0xCB74"),
                    0x75 => @panic("unhandled opcode: 0xCB75"),
                    0x76 => @panic("unhandled opcode: 0xCB76"),
                    0x77 => @panic("unhandled opcode: 0xCB77"),
                    0x78 => @panic("unhandled opcode: 0xCB78"),
                    0x79 => @panic("unhandled opcode: 0xCB79"),
                    0x7A => @panic("unhandled opcode: 0xCB7A"),
                    0x7B => @panic("unhandled opcode: 0xCB7B"),
                    0x7C => @panic("unhandled opcode: 0xCB7C"),
                    0x7D => @panic("unhandled opcode: 0xCB7D"),
                    0x7E => @panic("unhandled opcode: 0xCB7E"),
                    0x7F => @panic("unhandled opcode: 0xCB7F"),
                    0x80 => @panic("unhandled opcode: 0xCB80"),
                    0x81 => @panic("unhandled opcode: 0xCB81"),
                    0x82 => @panic("unhandled opcode: 0xCB82"),
                    0x83 => @panic("unhandled opcode: 0xCB83"),
                    0x84 => @panic("unhandled opcode: 0xCB84"),
                    0x85 => @panic("unhandled opcode: 0xCB85"),
                    0x86 => @panic("unhandled opcode: 0xCB86"),
                    0x87 => @panic("unhandled opcode: 0xCB87"),
                    0x88 => @panic("unhandled opcode: 0xCB88"),
                    0x89 => @panic("unhandled opcode: 0xCB89"),
                    0x8A => @panic("unhandled opcode: 0xCB8A"),
                    0x8B => @panic("unhandled opcode: 0xCB8B"),
                    0x8C => @panic("unhandled opcode: 0xCB8C"),
                    0x8D => @panic("unhandled opcode: 0xCB8D"),
                    0x8E => @panic("unhandled opcode: 0xCB8E"),
                    0x8F => @panic("unhandled opcode: 0xCB8F"),
                    0x90 => @panic("unhandled opcode: 0xCB90"),
                    0x91 => @panic("unhandled opcode: 0xCB91"),
                    0x92 => @panic("unhandled opcode: 0xCB92"),
                    0x93 => @panic("unhandled opcode: 0xCB93"),
                    0x94 => @panic("unhandled opcode: 0xCB94"),
                    0x95 => @panic("unhandled opcode: 0xCB95"),
                    0x96 => @panic("unhandled opcode: 0xCB96"),
                    0x97 => @panic("unhandled opcode: 0xCB97"),
                    0x98 => @panic("unhandled opcode: 0xCB98"),
                    0x99 => @panic("unhandled opcode: 0xCB99"),
                    0x9A => @panic("unhandled opcode: 0xCB9A"),
                    0x9B => @panic("unhandled opcode: 0xCB9B"),
                    0x9C => @panic("unhandled opcode: 0xCB9C"),
                    0x9D => @panic("unhandled opcode: 0xCB9D"),
                    0x9E => @panic("unhandled opcode: 0xCB9E"),
                    0x9F => @panic("unhandled opcode: 0xCB9F"),
                    0xA0 => @panic("unhandled opcode: 0xCBA0"),
                    0xA1 => @panic("unhandled opcode: 0xCBA1"),
                    0xA2 => @panic("unhandled opcode: 0xCBA2"),
                    0xA3 => @panic("unhandled opcode: 0xCBA3"),
                    0xA4 => @panic("unhandled opcode: 0xCBA4"),
                    0xA5 => @panic("unhandled opcode: 0xCBA5"),
                    0xA6 => @panic("unhandled opcode: 0xCBA6"),
                    0xA7 => @panic("unhandled opcode: 0xCBA7"),
                    0xA8 => @panic("unhandled opcode: 0xCBA8"),
                    0xA9 => @panic("unhandled opcode: 0xCBA9"),
                    0xAA => @panic("unhandled opcode: 0xCBAA"),
                    0xAB => @panic("unhandled opcode: 0xCBAB"),
                    0xAC => @panic("unhandled opcode: 0xCBAC"),
                    0xAD => @panic("unhandled opcode: 0xCBAD"),
                    0xAE => @panic("unhandled opcode: 0xCBAE"),
                    0xAF => @panic("unhandled opcode: 0xCBAF"),
                    0xB0 => @panic("unhandled opcode: 0xCBB0"),
                    0xB1 => @panic("unhandled opcode: 0xCBB1"),
                    0xB2 => @panic("unhandled opcode: 0xCBB2"),
                    0xB3 => @panic("unhandled opcode: 0xCBB3"),
                    0xB4 => @panic("unhandled opcode: 0xCBB4"),
                    0xB5 => @panic("unhandled opcode: 0xCBB5"),
                    0xB6 => @panic("unhandled opcode: 0xCBB6"),
                    0xB7 => @panic("unhandled opcode: 0xCBB7"),
                    0xB8 => @panic("unhandled opcode: 0xCBB8"),
                    0xB9 => @panic("unhandled opcode: 0xCBB9"),
                    0xBA => @panic("unhandled opcode: 0xCBBA"),
                    0xBB => @panic("unhandled opcode: 0xCBBB"),
                    0xBC => @panic("unhandled opcode: 0xCBBC"),
                    0xBD => @panic("unhandled opcode: 0xCBBD"),
                    0xBE => @panic("unhandled opcode: 0xCBBE"),
                    0xBF => @panic("unhandled opcode: 0xCBBF"),
                    0xC0 => @panic("unhandled opcode: 0xCBC0"),
                    0xC1 => @panic("unhandled opcode: 0xCBC1"),
                    0xC2 => @panic("unhandled opcode: 0xCBC2"),
                    0xC3 => @panic("unhandled opcode: 0xCBC3"),
                    0xC4 => @panic("unhandled opcode: 0xCBC4"),
                    0xC5 => @panic("unhandled opcode: 0xCBC5"),
                    0xC6 => @panic("unhandled opcode: 0xCBC6"),
                    0xC7 => @panic("unhandled opcode: 0xCBC7"),
                    0xC8 => @panic("unhandled opcode: 0xCBC8"),
                    0xC9 => @panic("unhandled opcode: 0xCBC9"),
                    0xCA => @panic("unhandled opcode: 0xCBCA"),
                    0xCB => @panic("unhandled opcode: 0xCBCB"),
                    0xCC => @panic("unhandled opcode: 0xCBCC"),
                    0xCD => @panic("unhandled opcode: 0xCBCD"),
                    0xCE => @panic("unhandled opcode: 0xCBCE"),
                    0xCF => @panic("unhandled opcode: 0xCBCF"),
                    0xD0 => @panic("unhandled opcode: 0xCBD0"),
                    0xD1 => @panic("unhandled opcode: 0xCBD1"),
                    0xD2 => @panic("unhandled opcode: 0xCBD2"),
                    0xD3 => @panic("unhandled opcode: 0xCBD3"),
                    0xD4 => @panic("unhandled opcode: 0xCBD4"),
                    0xD5 => @panic("unhandled opcode: 0xCBD5"),
                    0xD6 => @panic("unhandled opcode: 0xCBD6"),
                    0xD7 => @panic("unhandled opcode: 0xCBD7"),
                    0xD8 => @panic("unhandled opcode: 0xCBD8"),
                    0xD9 => @panic("unhandled opcode: 0xCBD9"),
                    0xDA => @panic("unhandled opcode: 0xCBDA"),
                    0xDB => @panic("unhandled opcode: 0xCBDB"),
                    0xDC => @panic("unhandled opcode: 0xCBDC"),
                    0xDD => @panic("unhandled opcode: 0xCBDD"),
                    0xDE => @panic("unhandled opcode: 0xCBDE"),
                    0xDF => @panic("unhandled opcode: 0xCBDF"),
                    0xE0 => @panic("unhandled opcode: 0xCBE0"),
                    0xE1 => @panic("unhandled opcode: 0xCBE1"),
                    0xE2 => @panic("unhandled opcode: 0xCBE2"),
                    0xE3 => @panic("unhandled opcode: 0xCBE3"),
                    0xE4 => @panic("unhandled opcode: 0xCBE4"),
                    0xE5 => @panic("unhandled opcode: 0xCBE5"),
                    0xE6 => @panic("unhandled opcode: 0xCBE6"),
                    0xE7 => @panic("unhandled opcode: 0xCBE7"),
                    0xE8 => @panic("unhandled opcode: 0xCBE8"),
                    0xE9 => @panic("unhandled opcode: 0xCBE9"),
                    0xEA => @panic("unhandled opcode: 0xCBEA"),
                    0xEB => @panic("unhandled opcode: 0xCBEB"),
                    0xEC => @panic("unhandled opcode: 0xCBEC"),
                    0xED => @panic("unhandled opcode: 0xCBED"),
                    0xEE => @panic("unhandled opcode: 0xCBEE"),
                    0xEF => @panic("unhandled opcode: 0xCBEF"),
                    0xF0 => @panic("unhandled opcode: 0xCBF0"),
                    0xF1 => @panic("unhandled opcode: 0xCBF1"),
                    0xF2 => @panic("unhandled opcode: 0xCBF2"),
                    0xF3 => @panic("unhandled opcode: 0xCBF3"),
                    0xF4 => @panic("unhandled opcode: 0xCBF4"),
                    0xF5 => @panic("unhandled opcode: 0xCBF5"),
                    0xF6 => @panic("unhandled opcode: 0xCBF6"),
                    0xF7 => @panic("unhandled opcode: 0xCBF7"),
                    0xF8 => @panic("unhandled opcode: 0xCBF8"),
                    0xF9 => @panic("unhandled opcode: 0xCBF9"),
                    0xFA => @panic("unhandled opcode: 0xCBFA"),
                    0xFB => @panic("unhandled opcode: 0xCBFB"),
                    0xFC => @panic("unhandled opcode: 0xCBFC"),
                    0xFD => @panic("unhandled opcode: 0xCBFD"),
                    0xFE => @panic("unhandled opcode: 0xCBFE"),
                    0xFF => @panic("unhandled opcode: 0xCBFF"),
                }
            },
            0xCC => @panic("unhandled opcode: 0xCC"),
            0xCD => @panic("unhandled opcode: 0xCD"),
            0xCE => @panic("unhandled opcode: 0xCE"),
            0xCF => @panic("unhandled opcode: 0xCF"),
            0xD0 => @panic("unhandled opcode: 0xD0"),
            0xD1 => @panic("unhandled opcode: 0xD1"),
            0xD2 => @panic("unhandled opcode: 0xD2"),
            0xD3 => @panic("unhandled opcode: 0xD3"),
            0xD4 => @panic("unhandled opcode: 0xD4"),
            0xD5 => @panic("unhandled opcode: 0xD5"),
            0xD6 => self.sub(self.imm_u8()),
            0xD7 => @panic("unhandled opcode: 0xD7"),
            0xD8 => @panic("unhandled opcode: 0xD8"),
            0xD9 => @panic("unhandled opcode: 0xD9"),
            0xDA => @panic("unhandled opcode: 0xDA"),
            0xDB => @panic("unhandled opcode: 0xDB"),
            0xDC => @panic("unhandled opcode: 0xDC"),
            0xDD => @panic("unhandled opcode: 0xDD"),
            0xDE => @panic("unhandled opcode: 0xDE"),
            0xDF => @panic("unhandled opcode: 0xDF"),
            0xE0 => self.bus.getAddress(0xFF00 + @as(u16, self.imm_u8())).* = self.af.a,
            0xE1 => @panic("unhandled opcode: 0xE1"),
            0xE2 => @panic("unhandled opcode: 0xE2"),
            0xE3 => @panic("unhandled opcode: 0xE3"),
            0xE4 => @panic("unhandled opcode: 0xE4"),
            0xE5 => @panic("unhandled opcode: 0xE5"),
            0xE6 => @panic("unhandled opcode: 0xE6"),
            0xE7 => @panic("unhandled opcode: 0xE7"),
            0xE8 => {
                const a = self.sp;
                const b = self.imm_u8();
                self.sp +%= b;
                self.af.f.z = false;
                self.af.f.n = false;
                self.halfCarry(@truncate(u8, a), @truncate(u8, b));
                self.carry(a, b);
            },
            0xE9 => @panic("unhandled opcode: 0xE9"),
            0xEA => @panic("unhandled opcode: 0xEA"),
            0xEB => @panic("unhandled opcode: 0xEB"),
            0xEC => @panic("unhandled opcode: 0xEC"),
            0xED => @panic("unhandled opcode: 0xED"),
            0xEE => self.xor(self.imm_u8()),
            0xEF => @panic("unhandled opcode: 0xEF"),
            0xF0 => self.ldU8(Registers.A, self.bus.getAddress(0xFF00 + @as(u16, self.imm_u8())).*),
            0xF1 => @panic("unhandled opcode: 0xF1"),
            0xF2 => @panic("unhandled opcode: 0xF2"),
            0xF3 => @panic("unhandled opcode: 0xF3"),
            0xF4 => @panic("unhandled opcode: 0xF4"),
            0xF5 => @panic("unhandled opcode: 0xF5"),
            0xF6 => self.orReg(self.imm_u8()),
            0xF7 => @panic("unhandled opcode: 0xF7"),
            0xF8 => @panic("unhandled opcode: 0xF8"),
            0xF9 => @panic("unhandled opcode: 0xF9"),
            0xFA => @panic("unhandled opcode: 0xFA"),
            0xFB => @panic("unhandled opcode: 0xFB"),
            0xFC => @panic("unhandled opcode: 0xFC"),
            0xFD => @panic("unhandled opcode: 0xFD"),
            0xFE => @panic("unhandled opcode: 0xFE"),
            0xFF => @panic("unhandled opcode: 0xFF"),
        }
    }
}

fn addU8(self: *Self, val: u8) void {
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

fn dec(self: *Self, comptime field: Utils.Registers) void {
    switch (field) {
        .AF => self.af.sub(1),
        .BC => self.bc.sub(1),
        .DE => self.de.sub(1),
        .HL => self.hl.sub(1),
        .PC => self.pc -%= 1,
        .SP => self.sp -%= 1,
        else => {
            const res = blk: {
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
                    else => unreachable,
                }
            };

            self.af.f.z = res == 0;
            self.af.f.n = true;
            self.halfCarry(res +% 1, 1);
        },
    }
}

fn halt(self: *Self) void {
    self.halted = true;
}

fn inc(self: *Self, comptime field: Utils.Registers) void {
    switch (field) {
        .AF => self.af.add(1),
        .BC => self.bc.add(1),
        .DE => self.de.add(1),
        .HL => self.hl.add(1),
        .PC => self.pc +%= 1,
        .SP => self.sp +%= 1,
        else => {
            const res = blk: {
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
                    else => unreachable,
                }
            };
            self.af.f.z = res == 0;
            self.af.f.n = false;
            self.halfCarry(res -% 1, 1);
        },
    }
}

fn ldU16(self: *Self, comptime field: Utils.Registers, val: u16) void {
    switch (field) {
        .AF => self.af = @bitCast(regu16, val),
        .BC => self.bc = @bitCast(regu16, val),
        .DE => self.de = @bitCast(regu16, val),
        .HL => self.hl = @bitCast(regu16, val),
        .PC => self.pc = val,
        .SP => self.sp = val,
        else => @compileError("Tried to load u16 into u8 register"),
    }
}

fn ldU8(self: *Self, comptime field: Utils.Registers, val: u8) void {
    switch (field) {
        .A => self.af.a = val,
        .F => self.af.f = val,
        .B => self.bc.a = val,
        .C => self.bc.b = val,
        .D => self.de.a = val,
        .E => self.de.b = val,
        .H => self.hl.a = val,
        .L => self.hl.b = val,
        .PHL => self.bus.getAddress(@bitCast(u16, self.hl)).* = val,
        .PBC => self.bus.getAddress(@bitCast(u16, self.bc)).* = val,
        .PDE => self.bus.getAddress(@bitCast(u16, self.de)).* = val,
        else => @compileError("Tried to load u8 into u16 register"),
    }
}

fn noop(_: *Self) void {}

fn orReg(self: *Self, val: u8) void {
    self.af.a |= val;
    self.af.f.z = self.af.a == 0;
    self.af.f.n = false;
    self.af.f.h = false;
    self.af.f.c = false;
}

fn sub(self: *Self, val: u8) void {
    self.af.f.z = self.af.a == 0;
    self.af.f.n = true;
    self.halfCarry(self.af.a, val);
    self.carry(self.af.a, val);

    self.af.a -%= val;
}

fn xor(self: *Self, val: u8) void {
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

test "cpu registers" {
    const testing = std.testing;

    var bus = Bus.init(undefined);
    var ppu = Ppu.init();
    var cpu = Self.init(&bus, &ppu);
    cpu.af.f.z = 1;

    const val: u8 = 0b10000000;
    try testing.expectEqual(val, @bitCast(u8, cpu.af.f));
}

test {
    std.testing.refAllDecls(@This());
}
