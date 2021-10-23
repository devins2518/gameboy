const std = @import("std");
const debug = std.log.debug;
const SDL = @import("sdl2");
const sdl_native = SDL.c;

pub fn debugAddr(comptime header: []const u8, args: anytype) void {
    const format = " 0x{X:0>2}";
    debug(header ++ format, args);
}

pub const Registers = enum {
    AF,
    BC,
    DE,
    HL,
    A,
    F,
    B,
    C,
    D,
    E,
    H,
    L,
    SP,
    PC,
    PHL,
    PHLP,
    PHLM,
    PBC,
    PDE,

    pub fn format(value: Registers, comptime _: []const u8, _: std.fmt.FormatOptions, writer: anytype) !void {
        try writer.writeAll(@tagName(value));
    }
};

pub const Optional = enum {
    C,
    NC,
    Z,
    NZ,
    pub fn format(value: Optional, comptime _: []const u8, _: std.fmt.FormatOptions, writer: anytype) !void {
        try writer.writeAll(@tagName(value));
    }
};

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
    sp,
    pc,
    pbc,
    pde,
    phl,
    phlm,
    phlp,
    p: u16,
    valU8: u8,
    valU16: u16,
    // Pass in base address and offset
    addrOffset: struct { base: u16, offset: u8 },
    spOffset: i8,

    pub fn format(value: Argument, comptime _: []const u8, _: std.fmt.FormatOptions, writer: anytype) !void {
        const s = switch (value) {
            .a => "A",
            .f => "F",
            .b => "B",
            .c => "C",
            .d => "D",
            .e => "E",
            .h => "H",
            .l => "L",
            .af => "AF",
            .bc => "BC",
            .de => "DE",
            .hl => "HL",
            .sp => "SP",
            .pc => "PC",
            .pbc => "(BC)",
            .pde => "(DE)",
            .phl => "(HL)",
            .phlm => "(HL-)",
            .phlp => "(HL+)",
            else => blk: {
                var buf: []u8 = std.mem.zeroes([]u8);
                switch (value) {
                    .p => |v| _ = std.fmt.formatIntBuf(buf, v, 16, .upper, std.fmt.FormatOptions{}),
                    else => @panic("uh oh"),
                }
                break :blk buf;
            },
        };
        try writer.writeAll(s);
    }
};

pub fn getPath() []const u8 {
    comptime {
        const dn = std.fs.path.dirname;
        const root = @src().file;

        return dn(dn(root).?).? ++ "/roms/blargg/cpu_instrs/cpu_instrs.gb";
    }
}

pub fn sdlPanic() noreturn {
    const str = @as(?[*:0]const u8, sdl_native.SDL_GetError()) orelse "unknown error";
    @panic(std.mem.sliceTo(str, 0));
}

pub const LCDC = 0xFF40;
pub const LCDS = 0xFF41;
pub const SCY = 0xFF42;
pub const SCX = 0xFF43;
pub const LY = 0xFF44;
pub const LYC = 0xFF45;
pub const bgPaletteData = 0xFF47;
pub const obp0 = 0xFF48;
pub const obp1 = 0xFF49;
pub const WY = 0xFF4A;
pub const WX = 0xFF4B;

pub const lcdc_struct = packed struct {
    enable: bool,
    window_tilemap: u1,
    window_enable: bool,
    bg_window_address_mode: u1,
    bg_tilemap: u1,
    obj_size: u1,
    obj_enable: bool,
    window_priority: u1,
};
pub const lcd_status = packed struct {
    _: u1,
    // TODO
    lyc_eq_ly_stat: bool,
    mode2_int: bool,
    mode1_int: bool,
    mode0_int: bool,
    lyc_eq_ly_flag: bool,
    mode: mode,
};

pub const mode = enum(u2) {
    hblank,
    vblank,
    oam_scan,
    draw,
};
