use std::ops::{Deref, DerefMut};

pub struct Bus {
    rom: Rom,
    vram: VRam,
    ram: Ram,
    spr_attr_table: SprAttrTable,
    io_reg: IOReg,
    hram: HRam,
    ie_reg: u8,
}

impl Bus {
    pub fn new() -> Self {
        Self {
            rom: Rom::default(),
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
            0x0000..=0x3FFF => {
                #[cfg(debug_assertions)]
                println!("Attempt to read from ROM, bank 00: {:#X}", addr);

                // ROM_START = 0x0000 so it will panic at runtime
                self.rom[addr as usize]
            }
            0x4000..=0x7FFF => {
                #[cfg(debug_assertions)]
                println!("Attempt to read from ROM, bank 01: {:#X}", addr);

                unimplemented!()
            }
            0x8000..=0x9FFF => {
                #[cfg(debug_assertions)]
                println!("Attempt to read from VRAM: {:#X}", addr);

                self.vram[addr as usize % VRAM_START]
            }
            0xA000..=0xBFFF => {
                #[cfg(debug_assertions)]
                println!("Attempt to read from RAM: {:#X}", addr);

                unimplemented!()
            }
            0xC000..=0xDFFF => {
                #[cfg(debug_assertions)]
                println!("Attempt to read from WRAM: {:#X}", addr);

                self.ram[addr as usize % RAM_START]
            }
            0xE000..=0xFDFF => {
                #[cfg(debug_assertions)]
                println!("Attempt to read from Echo RAM: {:#X}", addr);

                // I think
                // 0xE000 - 0xC000 = 0x2000
                self.ram[(addr as usize - 0x2000) % RAM_START]
            }
            0xFE00..=0xFE9F => {
                #[cfg(debug_assertions)]
                println!("Attempt to read from Sprite Attribute Table: {:#X}", addr);

                self.spr_attr_table[addr as usize % SPRATTRTABLE_START]
            }
            0xFEA0..=0xFEFF => {
                unreachable!("Attempted to read from prohibited area: {:#X}", addr)
            }
            0xFF00..=0xFF7F => {
                #[cfg(debug_assertions)]
                println!("Attempt to read from I/O registers: {:#X}", addr);

                self.io_reg[addr as usize % IOREG_START]
            }
            0xFF80..=0xFFFE => {
                #[cfg(debug_assertions)]
                println!("Attempt to read from high RAM: {:#X}", addr);

                self.hram[addr as usize % HRAM_START]
            }
            0xFFFF => {
                #[cfg(debug_assertions)]
                println!(
                    "Attempt to read from Interrupt Enable register: {:#X}",
                    addr
                );

                self.ie_reg
            }
        }
    }

    pub fn write_byte(&mut self, addr: u16, byte: u8) {
        match addr {
            0x0000..=0x3FFF => {
                #[cfg(debug_assertions)]
                println!("Attempt to write to ROM, bank 00: {:#X}", addr);

                // ROM_START = 0x0000 so it will panic at runtime
                self.rom[addr as usize] = byte;
            }
            0x4000..=0x7FFF => {
                #[cfg(debug_assertions)]
                println!("Attempt to write to ROM, bank 01: {:#X}", addr);

                unimplemented!()
            }
            0x8000..=0x9FFF => {
                #[cfg(debug_assertions)]
                println!("Attempt to write to VRAM: {:#X}", addr);

                self.vram[addr as usize % VRAM_START] = byte;
            }
            0xA000..=0xBFFF => {
                #[cfg(debug_assertions)]
                println!("Attempt to write to RAM: {:#X}", addr);

                unimplemented!()
            }
            0xC000..=0xDFFF => {
                #[cfg(debug_assertions)]
                println!("Attempt to write to WRAM: {:#X}", addr);

                self.ram[addr as usize % RAM_START] = byte;
            }
            0xE000..=0xFDFF => {
                #[cfg(debug_assertions)]
                println!("Attempt to write to Echo RAM: {:#X}", addr);

                unimplemented!()
            }
            0xFE00..=0xFE9F => {
                #[cfg(debug_assertions)]
                println!("Attempt to write to Sprite Attribute Table: {:#X}", addr);

                self.spr_attr_table[addr as usize % SPRATTRTABLE_START] = byte;
            }
            0xFEA0..=0xFEFF => {
                unreachable!("Attempted to write to prohibited are: {:#X}", addr)
            }
            0xFF00..=0xFF7F => {
                #[cfg(debug_assertions)]
                println!("Attempt to write to I/O registers: {:#X}", addr);

                self.io_reg[addr as usize % IOREG_START] = byte;
            }
            0xFF80..=0xFFFE => {
                #[cfg(debug_assertions)]
                println!("Attempt to write to high RAM: {:#X}", addr);

                self.hram[addr as usize % HRAM_START] = byte;
            }
            0xFFFF..=0xFFFF => {
                #[cfg(debug_assertions)]
                println!("Attempt to write to Interrupt Enable register: {:#X}", addr);

                self.ie_reg = byte;
            }
        }
    }
}

