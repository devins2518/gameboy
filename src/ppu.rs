use sdl2::render::Canvas;
use sdl2::video::Window;

pub struct Ppu {
    canvas: Canvas<Window>,

    pub clocks: u64,
}

impl Ppu {
    pub fn new(canvas: Canvas<Window>) -> Self {
        Self { canvas, clocks: 0 }
    }

    pub fn clock(&mut self) {
        self.clocks += 1;
    }
}
