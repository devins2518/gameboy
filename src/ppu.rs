use crate::utils::Event;
use sdl2::render::Canvas;
use sdl2::video::Window;
use tokio::sync::watch::Receiver;

pub struct Ppu {
    mode: Mode,

    pub clocks: u32,
    rx: Receiver<Event>,
}

impl Ppu {
    pub fn new(_canvas: Canvas<Window>, rx: Receiver<Event>) -> Self {
        Self {
            mode: Mode::Mode0,
            clocks: 0,
            rx,
        }
    }

    pub async fn run(&mut self) {
        if let Ok(()) = self.rx.changed().await {
            self.clock()
        }
    }

    pub fn clock(&mut self) {
        let old_clocks = self.clocks;

        match self.mode {
            Mode::Mode0 => self.hblank(),
            Mode::Mode1 => self.vblank(),
            Mode::Mode2 => self.oam_scan(),
            Mode::Mode3 => self.draw(),
        }

        self.clocks += 1;

        // old_clocks as i64 - self.clocks as i64
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
