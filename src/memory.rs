use crate::cartridge::Cartridge;
use std::{
    ops::{Deref, DerefMut},
    path::Path,
};

#[derive(Clone)]
pub struct Bus {
    bootrom: BootRom,
    cartridge: Cartridge,
    vram: VRam,
    ram: Ram,
    spr_attr_table: SprAttrTable,
    io_reg: IOReg,
    hram: HRam,
    ie_reg: u8,
}

impl Bus {
    pub fn new<P: AsRef<Path>>(path: P) -> Self {
        Self {
            bootrom: BootRom::default(),
            cartridge: Cartridge::new(path),
            vram: VRam::default(),
            ram: Ram::default(),
            spr_attr_table: SprAttrTable::default(),
            io_reg: IOReg::default(),
            hram: HRam::default(),
            ie_reg: 0x0,
        }
    }

    pub fn get_address(&self, addr: u16) -> u8 {
        match addr {
            0x0000..=0x00FF => {
                #[cfg(debug_assertions)]
                println!(
                    "Attempt to read from ROM, bank 00: {:#06X}, value: {:#04X}",
                    addr, self.bootrom[addr as usize]
                );

                // ROM_START = 0x0000 so it will panic at runtime
                self.bootrom[addr as usize]
            }
            0x0100..=0x3FFF => {
                #[cfg(debug_assertions)]
                println!("Attempt to read from ROM, bank 00: {:#06X}", addr);

                // ROM_START = 0x0000 so it will panic at runtime
                // self.rom[addr as usize]
                unimplemented!()
            }
            0x4000..=0x7FFF => {
                #[cfg(debug_assertions)]
                println!("Attempt to read from ROM, bank 01: {:#06X}", addr);

                // self.rom[addr as usize]
                unimplemented!()
            }
            0x8000..=0x9FFF => {
                #[cfg(debug_assertions)]
                println!(
                    "Attempt to read from VRAM: {:#06X}, value: {:#04X}",
                    addr,
                    self.vram[addr as usize % VRAM_START]
                );

                self.vram[addr as usize % VRAM_START]
            }
            0xA000..=0xBFFF => {
                #[cfg(debug_assertions)]
                println!("Attempt to read from cartridge RAM: {:#06X}", addr);

                unimplemented!()
            }
            0xC000..=0xDFFF => {
                #[cfg(debug_assertions)]
                println!(
                    "Attempt to read from WRAM: {:#06X}, value: {:#04X}",
                    addr,
                    self.ram[addr as usize % RAM_START]
                );

                self.ram[addr as usize % RAM_START]
            }
            0xE000..=0xFDFF => {
                #[cfg(debug_assertions)]
                println!(
                    "Attempt to read from Echo RAM: {:#06X}, value: {:#04X}",
                    addr,
                    self.ram[(addr as usize - 0x2000) % RAM_START]
                );

                // I think
                // 0xE000 - 0xC000 = 0x2000
                self.ram[(addr as usize - 0x2000) % RAM_START]
            }
            0xFE00..=0xFE9F => {
                #[cfg(debug_assertions)]
                println!(
                    "Attempt to read from Sprite Attribute Table: {:#06X}, value: {:#04X}",
                    addr,
                    self.spr_attr_table[addr as usize % SPRATTRTABLE_START]
                );

                self.spr_attr_table[addr as usize % SPRATTRTABLE_START]
            }
            0xFEA0..=0xFEFF => {
                unreachable!("Attempted to read from prohibited area: {:#06X}", addr)
            }
            0xFF00..=0xFF7F => {
                #[cfg(debug_assertions)]
                println!(
                    "Attempt to read from I/O registers: {:#06X}, value: {:#04X}",
                    addr,
                    self.io_reg[addr as usize % IOREG_START]
                );

                self.io_reg[addr as usize % IOREG_START]
            }
            0xFF80..=0xFFFE => {
                #[cfg(debug_assertions)]
                println!(
                    "Attempt to read from high RAM: {:#06X}, value: {:#04X}",
                    addr,
                    self.hram[addr as usize % HRAM_START]
                );

                self.hram[addr as usize % HRAM_START]
            }
            0xFFFF => {
                #[cfg(debug_assertions)]
                println!(
                    "Attempt to read from Interrupt Enable register: {:#06X}, value: {:#04X}",
                    addr, self.ie_reg
                );

                self.ie_reg
            }
        }
    }

