use std::{fs, path::Path};

#[derive(Clone)]
pub struct Cartridge {
    data: Vec<u8>,
}

impl Cartridge {
    pub fn new<P: AsRef<Path>>(path: P) -> Self {
        let data = fs::read(&path).expect(&format!("Could not open {}", path.as_ref().display()));

        Self { data }
    }
}
