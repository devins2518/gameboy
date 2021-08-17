use modular_bitfield::prelude::*;
use std::ops::{Deref, DerefMut};

use crate::memory::Memory;

pub struct Cpu {
    af: AFReg,
    bc: GPReg,
    de: GPReg,
    hl: GPReg,
    sp: u16,
    pc: u16,
    memory: Memory,
}

pub enum Register {
    A,
    F,
    B,
    C,
    D,
    E,
    H,
    L,
    AF,
    BC,
    DE,
    HL,
}

impl Cpu {
    pub fn new() -> Self {
        Cpu {
            af: 0.into(),
            bc: 0.into(),
            de: 0.into(),
            hl: 0.into(),
            sp: 0x0000,
            pc: 0x0000,
            memory: Memory::new(),
        }
    }
    fn get_regu8(&self, reg: Register) -> u8 {
        use Register::*;
        match reg {
            A => self.af.a(),
            F => self.af.f(),
            B => self.bc[0],
            C => self.bc[1],
            D => self.de[0],
            E => self.de[1],
            H => self.hl[0],
            L => self.hl[1],
            _ => unreachable!("Attempted to get u16 from get_regu8"),
        }
    }
    fn set_regu8(&mut self, reg: Register, val: u8) {
        use Register::*;
        let ref mut reg = match reg {
            A => self.af.into_bytes()[0],
            F => self.af.into_bytes()[1],
            B => self.bc[0],
            C => self.bc[1],
            D => self.de[0],
            E => self.de[1],
            H => self.hl[0],
            L => self.hl[1],
            _ => unreachable!("Attempted to get u16 from get_regu8"),
        };
        *reg = val;
    }
    fn get_regu16(&self, reg: Register) -> u16 {
        use Register::*;
        match reg {
            AF => self.af.into(),
            BC => self.bc.into(),
            DE => self.de.into(),
            HL => self.hl.into(),
            _ => unreachable!("Attempted to get u16 from get_regu8"),
        }
    }
    fn set_regu16(&self, reg: Register, val: u16) {
        use Register::*;
        let ref mut reg = match reg {
            AF => self.af.into(),
            BC => self.bc.into(),
            DE => self.de.into(),
            HL => self.hl.into(),
            _ => unreachable!("Attempted to get u16 from get_regu8"),
        };
        *reg = val;
    }

    fn ld_regu8(&mut self, reg: Register, n: u8) {
        self.set_regu8(reg, n);
    }

    fn ld_regu16(&mut self, reg: Register, n: u16) {
        self.set_regu16(reg, n);
    }

    fn push(&mut self, reg: Register) {
        use Register::*;

        let mut range = self.memory.get_address_mut(self.pc);

        let ref reg = match reg {
            AF => self.af.into_bytes(),
            BC => self.bc.0,
            DE => self.de.0,
            HL => self.hl.0,
            _ => unreachable!("Attempted to push single byte register onto stack"),
        };

        range[0] = reg[0];
        self.pc -= 1;
        range[1] = reg[1];
        self.pc -= 1;
    }

    fn pop(&mut self, reg: Register) {
        use Register::*;

        let range = self.memory.get_address(self.pc);

        let mut reg = match reg {
            AF => self.af.into_bytes(),
            BC => self.bc.0,
            DE => self.de.0,
            HL => self.hl.0,
            _ => unreachable!("Attempted to push single byte register onto stack"),
        };

        reg[0] = range[0];
        self.pc += 1;
        reg[1] = range[1];
        self.pc += 1;
    }

    fn add(&mut self, n: u8) {
        let (val, over) = self.af.into_bytes()[0].overflowing_add(n);
        self.af.set_a(val);
        if over {
            self.af.set_z(true);
        }
        unimplemented!("Rest of flags")
    }

    fn or(&mut self, n: u8) {
        let val = self.af.a();
        self.af.set_a(val | n);
        self.af.reset_flags();
        if self.af.a() == 0 {
            self.af.set_z(true);
        }
    }

