use crate::cpu::Cpu;
use crate::memory::Bus;
use crate::ppu::Ppu;
use crate::utils::Event as GBEvent;
use log::{debug, trace};
use sdl2::event::Event;
use sdl2::keyboard::Keycode;
use sdl2::pixels::Color;
use sdl2::Sdl;
use std::sync::mpsc;
use std::time::{Duration, Instant};

pub struct GameBoy {
    cpu: Cpu,
    ppu: Ppu,
    auto: bool,
    sdl_context: Sdl,

    schedule_clocks: i64,
}

impl GameBoy {
    pub fn new(path: &str, context: Sdl) -> Self {
        let (rx, tx) = mpsc::channel::<GBEvent>();

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
            sdl_context: context,
            schedule_clocks: 0,
        }
    }

    pub fn run(&mut self) {
        let mut event_pump = self.sdl_context.event_pump().unwrap();
        let frame_interval = Duration::new(0, 1000000000u32 / 60);
        let clock_interval = Duration::from_micros(1);
        'main_loop: loop {
            let now = Instant::now();
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
                        self.auto = !self.auto;
                    }
                    Event::KeyDown {
                        keycode: Some(Keycode::G),
                        ..
                    } => self.step(),
                    _ => (),
                }
            }

            let frame_delta = now.elapsed();
            if self.auto && frame_delta > clock_interval {
                self.step();
                println!("{:?}", frame_delta);
                ::std::thread::sleep(frame_delta - clock_interval)
            }

            if frame_delta < frame_interval {
                // ::std::thread::sleep(frame_interval - frame_delta)
            };
        }
    }

    fn step(&mut self) {
        debug!(
            "\nCPU Clocks: {}\nPPU Clocks: {}\nScheduler Clocks: {}",
            self.cpu.clocks, self.ppu.clocks, self.schedule_clocks
        );
        self.schedule_clocks += if self.schedule_clocks <= 0 {
            trace!("Clocking CPU");
            self.cpu.clock()
        } else {
            trace!("Clocking PPU");
            self.ppu.clock()
        };
    }
}
