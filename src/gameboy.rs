use crate::cpu::Cpu;
use crate::memory::Bus;
use crate::ppu::Ppu;
use crate::utils::Event as GBEvent;
use log::{debug, trace};
use sdl2::event::Event;
use sdl2::keyboard::Keycode;
use sdl2::pixels::Color;
use sdl2::Sdl;
use std::future::Future;
use std::pin::Pin;
use std::sync::Arc;
use std::task::Wake;
use std::thread::Thread;
use std::time::{Duration, Instant};
use tokio::sync::watch::{self, Sender};
use tokio::task::JoinHandle;

pub struct GameBoy {
    tx: Sender<GBEvent>,
    auto: bool,
    sdl_context: Sdl,

    schedule_clocks: i64,
    bus: JoinHandle<()>,
    cpu: JoinHandle<()>,
    ppu: JoinHandle<()>,
}

impl GameBoy {
    pub fn new(path: &str, context: Sdl) -> Self {
        let (tx, rx) = watch::channel::<GBEvent>(GBEvent::Clock);

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

        let mut bus = Bus::new(&path, rx.clone());
        let mut cpu = Cpu::new(rx.clone());
        let mut ppu = Ppu::new(canvas, rx.clone());

        let bus = tokio::task::spawn(async move {
            bus.run();
        });
        let cpu = tokio::task::spawn(async move {
            cpu.run();
        });
        let ppu = tokio::task::spawn(async move {
            ppu.run();
        });

        Self {
            tx,
            auto: false,
            sdl_context: context,
            schedule_clocks: 0,

            bus,
            cpu,
            ppu,
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
            }

            // if frame_delta < frame_interval {
            //     ::std::thread::sleep(frame_interval - frame_delta)
            // };
        }
    }

    fn step(&mut self) {
        self.tx.send(GBEvent::Clock).unwrap();
    }
}

impl Wake for GameBoy {
    fn wake(self: Arc<Self>) {}
}
