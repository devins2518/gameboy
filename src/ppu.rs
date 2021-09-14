use sdl2::render::Canvas;
use sdl2::video::Window;

pub struct Ppu {
    canvas: Canvas<Window>,
}

impl Ppu {
    pub fn new(canvas: Canvas<Window>) -> Self {
        Self { canvas }
    }

    pub fn clock(&mut self) {}
}
