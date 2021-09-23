mod cartridge;
mod cpu;
mod gameboy;
mod memory;
mod ppu;
mod utils;

use env_logger::Env;
use gameboy::GameBoy;

fn main() {
    env_logger::Builder::from_env(Env::default().default_filter_or("info")).init();

    let path = std::env::args().nth(0).unwrap_or_else(|| {
        String::from(concat!(
            env!("CARGO_MANIFEST_DIR"),
            "/roms/blargg/cpu_instrs/cpu_instrs.gb"
        ))
    });
    let sdl_context = sdl2::init().unwrap();

    let mut gb = GameBoy::new(&path, sdl_context);

    gb.run();
}
