mod cpu;
mod memory;
mod utils;

use cpu::Cpu;

fn main() {
    let mut cpu = Cpu::new();
    loop {
        cpu.clock();
    }
}
