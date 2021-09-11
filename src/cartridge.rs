use std::{fs, path::Path};

#[derive(Clone)]
pub struct Cartridge {
    data: Vec<u8>,
    mapper: Mapper,
}

impl Cartridge {
    pub fn new<P: AsRef<Path>>(path: P) -> Self {
        let data = fs::read(&path).expect(&format!("Could not open {}", path.as_ref().display()));

        use Mapper::*;
        let mapper = match data[0x47] {
            0x00 => RomOnly,
            _ => unreachable!(),
        };

        Self { data, mapper }
    }

    pub fn get_address(&self, addr: u16) -> u8 {
        match self.mapper {
            Mapper::RomOnly => self.data[addr as usize],
            _ => unimplemented!(),
        }
    }

    pub fn write_address(&mut self, addr: u16, n: u8) {
        match self.mapper {
            Mapper::RomOnly => self.data[addr as usize] = n,
            _ => unimplemented!(),
        }
    }
}

#[derive(Clone)]
enum Mapper {
    RomOnly,
}

type Bank = [u8; 0x4000];

struct Mbc1 {
    rom_banks: [Bank; 0x80],
    rom_bank: u8,
    ram_banks: [Bank; 0x80],
    ram_bank: u8,
    ram_enable: bool,
    rom_ram: RomRam,
}

enum RomRam {
    Rom,
    Ram,
}