    fn xor(&mut self, n: u8) {
        let val = self.af.a();
        self.af.set_a(val ^ n);
        self.af.reset_flags();
        if self.af.a() == 0 {
            self.af.set_z(true);
        }
    }

    fn cp(&mut self, n: u8) {
        let val = self.af.a() - n;
        if val == 0 {
            self.af.set_z(true);
        }
        self.af.set_n(true);
        unimplemented!("Failed to handle h and c flags");
    }

    fn swap(&mut self, reg: Register) {
        use Register::*;
        let result = match reg {
            A => {
                let a = self.af.a();
                let res = ((a & 0x0f) << 4) | ((a & 0xf0) >> 4);
                self.af.set_a(res);
                res
            }
            B => {
                let b = self.bc[0];
                let res = ((b & 0x0f) << 4) | ((b & 0xf0) >> 4);
                self.bc[0] = res;
                res
            }
            C => {
                let c = self.bc[1];
                let res = ((c & 0x0f) << 4) | ((c & 0xf0) >> 4);
                self.bc[1] = res;
                res
            }
            D => {
                let d = self.de[0];
                let res = ((d & 0x0f) << 4) | ((d & 0xf0) >> 4);
                self.de[0] = res;
                res
            }
            E => {
                let e = self.de[1];
                let res = ((e & 0x0f) << 4) | ((e & 0xf0) >> 4);
                self.de[1] = res;
                res
            }
            H => {
                let h = self.hl[0];
                let res = ((h & 0x0f) << 4) | ((h & 0xf0) >> 4);
                self.hl[0] = res;
                res
            }
            L => {
                let l = self.hl[1];
                let res = ((l & 0x0f) << 4) | ((l & 0xf0) >> 4);
                self.hl[1] = res;
                res
            }
            HL => {
                let bytes = self.memory.get_address_mut(self.hl.into());
                let byte = bytes[0];
                let res = ((byte & 0x0f) << 4) | ((byte & 0xf0) >> 4);
                bytes[0] = res;
                res
            }
            _ => unreachable!(),
        };

        self.af.reset_flags();
        self.af.set_z(result == 0);
    }

    fn cpl(&mut self) {
        self.af.set_a(!self.af.a());
        self.af.set_n(true);
        self.af.set_h(true);
    }
}

#[derive(Clone, Copy)]
struct GPReg([u8; 2]);

impl Deref for GPReg {
    type Target = [u8; 2];

    fn deref(&self) -> &Self::Target {
        &self.0
    }
}

impl DerefMut for GPReg {
    fn deref_mut(&mut self) -> &mut Self::Target {
        &mut self.0
    }
}

impl From<u16> for GPReg {
    fn from(x: u16) -> Self {
        Self(x.to_ne_bytes())
    }
}

impl From<GPReg> for u16 {
    fn from(x: GPReg) -> Self {
        u16::from_ne_bytes(*x)
    }
}

#[bitfield]
#[derive(Clone, Copy, Debug)]
struct AFReg {
    a: u8,
    #[skip]
    __: B4,
    c: bool,
    h: bool,
    n: bool,
    z: bool,
}

impl AFReg {
    fn f(&self) -> u8 {
        self.into_bytes()[1]
    }

    fn reset_flags(&mut self) {
        self.set_c(false);
        self.set_h(false);
        self.set_n(false);
        self.set_z(false);
    }
}

impl From<u16> for AFReg {
    fn from(x: u16) -> Self {
        Self::from_bytes(x.to_ne_bytes())
    }
}

impl From<AFReg> for u16 {
    fn from(x: AFReg) -> Self {
        u16::from_ne_bytes(x.into_bytes())
    }
}

#[test]
fn test_cpu() {
    let mut cpu = Cpu::new();
    cpu.af.set_z(true);
    assert_eq!(cpu.af.z(), true);
    cpu.af.set_z(false);
    assert_eq!(cpu.af.z(), false);
    cpu.af.set_z(true);
    assert_eq!(cpu.get_regu8(Register::F), 0b10000000);
}
