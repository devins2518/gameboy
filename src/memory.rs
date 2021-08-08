pub fn map_memory<'a>(addr: u16) -> &'a [u8] {
    match addr {
        0x0000..=0x3FFF => println!("Attempt to read from ROM, bank 00"),
        0x4000..=0x7FFF => println!("Attempt to read from ROM, bank 01"),
        0x8000..=0x9FFF => println!("Attempt to read from VRAM"),
        0xA000..=0xBFFF => println!("Attempt to read from RAM"),
        0xC000..=0xCFFF => println!("Attempt to read from WRAM"),
        0xD000..=0xDFFF => println!("Attempt to read from WRAM"),
        0xE000..=0xFDFF => println!("Attempt to read from Echo RAM"),
        0xFE00..=0xFE9F => println!("Attempt to read from Sprite Attribute Table"),
        0xFEA0..=0xFEFF => println!("Attempt to read from prohibited area"),
        0xFF00..=0xFF7F => println!("Attempt to read from I/O registers"),
        0xFF80..=0xFFFE => println!("Attempt to read from high RAM"),
        0xFFFF..=0xFFFF => println!("Attempt to read from Interrupt Enable register"),
    }

    unimplemented!()
}
