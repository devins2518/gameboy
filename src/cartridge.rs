use std::{fs, path::Path};

#[derive(Clone)]
pub struct Cartridge {
    data: Vec<u8>,
    // mapper: Mapper,
}

#[derive(Clone)]
enum Mapper {
    None,
    Mbc1,
    Mbc2,
    Mbc3,
    Mbc4,
    Mbc5,
    Mbc6,
    Mbc7,
    HuC1,
}

impl Cartridge {
    pub fn new<P: AsRef<Path>>(path: P) -> Self {
        let data = fs::read(&path).expect(&format!("Could not open {}", path.as_ref().display()));

        Self { data }
    }
}