    pub fn write_byte(&mut self, addr: u16, byte: u8) {
        match addr {
            0x0000..=0x00FF => {
                #[cfg(debug_assertions)]
                println!("Attempt to write to ROM, bank 00. {:#06X}", addr);

                // ROM_START = 0x0000 so it will panic at runtime
                self.bootrom[addr as usize] = byte;
            }
            0x0100..=0x3FFF => {
                #[cfg(debug_assertions)]
                println!("Attempt to write to ROM, bank 00. Addr: {:#06X}", addr);

                // ROM_START = 0x0000 so it will panic at runtime
                // self.rom[addr as usize] = byte;
                unimplemented!()
            }
            0x4000..=0x7FFF => {
                #[cfg(debug_assertions)]
                println!("Attempt to write to ROM, bank 01. Addr: {:#06X}", addr);

                // self.rom[addr as usize] = byte;
                unimplemented!()
            }
            0x8000..=0x9FFF => {
                #[cfg(debug_assertions)]
                println!("Attempt to write to VRAM. Addr: {:#06X}", addr);

                self.vram[addr as usize % VRAM_START] = byte;
            }
            0xA000..=0xBFFF => {
                #[cfg(debug_assertions)]
                println!("Attempt to write to RAM. Addr: {:#06X}", addr);

                unimplemented!()
            }
            0xC000..=0xDFFF => {
                #[cfg(debug_assertions)]
                println!("Attempt to write to WRAM. Addr: {:#06X}", addr);

                self.ram[addr as usize % RAM_START] = byte;
            }
            0xE000..=0xFDFF => {
                #[cfg(debug_assertions)]
                println!("Attempt to write to Echo RAM. Addr: {:#06X}", addr);

                unimplemented!()
            }
            0xFE00..=0xFE9F => {
                #[cfg(debug_assertions)]
                println!(
                    "Attempt to write to Sprite Attribute Table. Addr: {:#06X}",
                    addr
                );

                self.spr_attr_table[addr as usize % SPRATTRTABLE_START] = byte;
            }
            0xFEA0..=0xFEFF => {
                unreachable!("Attempted to write to prohibited are. Addr: {:#06X}", addr)
            }
            0xFF00..=0xFF7F => {
                #[cfg(debug_assertions)]
                println!("Attempt to write to I/O registers. Addr: {:#06X}", addr);

                self.io_reg[addr as usize % IOREG_START] = byte;
            }
            0xFF80..=0xFFFE => {
                #[cfg(debug_assertions)]
                println!("Attempt to write to high RAM. Addr: {:#06X}", addr);

                self.hram[addr as usize % HRAM_START] = byte;
            }
            0xFFFF..=0xFFFF => {
                #[cfg(debug_assertions)]
                println!(
                    "Attempt to write to Interrupt Enable register. Addr: {:#06X}",
                    addr
                );

                self.ie_reg = byte;
            }
        }
    }
}

pub const BOOTROM_SIZE: usize = 0x0100;
pub const BOOTROM_START: usize = 0x0000;
#[derive(Clone)]
struct BootRom([u8; BOOTROM_SIZE]);

impl Default for BootRom {
    fn default() -> Self {
        // https://gbdev.gg8.se/wiki/articles/Gameboy_Bootstrap_ROM#Contents_of_the_ROM
        let mem: [u8; BOOTROM_SIZE] = [
            0x31, 0xfe, 0xff, 0xaf, 0x21, 0xff, 0x9f, 0x32, 0xcb, 0x7c, 0x20, 0xfb, 0x21, 0x26,
            0xff, 0x0e, 0x11, 0x3e, 0x80, 0x32, 0xe2, 0x0c, 0x3e, 0xf3, 0xe2, 0x32, 0x3e, 0x77,
            0x77, 0x3e, 0xfc, 0xe0, 0x47, 0x11, 0x04, 0x01, 0x21, 0x10, 0x80, 0x1a, 0xcd, 0x95,
            0x00, 0xcd, 0x96, 0x00, 0x13, 0x7b, 0xfe, 0x34, 0x20, 0xf3, 0x11, 0xd8, 0x00, 0x06,
            0x08, 0x1a, 0x13, 0x22, 0x23, 0x05, 0x20, 0xf9, 0x3e, 0x19, 0xea, 0x10, 0x99, 0x21,
            0x2f, 0x99, 0x0e, 0x0c, 0x3d, 0x28, 0x08, 0x32, 0x0d, 0x20, 0xf9, 0x2e, 0x0f, 0x18,
            0xf3, 0x67, 0x3e, 0x64, 0x57, 0xe0, 0x42, 0x3e, 0x91, 0xe0, 0x40, 0x04, 0x1e, 0x02,
            0x0e, 0x0c, 0xf0, 0x44, 0xfe, 0x90, 0x20, 0xfa, 0x0d, 0x20, 0xf7, 0x1d, 0x20, 0xf2,
            0x0e, 0x13, 0x24, 0x7c, 0x1e, 0x83, 0xfe, 0x62, 0x28, 0x06, 0x1e, 0xc1, 0xfe, 0x64,
            0x20, 0x06, 0x7b, 0xe2, 0x0c, 0x3e, 0x87, 0xe2, 0xf0, 0x42, 0x90, 0xe0, 0x42, 0x15,
            0x20, 0xd2, 0x05, 0x20, 0x4f, 0x16, 0x20, 0x18, 0xcb, 0x4f, 0x06, 0x04, 0xc5, 0xcb,
            0x11, 0x17, 0xc1, 0xcb, 0x11, 0x17, 0x05, 0x20, 0xf5, 0x22, 0x23, 0x22, 0x23, 0xc9,
            0xce, 0xed, 0x66, 0x66, 0xcc, 0x0d, 0x00, 0x0b, 0x03, 0x73, 0x00, 0x83, 0x00, 0x0c,
            0x00, 0x0d, 0x00, 0x08, 0x11, 0x1f, 0x88, 0x89, 0x00, 0x0e, 0xdc, 0xcc, 0x6e, 0xe6,
            0xdd, 0xdd, 0xd9, 0x99, 0xbb, 0xbb, 0x67, 0x63, 0x6e, 0x0e, 0xec, 0xcc, 0xdd, 0xdc,
            0x99, 0x9f, 0xbb, 0xb9, 0x33, 0x3e, 0x3c, 0x42, 0xb9, 0xa5, 0xb9, 0xa5, 0x42, 0x3c,
            0x21, 0x04, 0x01, 0x11, 0xa8, 0x00, 0x1a, 0x13, 0xbe, 0x20, 0xfe, 0x23, 0x7d, 0xfe,
            0x34, 0x20, 0xf5, 0x06, 0x19, 0x78, 0x86, 0x23, 0x05, 0x20, 0xfb, 0x86, 0x20, 0xfe,
            0x3e, 0x01, 0xe0, 0x50,
        ];

        Self(mem)
    }
}

