const Bus = @import("Bus.zig");
const utils = @import("utils.zig");
const SDL = @import("sdl2");
const Self = @This();

lcdc: *utils.lcdc_struct,
lcds: *utils.lcd_status,
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

mode: utils.mode,

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
        .mode = .oam_scan,
        .lcdc = bus.getLcdc(),
        .lcds = bus.getLcds(),
        .scy = bus.getScy(),
        .scx = bus.getScx(),
        .ly = bus.getLy(),
        .lyc = bus.getLyc(),
        .bgPaletteData = bus.getBgPaletteData(),
        .obp0 = bus.getObp0(),
        .obp1 = bus.getObp1(),
        .wy = bus.getWy(),
        .wx = bus.getWx(),
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
