const std = @import("std");
const debug = @import("utils.zig").debug_addr;
const Self = @This();
const Utils = @import("utils.zig");
const Registers = Utils.Registers;
const Bus = @import("Bus.zig");
const Ppu = @import("Ppu.zig");

const regu16 = packed struct {
    a: u8 = 0x00,
    b: u8 = 0x00,

    fn add(self: *regu16, rhs: u16) void {
        self.* = @bitCast(regu16, @bitCast(u16, self.*) + rhs);
    }
    fn sub(self: *regu16, rhs: u16) void {
        self.* = @bitCast(regu16, @bitCast(u16, self.*) - rhs);
    }
};
const afreg = packed struct {
    a: u8 = 0x00,
    f: packed struct { _: u4 = 0x0, c: u1 = 0, h: u1 = 0, n: u1 = 0, z: u1 = 0 } = .{},

    fn add(self: *afreg, rhs: u16) void {
        self.* = @bitCast(afreg, @bitCast(u16, self.*) + rhs);
    }
    fn sub(self: *afreg, rhs: u16) void {
        self.* = @bitCast(afreg, @bitCast(u16, self.*) - rhs);
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
            0x01 => self.ld_u16(Registers.BC, self.imm_u16()),
            0x02 => self.ld_p(@bitCast(u16, self.bc), self.af.a),
            0x03 => self.inc(Registers.BC),
            0x04 => self.inc(Registers.B),
            0x05 => self.dec(Registers.B),
            0x06 => @panic("unhandled opcode: 0x06"),
            0x07 => @panic("unhandled opcode: 0x07"),
            0x08 => @panic("unhandled opcode: 0x08"),
            0x09 => @panic("unhandled opcode: 0x09"),
            0x0A => @panic("unhandled opcode: 0x0A"),
            0x0B => @panic("unhandled opcode: 0x0B"),
            0x0C => self.inc(Registers.C),
            0x0D => self.dec(Registers.C),
            0x0E => @panic("unhandled opcode: 0x0E"),
            0x0F => @panic("unhandled opcode: 0x0F"),
            0x10 => @panic("unhandled opcode: 0x10"),
            0x11 => @panic("unhandled opcode: 0x11"),
            0x12 => @panic("unhandled opcode: 0x12"),
            0x13 => self.inc(Registers.DE),
            0x14 => self.inc(Registers.D),
            0x15 => self.dec(Registers.D),
            0x16 => @panic("unhandled opcode: 0x16"),
            0x17 => @panic("unhandled opcode: 0x17"),
            0x18 => @panic("unhandled opcode: 0x18"),
            0x19 => @panic("unhandled opcode: 0x19"),
            0x1A => @panic("unhandled opcode: 0x1A"),
            0x1B => @panic("unhandled opcode: 0x1B"),
            0x1C => self.inc(Registers.E),
            0x1D => self.dec(Registers.E),
            0x1E => @panic("unhandled opcode: 0x1E"),
            0x1F => @panic("unhandled opcode: 0x1F"),
            0x20 => @panic("unhandled opcode: 0x20"),
            0x21 => @panic("unhandled opcode: 0x21"),
            0x22 => @panic("unhandled opcode: 0x22"),
            0x23 => self.inc(Registers.HL),
            0x24 => self.inc(Registers.H),
            0x25 => self.dec(Registers.H),
            0x26 => @panic("unhandled opcode: 0x26"),
            0x27 => @panic("unhandled opcode: 0x27"),
            0x28 => @panic("unhandled opcode: 0x28"),
            0x29 => @panic("unhandled opcode: 0x29"),
            0x2A => @panic("unhandled opcode: 0x2A"),
            0x2B => @panic("unhandled opcode: 0x2B"),
            0x2C => self.inc(Registers.L),
            0x2D => self.dec(Registers.L),
            0x2E => @panic("unhandled opcode: 0x2E"),
            0x2F => @panic("unhandled opcode: 0x2F"),
            0x30 => @panic("unhandled opcode: 0x30"),
            0x31 => @panic("unhandled opcode: 0x31"),
            0x32 => @panic("unhandled opcode: 0x32"),
            0x33 => self.inc(Registers.SP),
            0x34 => self.inc(Registers.PHL),
            0x35 => self.dec(Registers.PHL),
            0x36 => @panic("unhandled opcode: 0x36"),
            0x37 => @panic("unhandled opcode: 0x37"),
            0x38 => @panic("unhandled opcode: 0x38"),
            0x39 => @panic("unhandled opcode: 0x39"),
            0x3A => @panic("unhandled opcode: 0x3A"),
            0x3B => @panic("unhandled opcode: 0x3B"),
            0x3C => self.inc(Registers.A),
            0x3D => self.dec(Registers.A),
            0x3E => @panic("unhandled opcode: 0x3E"),
            0x3F => @panic("unhandled opcode: 0x3F"),
            0x40 => @panic("unhandled opcode: 0x40"),
            0x41 => @panic("unhandled opcode: 0x41"),
            0x42 => @panic("unhandled opcode: 0x42"),
            0x43 => @panic("unhandled opcode: 0x43"),
            0x44 => @panic("unhandled opcode: 0x44"),
            0x45 => @panic("unhandled opcode: 0x45"),
            0x46 => @panic("unhandled opcode: 0x46"),
            0x47 => @panic("unhandled opcode: 0x47"),
            0x48 => @panic("unhandled opcode: 0x48"),
            0x49 => @panic("unhandled opcode: 0x49"),
            0x4A => @panic("unhandled opcode: 0x4A"),
            0x4B => @panic("unhandled opcode: 0x4B"),
            0x4C => @panic("unhandled opcode: 0x4C"),
            0x4D => @panic("unhandled opcode: 0x4D"),
            0x4E => @panic("unhandled opcode: 0x4E"),
            0x4F => @panic("unhandled opcode: 0x4F"),
            0x50 => @panic("unhandled opcode: 0x50"),
            0x51 => @panic("unhandled opcode: 0x51"),
            0x52 => @panic("unhandled opcode: 0x52"),
            0x53 => @panic("unhandled opcode: 0x53"),
            0x54 => @panic("unhandled opcode: 0x54"),
            0x55 => @panic("unhandled opcode: 0x55"),
            0x56 => @panic("unhandled opcode: 0x56"),
            0x57 => @panic("unhandled opcode: 0x57"),
            0x58 => @panic("unhandled opcode: 0x58"),
            0x59 => @panic("unhandled opcode: 0x59"),
            0x5A => @panic("unhandled opcode: 0x5A"),
            0x5B => @panic("unhandled opcode: 0x5B"),
            0x5C => @panic("unhandled opcode: 0x5C"),
            0x5D => @panic("unhandled opcode: 0x5D"),
            0x5E => @panic("unhandled opcode: 0x5E"),
            0x5F => @panic("unhandled opcode: 0x5F"),
            0x60 => @panic("unhandled opcode: 0x60"),
            0x61 => @panic("unhandled opcode: 0x61"),
            0x62 => @panic("unhandled opcode: 0x62"),
            0x63 => @panic("unhandled opcode: 0x63"),
            0x64 => @panic("unhandled opcode: 0x64"),
            0x65 => @panic("unhandled opcode: 0x65"),
            0x66 => @panic("unhandled opcode: 0x66"),
            0x67 => @panic("unhandled opcode: 0x67"),
            0x68 => @panic("unhandled opcode: 0x68"),
            0x69 => @panic("unhandled opcode: 0x69"),
            0x6A => @panic("unhandled opcode: 0x6A"),
            0x6B => @panic("unhandled opcode: 0x6B"),
            0x6C => @panic("unhandled opcode: 0x6C"),
            0x6D => @panic("unhandled opcode: 0x6D"),
            0x6E => @panic("unhandled opcode: 0x6E"),
            0x6F => @panic("unhandled opcode: 0x6F"),
            0x70 => @panic("unhandled opcode: 0x70"),
            0x71 => @panic("unhandled opcode: 0x71"),
            0x72 => @panic("unhandled opcode: 0x72"),
            0x73 => @panic("unhandled opcode: 0x73"),
            0x74 => @panic("unhandled opcode: 0x74"),
            0x75 => @panic("unhandled opcode: 0x75"),
            0x76 => @panic("unhandled opcode: 0x76"),
            0x77 => @panic("unhandled opcode: 0x77"),
            0x78 => @panic("unhandled opcode: 0x78"),
            0x79 => @panic("unhandled opcode: 0x79"),
            0x7A => @panic("unhandled opcode: 0x7A"),
            0x7B => @panic("unhandled opcode: 0x7B"),
            0x7C => @panic("unhandled opcode: 0x7C"),
            0x7D => @panic("unhandled opcode: 0x7D"),
            0x7E => @panic("unhandled opcode: 0x7E"),
            0x7F => @panic("unhandled opcode: 0x7F"),
            0x80 => @panic("unhandled opcode: 0x80"),
            0x81 => @panic("unhandled opcode: 0x81"),
            0x82 => @panic("unhandled opcode: 0x82"),
            0x83 => @panic("unhandled opcode: 0x83"),
            0x84 => @panic("unhandled opcode: 0x84"),
            0x85 => @panic("unhandled opcode: 0x85"),
            0x86 => @panic("unhandled opcode: 0x86"),
            0x87 => @panic("unhandled opcode: 0x87"),
            0x88 => @panic("unhandled opcode: 0x88"),
            0x89 => @panic("unhandled opcode: 0x89"),
            0x8A => @panic("unhandled opcode: 0x8A"),
            0x8B => @panic("unhandled opcode: 0x8B"),
            0x8C => @panic("unhandled opcode: 0x8C"),
            0x8D => @panic("unhandled opcode: 0x8D"),
            0x8E => @panic("unhandled opcode: 0x8E"),
            0x8F => @panic("unhandled opcode: 0x8F"),
            0x90 => @panic("unhandled opcode: 0x90"),
            0x91 => @panic("unhandled opcode: 0x91"),
            0x92 => @panic("unhandled opcode: 0x92"),
            0x93 => @panic("unhandled opcode: 0x93"),
            0x94 => @panic("unhandled opcode: 0x94"),
            0x95 => @panic("unhandled opcode: 0x95"),
            0x96 => @panic("unhandled opcode: 0x96"),
            0x97 => @panic("unhandled opcode: 0x97"),
            0x98 => @panic("unhandled opcode: 0x98"),
            0x99 => @panic("unhandled opcode: 0x99"),
            0x9A => @panic("unhandled opcode: 0x9A"),
            0x9B => @panic("unhandled opcode: 0x9B"),
            0x9C => @panic("unhandled opcode: 0x9C"),
            0x9D => @panic("unhandled opcode: 0x9D"),
            0x9E => @panic("unhandled opcode: 0x9E"),
            0x9F => @panic("unhandled opcode: 0x9F"),
            0xA0 => @panic("unhandled opcode: 0xA0"),
            0xA1 => @panic("unhandled opcode: 0xA1"),
            0xA2 => @panic("unhandled opcode: 0xA2"),
            0xA3 => @panic("unhandled opcode: 0xA3"),
            0xA4 => @panic("unhandled opcode: 0xA4"),
            0xA5 => @panic("unhandled opcode: 0xA5"),
            0xA6 => @panic("unhandled opcode: 0xA6"),
            0xA7 => @panic("unhandled opcode: 0xA7"),
            0xA8 => @panic("unhandled opcode: 0xA8"),
            0xA9 => @panic("unhandled opcode: 0xA9"),
            0xAA => @panic("unhandled opcode: 0xAA"),
            0xAB => @panic("unhandled opcode: 0xAB"),
            0xAC => @panic("unhandled opcode: 0xAC"),
            0xAD => @panic("unhandled opcode: 0xAD"),
            0xAE => @panic("unhandled opcode: 0xAE"),
            0xAF => @panic("unhandled opcode: 0xAF"),
            0xB0 => @panic("unhandled opcode: 0xB0"),
            0xB1 => @panic("unhandled opcode: 0xB1"),
            0xB2 => @panic("unhandled opcode: 0xB2"),
            0xB3 => @panic("unhandled opcode: 0xB3"),
            0xB4 => @panic("unhandled opcode: 0xB4"),
            0xB5 => @panic("unhandled opcode: 0xB5"),
            0xB6 => @panic("unhandled opcode: 0xB6"),
            0xB7 => @panic("unhandled opcode: 0xB7"),
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
            0xC6 => @panic("unhandled opcode: 0xC6"),
            0xC7 => @panic("unhandled opcode: 0xC7"),
            0xC8 => @panic("unhandled opcode: 0xC8"),
            0xC9 => @panic("unhandled opcode: 0xC9"),
            0xCA => @panic("unhandled opcode: 0xCA"),
            0xCB => @panic("unhandled opcode: 0xCB"),
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
            0xD6 => @panic("unhandled opcode: 0xD6"),
            0xD7 => @panic("unhandled opcode: 0xD7"),
            0xD8 => @panic("unhandled opcode: 0xD8"),
            0xD9 => @panic("unhandled opcode: 0xD9"),
            0xDA => @panic("unhandled opcode: 0xDA"),
            0xDB => @panic("unhandled opcode: 0xDB"),
            0xDC => @panic("unhandled opcode: 0xDC"),
            0xDD => @panic("unhandled opcode: 0xDD"),
            0xDE => @panic("unhandled opcode: 0xDE"),
            0xDF => @panic("unhandled opcode: 0xDF"),
            0xE0 => @panic("unhandled opcode: 0xE0"),
            0xE1 => @panic("unhandled opcode: 0xE1"),
            0xE2 => @panic("unhandled opcode: 0xE2"),
            0xE3 => @panic("unhandled opcode: 0xE3"),
            0xE4 => @panic("unhandled opcode: 0xE4"),
            0xE5 => @panic("unhandled opcode: 0xE5"),
            0xE6 => @panic("unhandled opcode: 0xE6"),
            0xE7 => @panic("unhandled opcode: 0xE7"),
            0xE8 => @panic("unhandled opcode: 0xE8"),
            0xE9 => @panic("unhandled opcode: 0xE9"),
            0xEA => @panic("unhandled opcode: 0xEA"),
            0xEB => @panic("unhandled opcode: 0xEB"),
            0xEC => @panic("unhandled opcode: 0xEC"),
            0xED => @panic("unhandled opcode: 0xED"),
            0xEE => @panic("unhandled opcode: 0xEE"),
            0xEF => @panic("unhandled opcode: 0xEF"),
            0xF0 => @panic("unhandled opcode: 0xF0"),
            0xF1 => @panic("unhandled opcode: 0xF1"),
            0xF2 => @panic("unhandled opcode: 0xF2"),
            0xF3 => @panic("unhandled opcode: 0xF3"),
            0xF4 => @panic("unhandled opcode: 0xF4"),
            0xF5 => @panic("unhandled opcode: 0xF5"),
            0xF6 => @panic("unhandled opcode: 0xF6"),
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

fn noop(self: *Self) void {
    suspend {
        self.current_frame = @frame();
    }
}

fn ld_u16(self: *Self, comptime field: Utils.Registers, val: u16) void {
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

fn ld_u8(self: *Self, comptime field: Utils.Registers, val: u8) void {
    switch (field) {
        .A => self.af.a = val,
        .F => self.af.f = val,
        .B => self.bc.a = val,
        .C => self.bc.b = val,
        .D => self.de.a = val,
        .E => self.de.b = val,
        .H => self.hl.a = val,
        .L => self.hl.b = val,
        .PHL => self.bus.setAddress(@bitCast(u16, self.hl), val),
        else => @compileError("Tried to load u8 into u16 register"),
    }
}

fn ld_p(self: *Self, addr: u16, val: u8) void {
    self.bus.setAddress(addr, val);
}

fn inc(self: *Self, comptime field: Utils.Registers) void {
    switch (field) {
        .AF => self.af.add(1),
        .BC => self.bc.add(1),
        .DE => self.de.add(1),
        .HL => self.hl.add(1),
        .PHL => self.bus.setAddress(@bitCast(u16, self.hl), self.bus.getAddress(@bitCast(u16, self.hl)).* + 1),
        .PC => self.pc += 1,
        .SP => self.sp += 1,
        .A => self.af.a += 1,
        .F => self.af.f += 1,
        .B => self.bc.a += 1,
        .C => self.bc.b += 1,
        .D => self.de.a += 1,
        .E => self.de.b += 1,
        .H => self.hl.a += 1,
        .L => self.hl.b += 1,
    }
}

fn dec(self: *Self, comptime field: Utils.Registers) void {
    switch (field) {
        .AF => self.af.sub(1),
        .BC => self.bc.sub(1),
        .DE => self.de.sub(1),
        .HL => self.hl.sub(1),
        .PHL => self.bus.setAddress(@bitCast(u16, self.hl), self.bus.getAddress(@bitCast(u16, self.hl)).* - 1),
        .PC => self.pc -= 1,
        .SP => self.sp -= 1,
        .A => self.af.a -= 1,
        .F => self.af.f -= 1,
        .B => self.bc.a -= 1,
        .C => self.bc.b -= 1,
        .D => self.de.a -= 1,
        .E => self.de.b -= 1,
        .H => self.hl.a -= 1,
        .L => self.hl.b -= 1,
    }
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