impl Deref for BootRom {
    type Target = [u8; BOOTROM_SIZE];

    fn deref(&self) -> &Self::Target {
        &self.0
    }
}

impl DerefMut for BootRom {
    fn deref_mut(&mut self) -> &mut Self::Target {
        &mut self.0
    }
}

pub const VRAM_SIZE: usize = 0x8000;
pub const VRAM_START: usize = 0x8000;
#[derive(Clone)]
struct VRam([u8; VRAM_SIZE]);

impl Default for VRam {
    fn default() -> Self {
        Self([0; VRAM_SIZE])
    }
}

impl Deref for VRam {
    type Target = [u8; VRAM_SIZE];

    fn deref(&self) -> &Self::Target {
        &self.0
    }
}

impl DerefMut for VRam {
    fn deref_mut(&mut self) -> &mut Self::Target {
        &mut self.0
    }
}

pub const RAM_SIZE: usize = 0x8000;
pub const RAM_START: usize = 0xC000;
#[derive(Clone)]
struct Ram([u8; RAM_SIZE]);

impl Default for Ram {
    fn default() -> Self {
        Self([0; RAM_SIZE])
    }
}

impl Deref for Ram {
    type Target = [u8; RAM_SIZE];

    fn deref(&self) -> &Self::Target {
        &self.0
    }
}

impl DerefMut for Ram {
    fn deref_mut(&mut self) -> &mut Self::Target {
        &mut self.0
    }
}

pub const SPRATTRTABLE_SIZE: usize = 0xA0;
pub const SPRATTRTABLE_START: usize = 0xFE00;
#[derive(Clone)]
struct SprAttrTable([u8; SPRATTRTABLE_SIZE]);

impl Default for SprAttrTable {
    fn default() -> Self {
        Self([0; SPRATTRTABLE_SIZE])
    }
}

impl Deref for SprAttrTable {
    type Target = [u8; SPRATTRTABLE_SIZE];

    fn deref(&self) -> &Self::Target {
        &self.0
    }
}

impl DerefMut for SprAttrTable {
    fn deref_mut(&mut self) -> &mut Self::Target {
        &mut self.0
    }
}

pub const IOREG_SIZE: usize = 0x80;
pub const IOREG_START: usize = 0xFF00;
#[derive(Clone)]
struct IOReg([u8; IOREG_SIZE]);

impl Default for IOReg {
    fn default() -> Self {
        Self([0; IOREG_SIZE])
    }
}

impl Deref for IOReg {
    type Target = [u8; IOREG_SIZE];

    fn deref(&self) -> &Self::Target {
        &self.0
    }
}

impl DerefMut for IOReg {
    fn deref_mut(&mut self) -> &mut Self::Target {
        &mut self.0
    }
}

pub const HRAM_SIZE: usize = 0x80;
pub const HRAM_START: usize = 0xFF80;
#[derive(Clone)]
struct HRam([u8; HRAM_SIZE]);

impl Default for HRam {
    fn default() -> Self {
        Self([0; HRAM_SIZE])
    }
}

impl Deref for HRam {
    type Target = [u8; HRAM_SIZE];

    fn deref(&self) -> &Self::Target {
        &self.0
    }
}

impl DerefMut for HRam {
    fn deref_mut(&mut self) -> &mut Self::Target {
        &mut self.0
    }
}
