const Bus = @import("Bus.zig");
const utils = @import("utils.zig");
const SDL = @import("sdl2");
const Self = @This();

const LCDC = 0xFF40;
const LCDS = 0xFF41;
const SCY = 0xFF42;
const SCX = 0xFF43;
const LY = 0xFF44;
const LYC = 0xFF45;
const bgPaletteData = 0xFF47;
const obp0 = 0xFF48;
const obp1 = 0xFF49;
const WY = 0xFF4A;
const WX = 0xFF4B;

const lcdc_struct = packed struct {
    enable: bool,
    window_tilemap: u1,
    window_enable: bool,
    bg_window_address_mode: u1,
    bg_tilemap: u1,
    obj_size: u1,
    obj_enable: bool,
    window_priority: u1,
};
const lcd_status = packed struct {
    _: u1,
    // TODO
    lyc_eq_ly_stat: bool,
    mode2_int: bool,
    mode1_int: bool,
    mode0_int: bool,
    lyc_eq_ly_flag: bool,
    mode: mode,
};
const mode = enum(u2) {
    hblank,
    vblank,
    oam_scan,
    draw,
};

lcdc: *lcdc_struct,
lcds: *lcd_status,
scy: *u8,
scx: *u8,
ly: *u8,
lyc: *u8,
bgPaletteData: *u8,
obp0: *u8,
obp1: *u8,
wy: *u8,
wx: *u8,

renderer: SDL.Renderer,
window: SDL.Window,
texture: SDL.Texture,

mode: mode,

const screen_h = 160;
const screen_w = 144;

pub fn init(bus: *Bus) !Self {
    try SDL.init(.{
        .video = true,
        .events = true,
        .audio = true,
    });

    var w = try SDL.createWindow(
        "Ziggyboy",
        .{ .centered = {} },
        .{ .centered = {} },
        screen_w,
        screen_h,
        .{ .shown = true },
    );
    var r = try SDL.createRenderer(w, null, .{ .accelerated = true });
    // Needed to create window in wayland
    r.present();
    var t = createTextureBuffer(&r);

    return Self{
        .renderer = r,
        .window = w,
        .texture = t,
        .mode = mode.oam_scan,
        .lcdc = @ptrCast(*lcdc_struct, bus.getAddressPtr(LCDC)),
        .lcds = @ptrCast(*lcd_status, bus.getAddressPtr(LCDS)),
        .scy = bus.getAddressPtr(SCY),
        .scx = bus.getAddressPtr(SCX),
        .ly = bus.getAddressPtr(LY),
        .lyc = bus.getAddressPtr(LYC),
        .bgPaletteData = bus.getAddressPtr(bgPaletteData),
        .obp0 = bus.getAddressPtr(obp0),
        .obp1 = bus.getAddressPtr(obp1),
        .wy = bus.getAddressPtr(WY),
        .wx = bus.getAddressPtr(WX),
    };
}

pub fn deinit(self: Self) void {
    self.window.destroy();
    self.renderer.destroy();
}

pub fn clock(self: *Self) !void {
    try self.renderer.setColorRGB(0xF7, 0xA4, 0x1D);
    self.renderer.present();
}

fn createTextureBuffer(r: *SDL.Renderer) SDL.Texture {
    return SDL.createTexture(r.*, .argb8888, .streaming, screen_w, screen_h) catch utils.sdlPanic();
}
