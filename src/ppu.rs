use sdl2::video::Window;

pub struct Ppu {
    window: Window,
}

impl Ppu {
    pub fn new(window: Window) -> Self {
        Self { window }
    }
}