pub const ROM_SIZE: usize = 0x8000;
pub const ROM_START: usize = 0x0000;
pub struct Rom([u8; ROM_SIZE]);

impl Default for Rom {
    fn default() -> Self {
        let mut mem = [0; ROM_SIZE];
        // https://gbdev.gg8.se/wiki/articles/Gameboy_Bootstrap_ROM#Contents_of_the_ROM
        mem[0x0..=0xFF].copy_from_slice(&[
            0x31, 0xfe, 0xff, 0x3e, 0x30, 0xe0, 0x00, 0xaf, 0x21, 0xff, 0x9f, 0x32, 0xcb, 0x7c,
            0x20, 0xfb, 0x21, 0x26, 0xff, 0x0e, 0x11, 0x3e, 0x80, 0x32, 0xe2, 0x0c, 0x3e, 0xf3,
            0xe2, 0x32, 0x3e, 0x77, 0x77, 0x3e, 0xfc, 0xe0, 0x47, 0x21, 0x5f, 0xc0, 0x0e, 0x08,
            0xaf, 0x32, 0x0d, 0x20, 0xfc, 0x11, 0x4f, 0x01, 0x3e, 0xfb, 0x0e, 0x06, 0xf5, 0x06,
            0x00, 0x1a, 0x1b, 0x32, 0x80, 0x47, 0x0d, 0x20, 0xf8, 0x32, 0xf1, 0x32, 0x0e, 0x0e,
            0xd6, 0x02, 0xfe, 0xef, 0x20, 0xea, 0x11, 0x04, 0x01, 0x21, 0x10, 0x80, 0x1a, 0xcd,
            0xd3, 0x00, 0xcd, 0xd4, 0x00, 0x13, 0x7b, 0xfe, 0x34, 0x20, 0xf3, 0x11, 0xe6, 0x00,
            0x06, 0x08, 0x1a, 0x13, 0x22, 0x23, 0x05, 0x20, 0xf9, 0x3e, 0x19, 0xea, 0x10, 0x99,
            0x21, 0x2f, 0x99, 0x0e, 0x0c, 0x3d, 0x28, 0x08, 0x32, 0x0d, 0x20, 0xf9, 0x2e, 0x0f,
            0x18, 0xf3, 0x3e, 0x91, 0xe0, 0x40, 0x21, 0x00, 0xc0, 0x0e, 0x00, 0x3e, 0x00, 0xe2,
            0x3e, 0x30, 0xe2, 0x06, 0x10, 0x1e, 0x08, 0x2a, 0x57, 0xcb, 0x42, 0x3e, 0x10, 0x20,
            0x02, 0x3e, 0x20, 0xe2, 0x3e, 0x30, 0xe2, 0xcb, 0x1a, 0x1d, 0x20, 0xef, 0x05, 0x20,
            0xe8, 0x3e, 0x20, 0xe2, 0x3e, 0x30, 0xe2, 0xcd, 0xc2, 0x00, 0x7d, 0xfe, 0x60, 0x20,
            0xd2, 0x0e, 0x13, 0x3e, 0xc1, 0xe2, 0x0c, 0x3e, 0x07, 0xe2, 0x18, 0x3a, 0x16, 0x04,
            0xf0, 0x44, 0xfe, 0x90, 0x20, 0xfa, 0x1e, 0x00, 0x1d, 0x20, 0xfd, 0x15, 0x20, 0xf2,
            0xc9, 0x4f, 0x06, 0x04, 0xc5, 0xcb, 0x11, 0x17, 0xc1, 0xcb, 0x11, 0x17, 0x05, 0x20,
            0xf5, 0x22, 0x23, 0x22, 0x23, 0xc9, 0x3c, 0x42, 0xb9, 0xa5, 0xb9, 0xa5, 0x42, 0x3c,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x3e, 0x01, 0xe0, 0x50,
        ]);

        Self(mem)
    }
}

impl Deref for Rom {
    type Target = [u8; ROM_SIZE];

    fn deref(&self) -> &Self::Target {
        &self.0
    }
}

impl DerefMut for Rom {
    fn deref_mut(&mut self) -> &mut Self::Target {
        &mut self.0
    }
}

pub const VRAM_SIZE: usize = 0x8000;
pub const VRAM_START: usize = 0x8000;
pub struct VRam([u8; VRAM_SIZE]);

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
pub struct Ram([u8; RAM_SIZE]);

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
pub struct SprAttrTable([u8; SPRATTRTABLE_SIZE]);

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
pub struct IOReg([u8; IOREG_SIZE]);

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
pub struct HRam([u8; HRAM_SIZE]);

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
