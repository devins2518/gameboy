use modular_bitfield::prelude::*;
use std::ops::{Deref, DerefMut};

use crate::memory::Bus;

pub struct Cpu {
    af: AFReg,
    bc: GPReg,
    de: GPReg,
    hl: GPReg,
    sp: u16,
    pc: u16,
    memory: Bus,
}

#[derive(Clone, Copy, Debug)]
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
            memory: Bus::new(),
        }
    }
    pub fn clock(&mut self) {
        let opcode = self.memory.get_address(self.pc);
        self.pc = self.pc.wrapping_add(1);
        match opcode {
            0x00 => self.nop(),
            0x01 => {
                let b1 = self.memory.get_address(self.pc);
                self.pc = self.pc.wrapping_add(1);
                let b2 = self.memory.get_address(self.pc);
                self.pc = self.pc.wrapping_add(1);
                self.ld_regu16(Register::BC, u16::from_le_bytes([b1, b2]));
            }
            0x02 => self.memory.write_byte(self.hl.into(), self.af.a()),
            0x03 => self.inc(Register::BC),
            0x04 => self.inc(Register::B),
            0x05 => self.dec(Register::B),
            0x06 => {
                let b = self.memory.get_address(self.pc);
                self.ld_regu8(Register::B, b);
            }
            0x07 => self.rlca(),
            0x08 => {
                let bytes = self.sp.to_le_bytes();
                self.memory.write_byte(self.pc, bytes[0]);
                self.pc = self.pc.wrapping_add(1);
                self.memory.write_byte(self.pc, bytes[1]);
            }
            0x09 => self.add(Register::HL, self.get_regu8(Register::BC)),
            0x0A => self.ld_regu8(
                Register::A,
                self.memory.get_address(self.get_regu16(Register::BC)),
            ),
            0x0B => self.dec(Register::BC),
            0x0C => self.inc(Register::C),
            0x0D => self.dec(Register::C),
            0x0E => self.ld_regu8(Register::C, self.memory.get_address(self.pc)),
            0x0F => self.rrca(),

            _ => unimplemented!("Unhandled opcode {:#x}", opcode),
        }
        self.pc += 1;
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
            _ => unreachable!("Attempted to get u16 from set_regu8"),
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
            _ => unreachable!("Attempted to set regu16 into regu8"),
        }
    }
    fn set_regu16(&self, reg: Register, val: u16) {
        use Register::*;
        let ref mut reg = match reg {
            AF => self.af.into(),
            BC => self.bc.into(),
            DE => self.de.into(),
            HL => self.hl.into(),
            _ => unreachable!("Attempted to set regu8 into regu16"),
        };
        *reg = val;
    }

    fn ld_regu8(&mut self, reg: Register, n: u8) {
        self.set_regu8(reg, n);
    }

    fn ld_regu16(&mut self, reg: Register, n: u16) {
        self.set_regu16(reg, n);
    }

    fn inc(&mut self, reg: Register) {
        use Register::*;

        let res = match reg {
            A => self.af.a().wrapping_add(1),
            B => self.bc[0].wrapping_add(1),
            C => self.bc[1].wrapping_add(1),
            D => self.de[0].wrapping_add(1),
            E => self.de[1].wrapping_add(1),
            H => self.hl[0].wrapping_add(1),
            L => self.hl[1].wrapping_add(1),
            BC => {
                let res = self.memory.get_address(self.bc.into()).wrapping_add(1);
                self.memory.write_byte(self.bc.into(), res);
                res
            }
            DE => {
                let res = self.memory.get_address(self.de.into()).wrapping_add(1);
                self.memory.write_byte(self.de.into(), res);
                res
            }
            HL => {
                let res = self.memory.get_address(self.hl.into()).wrapping_add(1);
                self.memory.write_byte(self.hl.into(), res);
                res
            }
            _ => unimplemented!(),
        };

        self.af.set_z(res == 0);
        self.af.set_n(false);
        unimplemented!("H flag")
    }

    fn dec(&mut self, reg: Register) {
        use Register::*;

        let res = match reg {
            A => self.af.a().wrapping_sub(1),
            B => self.bc[0].wrapping_sub(1),
            C => self.bc[1].wrapping_sub(1),
            D => self.de[0].wrapping_sub(1),
            E => self.de[1].wrapping_sub(1),
            H => self.hl[0].wrapping_sub(1),
            L => self.hl[1].wrapping_sub(1),
            BC => {
                let res = self.memory.get_address(self.bc.into()).wrapping_sub(1);
                self.memory.write_byte(self.bc.into(), res);
                res
            }
            DE => {
                let res = self.memory.get_address(self.de.into()).wrapping_sub(1);
                self.memory.write_byte(self.de.into(), res);
                res
            }
            HL => {
                let res = self.memory.get_address(self.hl.into()).wrapping_sub(1);
                self.memory.write_byte(self.hl.into(), res);
                res
            }
            _ => unimplemented!(),
        };

        self.af.set_z(res == 0);
        self.af.set_n(false);
        unimplemented!("H flag")
    }

    fn push(&mut self, reg: Register) {
        use Register::*;

        let ref reg = match reg {
            AF => self.af.into_bytes(),
            BC => self.bc.0,
            DE => self.de.0,
            HL => self.hl.0,
            _ => unreachable!("Attempted to push single byte register onto stack"),
        };

        self.memory.write_byte(self.sp, reg[0]);
        self.sp = self.sp.wrapping_sub(1);
        self.memory.write_byte(self.sp, reg[1]);
        self.sp = self.sp.wrapping_sub(1);
    }

    fn pop(&mut self, reg: Register) {
        use Register::*;

        let reg = match reg {
            AF => self.af.into_bytes(),
            BC => self.bc.0,
            DE => self.de.0,
            HL => self.hl.0,
            _ => unreachable!("Attempted to pop single byte register onto stack"),
        };

        self.memory.write_byte(self.sp, reg[0]);
        self.sp = self.sp.wrapping_add(1);
        self.memory.write_byte(self.sp, reg[1]);
        self.sp = self.sp.wrapping_add(1);
    }

    fn add(&mut self, reg: Register, n: u8) {
        let (val, over) = self.get_regu8(reg).overflowing_add(n);
        self.set_regu8(reg, val);
        self.af.set_z(over);
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
                let b = self.memory.get_address(self.hl.into());
                let res = ((b & 0x0f) << 4) | ((b & 0xf0) >> 4);
                self.memory.write_byte(self.hl.into(), res);
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

    fn ccf(&mut self) {
        self.af.set_n(false);
        self.af.set_h(false);
        self.af.set_c(!self.af.c());
    }

    fn scf(&mut self) {
        self.af.set_n(false);
        self.af.set_h(false);
        self.af.set_c(true);
    }

    fn nop(&self) {}

    fn halt(&mut self) {
        unimplemented!("Halt instruction encountered")
    }

    fn stop(&mut self) {
        unimplemented!("Stop instruction encountered")
    }

    fn di(&mut self) {
        unimplemented!("Disable interrupts instruction encountered")
    }

    fn ei(&mut self) {
        unimplemented!("Enable interrupts instruction encountered")
    }

    fn rlca(&mut self) {
        self.af.set_c(self.af.a() >> 7 == 1);
        let res = self.af.a().rotate_left(1);
        self.af.set_a(res);
        self.af.set_z(res == 0);
        unimplemented!();
    }

    fn rla(&mut self) {
        unimplemented!()
    }

    fn rrca(&mut self) {
        unimplemented!();
    }

    fn rra(&mut self) {
        unimplemented!()
    }

    fn rlc(&mut self) {
        unimplemented!()
    }

    fn rl(&mut self) {
        unimplemented!()
    }

    fn rrc(&mut self) {
        unimplemented!()
    }

    fn rr(&mut self) {
        unimplemented!()
    }

    fn sla(&mut self) {
        unimplemented!()
    }

    fn sra(&mut self) {
        unimplemented!()
    }

    fn srl(&mut self) {
        unimplemented!()
    }

    fn bit(&mut self, bit: u8, reg: Register) {
        use Register::*;
        let val = match reg {
            A => self.af.a(),
            B => self.bc[0],
            C => self.bc[1],
            D => self.de[0],
            E => self.de[1],
            H => self.hl[0],
            L => self.hl[1],
            HL => self.memory.get_address(self.hl.into()),
            _ => unreachable!("Tried to access invalid register in bit()"),
        };

        assert!(bit <= 7);

        self.af.set_z((val >> bit) == 0);
        self.af.set_n(false);
        self.af.set_h(true);
    }

    fn set(&mut self, bit: u8, reg: Register) {
        assert!(bit <= 7);
        let val = 1 << bit;

        use Register::*;
        match reg {
            A => {
                self.af.set_a(self.af.a() | 1 << bit);
            }
            B => {
                self.bc[0] |= val;
            }
            C => {
                self.bc[1] |= val;
            }
            D => {
                self.de[0] |= val;
            }
            E => {
                self.de[1] |= val;
            }
            H => {
                self.hl[0] |= val;
            }
            L => {
                self.hl[1] |= val;
            }
            HL => {
                let b = self.memory.get_address(self.hl.into());
                self.memory.write_byte(self.hl.into(), b | val);
            }
            _ => unreachable!("Tried to access invalid register in set()"),
        };
    }

    fn res(&mut self, bit: u8, reg: Register) {
        assert!(bit <= 7);
        let val = !(1 << bit);

        use Register::*;
        match reg {
            A => {
                self.af.set_a(self.af.a() & val);
            }
            B => {
                self.bc[0] &= val;
            }
            C => {
                self.bc[1] &= val;
            }
            D => {
                self.de[0] &= val;
            }
            E => {
                self.de[1] &= val;
            }
            H => {
                self.hl[0] &= val;
            }
            L => {
                self.hl[1] &= val;
            }
            HL => {
                let b = self.memory.get_address(self.hl.into());
                self.memory.write_byte(self.hl.into(), b & val);
            }
            _ => unreachable!("Tried to access invalid register in res()"),
        };
    }

    fn jp(&mut self, val: u16) {
        unimplemented!()
    }

    fn jpc(&mut self, cond: Condition, val: u16) {
        let cond = match cond {
            Condition::NZ => self.af.z() == false,
            Condition::Z => self.af.z() == true,
            Condition::NC => self.af.c() == false,
            Condition::C => self.af.c() == true,
        };

        if cond {
            self.pc = val;
        }
    }

    fn jr(&mut self, n: i8) {
        if n <= 0 {
            self.pc += (0 - n) as u16
        } else {
            self.pc += n as u16
        }
    }

    fn jrc(&mut self, cond: Condition, n: u8) {
        let cond = match cond {
            Condition::NZ => self.af.z() == false,
            Condition::Z => self.af.z() == true,
            Condition::NC => self.af.c() == false,
            Condition::C => self.af.c() == true,
        };

        if cond {
            if n <= 0 {
                self.pc += (0 - n) as u16
            } else {
                self.pc += n as u16
            }
        }
    }

    fn call(&mut self, addr: u16) {
        let b1 = self.memory.get_address(self.pc);
        let b2 = self.memory.get_address(self.pc + 1);

        // Push PC to stack
        self.memory.write_byte(self.pc, b1);
        self.sp = self.sp.wrapping_sub(1);
        self.memory.write_byte(self.pc, b2);
        self.sp = self.sp.wrapping_sub(1);

        self.pc = addr;
    }

    fn callc(&mut self, cond: Condition, addr: u16) {
        let cond = match cond {
            Condition::NZ => self.af.z() == false,
            Condition::Z => self.af.z() == true,
            Condition::NC => self.af.c() == false,
            Condition::C => self.af.c() == true,
        };

        if cond {
            let b1 = self.memory.get_address(self.pc);
            let b2 = self.memory.get_address(self.pc + 1);

            // Push PC to stack
            self.memory.write_byte(self.pc, b1);
            self.sp = self.sp.wrapping_sub(1);
            self.memory.write_byte(self.pc, b2);
            self.sp = self.sp.wrapping_sub(1);

            self.pc = addr;
        }
    }

    fn rst(&mut self) {
        unimplemented!()
    }

    fn ret(&mut self) {
        let b1 = self.memory.get_address(self.pc);
        let b2 = self.memory.get_address(self.pc + 1);

        // Push PC to stack
        self.pc = u16::from_le_bytes([b1, b2]);
        self.sp += 2;
    }

    fn retc(&mut self, cond: Condition) {
        let cond = match cond {
            Condition::NZ => self.af.z() == false,
            Condition::Z => self.af.z() == true,
            Condition::NC => self.af.c() == false,
            Condition::C => self.af.c() == true,
        };

        if cond {
            let b1 = self.memory.get_address(self.pc);
            let b2 = self.memory.get_address(self.pc + 1);

            // Push PC to stack
            self.pc = u16::from_le_bytes([b2, b1]);
            self.sp = self.sp.wrapping_sub(2);
        }
    }

    fn reti(&mut self) {
        unimplemented!()
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

enum Condition {
    NZ,
    Z,
    NC,
    C,
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
    println!("{}", 0xA0 % 0x0);
}
