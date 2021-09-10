mod cartridge;
mod cpu;
mod memory;
mod ppu;
mod utils;

use cpu::Cpu;
use env_logger::Env;
use memory::Bus;
use ppu::Ppu;
use sdl2::event::Event;
use sdl2::keyboard::Keycode;
use sdl2::pixels::Color;
use sdl2::Sdl;
use std::time::Duration;

struct GameBoy {
    cpu: Cpu,
    ppu: Ppu,
    auto: bool,
}

impl GameBoy {
    fn new(path: &str, context: &Sdl) -> Self {
        let bus = Bus::new(&path);
        let cpu = Cpu::new(bus);

        let video_subsystem = context.video().unwrap();
        let window = video_subsystem
            .window("rustyboy", 160, 144)
            .position_centered()
            .build()
            .unwrap();
        let mut canvas = window.into_canvas().build().unwrap();

        canvas.set_draw_color(Color::RGB(0, 255, 255));
        canvas.clear();
        canvas.present();

        let ppu = Ppu::new(canvas);

        Self {
            cpu,
            ppu,
            auto: false,
        }
    }

    fn clock(&mut self) {
        self.cpu.clock();
    }
}

fn main() {
    env_logger::Builder::from_env(Env::default().default_filter_or("info")).init();

    let path = std::env::args().nth(0).unwrap();
    let sdl_context = sdl2::init().unwrap();

    let mut gb = GameBoy::new(&path, &sdl_context);

    let mut event_pump = sdl_context.event_pump().unwrap();
    'main_loop: loop {
        for event in event_pump.poll_iter() {
            match event {
                Event::Quit { .. }
                | Event::KeyDown {
                    keycode: Some(Keycode::Escape),
                    ..
                } => break 'main_loop,
                Event::KeyDown {
                    keycode: Some(Keycode::A),
                    ..
                } => {
                    gb.auto = !gb.auto;
                    println!("gb.auto {}", gb.auto);
                }
                Event::KeyDown {
                    keycode: Some(Keycode::G),
                    ..
                } => gb.clock(),
                _ => (),
            }
        }
        if gb.auto {
            gb.clock()
        }

        ::std::thread::sleep(Duration::new(0, 1_000_000_000u32 / 60));
    }
}
