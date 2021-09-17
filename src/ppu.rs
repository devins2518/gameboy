use sdl2::render::Canvas;
use sdl2::video::Window;

pub struct Ppu {
    canvas: Canvas<Window>,

    pub clocks: u32,
}

impl Ppu {
    pub fn new(canvas: Canvas<Window>) -> Self {
        Self { canvas, clocks: 0 }
    }

    pub fn clock(&mut self) -> i64 {
        let old_clocks = self.clocks;

        self.clocks += 1;

        i64::from(self.clocks - old_clocks)
    }
}
