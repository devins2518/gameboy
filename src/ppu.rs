use sdl2::render::Canvas;
use sdl2::video::Window;

pub struct Ppu {
    canvas: Canvas<Window>,
    mode: Mode,

    pub clocks: u32,
}

impl Ppu {
    pub fn new(canvas: Canvas<Window>) -> Self {
        Self {
            canvas,
            mode: Mode::Mode0,
            clocks: 0,
        }
    }

    pub fn clock(&mut self) -> i64 {
        let old_clocks = self.clocks;

        match self.mode {
            Mode::Mode0 => self.hblank(),
            Mode::Mode1 => self.vblank(),
            Mode::Mode2 => self.oam_scan(),
            Mode::Mode3 => self.draw(),
        }

        self.clocks += 1;

        old_clocks as i64 - self.clocks as i64
    }

    fn oam_scan(&mut self) {
        ()
        //unimplemented!()
    }
    fn draw(&mut self) {
        ()
        //unimplemented!()
    }
    fn hblank(&mut self) {
        ()
        //unimplemented!()
    }
    fn vblank(&mut self) {
        ()
        //unimplemented!()
    }
}

enum Mode {
    Mode0, // HBlank
    Mode1, // VBlank
    Mode2, // OAM Scan
    Mode3, // Drawing
}
