use std::{fs, path::Path};

#[derive(Clone)]
pub struct Cartridge {
    data: Vec<u8>,
    mapper: Mapper,
}

#[derive(Clone)]
enum Mapper {
    RomOnly,
    Mbc5,
    Mbc1,
    Mbc5Ram,
    Mbc1Ram,
    Mbc5RamBattery,
    Mbc1RamBattery,
    Mbc5Rumble,
    Mbc2,
    Mbc5RumbleRam,
    Mbc2Battery,
    Mbc5RumbleRamBattery,
    RomRam,
    Mbc6,
    RomRamBattery,
    Mbc7SensorRumbleRamBattery,
    Mmm01,
    Mmm01Ram,
    Mmm01RamBattery,
    Mbc3TimerBattery,
    Mbc3TimerRamBattery,
    PocketCamera,
    Mbc3,
    BandaiTama5,
    Mbc3Ram,
    Huc3,
    Mbc3RamBattery,
    Huc1RamBattery,
}

impl Cartridge {
    pub fn new<P: AsRef<Path>>(path: P) -> Self {
        let data = fs::read(&path).expect(&format!("Could not open {}", path.as_ref().display()));

        use Mapper::*;
        let mapper = match data[0x47] {
            0x00 => RomOnly,
            0x01 => Mbc1,
            0x02 => Mbc1Ram,
            0x03 => Mbc1RamBattery,
            0x05 => Mbc2,
            0x06 => Mbc2Battery,
            0x08 => RomRam,
            0x09 => RomRamBattery,
            0x0B => Mmm01,
            0x0C => Mmm01Ram,
            0x0D => Mmm01RamBattery,
            0x0F => Mbc3TimerBattery,
            0x10 => Mbc3TimerRamBattery,
            0x11 => Mbc3,
            0x12 => Mbc3Ram,
            0x13 => Mbc3RamBattery,
            0x19 => Mbc5,
            0x1A => Mbc5Ram,
            0x1B => Mbc5RamBattery,
            0x1C => Mbc5Rumble,
            0x1D => Mbc5RumbleRam,
            0x1E => Mbc5RumbleRamBattery,
            0x20 => Mbc6,
            0x22 => Mbc7SensorRumbleRamBattery,
            0xFC => PocketCamera,
            0xFD => BandaiTama5,
            0xFE => Huc3,
            0xFF => Huc1RamBattery,
            _ => unreachable!(),
        };

        Self { data, mapper }
    }
}
