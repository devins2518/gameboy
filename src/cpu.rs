use std::ops::{Deref, DerefMut};

use crate::memory::Memory;

pub struct Cpu {
    regs: [GPReg; 3],
    af: GPReg,
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
            regs: [0.into(); 3],
            af: 0.into(),
            sp: 0x0000,
            pc: 0x0000,
            memory: Memory::new(),
        }
    }
    fn get_z(&self) -> bool {
        self.af[1] >> 7 == 1
    }
    fn get_n(&self) -> bool {
        (self.af[0] >> 6) & 0x1 == 1
    }
    fn get_h(&self) -> bool {
        (self.af[1] >> 5) & 0x1 == 1
    }
    fn get_c(&self) -> bool {
        (self.af[0] >> 4) & 0x1 == 1
    }
    fn get_regu8(&self, reg: Register) -> u8 {
        use Register::*;
        match reg {
            A => self.af[0],
            F => self.af[1],
            B => self.regs[0][0],
            C => self.regs[0][1],
            D => self.regs[1][0],
            E => self.regs[1][1],
            H => self.regs[2][0],
            L => self.regs[2][1],
            _ => unreachable!("Attempted to get u16 from get_regu8"),
        }
    }
    fn set_regu8(&mut self, reg: Register, val: u8) {
        use Register::*;
        let ref mut reg = match reg {
            A => self.af[0],
            F => self.af[1],
            B => self.regs[0][0],
            C => self.regs[0][1],
            D => self.regs[1][0],
            E => self.regs[1][1],
            H => self.regs[2][0],
            L => self.regs[2][1],
            _ => unreachable!("Attempted to get u16 from get_regu8"),
        };
        *reg = val;
    }
    fn get_regu16(&self, reg: Register) -> u16 {
        use Register::*;
        match reg {
            AF => self.af.into(),
            BC => self.regs[0].into(),
            DE => self.regs[1].into(),
            HL => self.regs[2].into(),
            _ => unreachable!("Attempted to get u16 from get_regu8"),
        }
    }
    fn set_regu16(&self, reg: Register, val: u16) {
        use Register::*;
        let ref mut reg = match reg {
            AF => self.af.into(),
            BC => self.regs[0].into(),
            DE => self.regs[1].into(),
            HL => self.regs[2].into(),
            _ => unreachable!("Attempted to get u16 from get_regu8"),
        };
        *reg = val;
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
