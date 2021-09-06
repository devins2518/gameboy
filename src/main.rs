mod cartridge;
mod cpu;
mod memory;
mod utils;

use cpu::Cpu;
use memory::Bus;

struct GameBoy {
    cpu: Cpu,
}

fn main() {
    let path = std::env::args().nth(0).unwrap();
    let bus = Bus::new(&path);

    let cpu = Cpu::new(bus);

    let mut gb = GameBoy { cpu };

    loop {
        gb.cpu.clock();
    }
}
