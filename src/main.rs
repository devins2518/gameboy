mod cpu;
mod memory;

use cpu::Cpu;

fn main() {
    let mut cpu = Cpu::new();
    loop {
        cpu.clock();
    }
}
