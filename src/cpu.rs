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
    SP,
    PC,
    PAF,
    PBC,
    PDE,
    PHL,
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

        #[cfg(debug_assertions)]
        println!("Matching opcode: {:#X}", opcode);

        match opcode {
            0x00 => self.nop(),
            0x01 => {
                let b1 = self.memory.get_address(self.pc);
                self.pc = self.pc.wrapping_add(1);
                let b2 = self.memory.get_address(self.pc);
                self.pc = self.pc.wrapping_add(1);
                self.ld_regu16(Register::BC, u16::from_ne_bytes([b1, b2]));
            }
            0x02 => self.memory.write_byte(self.bc.into(), self.af.a()),
            0x03 => self.inc(Register::BC),
            0x04 => self.inc(Register::B),
            0x05 => self.dec(Register::B),
            0x06 => {
                let b = self.memory.get_address(self.pc);
                self.pc = self.pc.wrapping_add(self.pc);
                self.ld_regu8(Register::B, b);
            }
            0x07 => self.rlca(),
            0x08 => {
                let bytes = self.sp.to_le_bytes();
                self.memory.write_byte(self.pc, bytes[0]);
                self.pc = self.pc.wrapping_add(1);
                self.memory.write_byte(self.pc, bytes[1]);
            }
            0x09 => self.add_u8(Register::HL, self.get_regu8(Register::BC)),
            0x0A => self.ld_regu8(
                Register::A,
                self.memory.get_address(self.get_regu16(Register::BC)),
            ),
            0x0B => self.dec(Register::BC),
            0x0C => self.inc(Register::C),
            0x0D => self.dec(Register::C),
            0x0E => {
                self.ld_regu8(Register::C, self.memory.get_address(self.pc));
                self.pc = self.pc.wrapping_add(self.pc);
            }
            0x0F => self.rrca(),
            0x10 => self.stop(),
            0x11 => {
                let b1 = self.memory.get_address(self.pc);
                self.pc = self.pc.wrapping_add(1);
                let b2 = self.memory.get_address(self.pc);
                self.pc = self.pc.wrapping_add(1);
                self.ld_regu16(Register::DE, u16::from_ne_bytes([b1, b2]));
            }
            0x12 => self.memory.write_byte(self.de.into(), self.af.a()),
            0x13 => self.inc(Register::DE),
            0x14 => self.inc(Register::D),
            0x15 => self.dec(Register::D),
            0x16 => {
                self.ld_regu8(Register::D, self.memory.get_address(self.pc));
                self.pc = self.pc.wrapping_add(1);
            }
            0x17 => self.rla(),
            0x18 => {
                self.jr(self.memory.get_address(self.pc) as i8);
                self.pc = self.pc.wrapping_add(1);
            }
            0x19 => self.add_u8(Register::HL, self.get_regu8(Register::DE)),
            0x1A => self.ld_regu8(
                Register::A,
                self.memory.get_address(self.get_regu16(Register::DE)),
            ),
            0x1B => self.dec(Register::DE),
            0x1C => self.inc(Register::E),
            0x1D => self.dec(Register::E),
            0x1E => {
                self.ld_regu8(Register::E, self.memory.get_address(self.pc));
                self.pc = self.pc.wrapping_add(1);
            }
            0x1F => self.rra(),
            0x20 => {
                self.jrc(Condition::NZ, self.memory.get_address(self.pc) as i8);
                self.pc = self.pc.wrapping_add(1);
            }
            0x21 => {
                let b1 = self.memory.get_address(self.pc);
                self.pc = self.pc.wrapping_add(1);
                let b2 = self.memory.get_address(self.pc);
                self.pc = self.pc.wrapping_add(1);
                self.ld_regu16(Register::HL, u16::from_ne_bytes([b1, b2]));
            }
            0x22 => {
                self.memory
                    .write_byte(self.get_regu16(Register::HL), self.get_regu8(Register::A));
                self.hl = GPReg::from(self.get_regu16(Register::HL).wrapping_add(1));
            }
            0x23 => self.inc(Register::HL),
            0x24 => self.inc(Register::H),
            0x25 => self.dec(Register::H),
            0x26 => {
                self.ld_regu8(Register::H, self.memory.get_address(self.pc));
                self.pc = self.pc.wrapping_add(1);
            }
            0x27 => self.daa(),
            0x28 => {
                self.jrc(Condition::Z, self.memory.get_address(self.pc) as i8);
                self.pc = self.pc.wrapping_add(1);
            }
            0x29 => self.add_u16(Register::HL, self.get_regu16(Register::HL)),
            0x2A => {
                self.ld_regu8(
                    Register::A,
                    self.memory.get_address(self.get_regu16(Register::HL)),
                );
                self.hl = GPReg::from(self.get_regu16(Register::HL).wrapping_add(1));
            }
            0x2B => {
                // dec(sp)
                unimplemented!()
            }
            0x2C => self.inc(Register::L),
            0x2D => self.dec(Register::L),
            0x2E => {
                self.ld_regu8(Register::L, self.memory.get_address(self.pc));
                self.pc = self.pc.wrapping_add(1);
            }
            0x2F => self.cpl(),
            0x30 => {
                self.jrc(Condition::NC, self.memory.get_address(self.pc) as i8);
                self.pc = self.pc.wrapping_add(1);
            }
            0x31 => {
                let b1 = self.memory.get_address(self.pc);
                self.pc = self.pc.wrapping_add(1);
                let b2 = self.memory.get_address(self.pc);
                self.pc = self.pc.wrapping_add(1);
                let n = u16::from_ne_bytes([b1, b2]);
                self.ld_regu16(Register::SP, n);
            }
            0x32 => {
                self.memory
                    .write_byte(self.get_regu16(Register::HL), self.get_regu8(Register::A));
                self.hl = GPReg::from(self.get_regu16(Register::HL).wrapping_sub(1));
            }
            0x33 => self.inc(Register::SP),
            0x34 => self.inc(Register::PHL),
            0x35 => self.dec(Register::PHL),
            0x36 => {
                self.ld_regu8(Register::PHL, self.memory.get_address(self.pc));
                self.pc = self.pc.wrapping_add(1);
            }
            0x37 => self.scf(),
            0x38 => {
                self.jrc(Condition::C, self.memory.get_address(self.pc) as i8);
                self.pc = self.pc.wrapping_add(1);
            }
            0x39 => self.add_u16(Register::HL, self.get_regu16(Register::SP)),
            0x3A => {
                self.ld_regu8(Register::A, self.get_regu8(Register::PHL));
                self.memory.write_byte(
                    self.get_regu16(Register::PHL),
                    self.memory
                        .get_address(self.get_regu16(Register::PHL))
                        .wrapping_sub(1),
                );
            }
            0x3B => self.dec(Register::SP),
            0x3C => self.inc(Register::A),
            0x3D => self.dec(Register::A),
            0x3E => {
                let b = self.memory.get_address(self.pc);
                self.pc = self.pc.wrapping_add(self.pc);
                self.ld_regu8(Register::A, b);
            }
            0x3F => self.ccf(),
            0x40 => self.ld_regu8(Register::B, self.get_regu8(Register::B)),
            0x41 => self.ld_regu8(Register::B, self.get_regu8(Register::C)),
            0x42 => self.ld_regu8(Register::B, self.get_regu8(Register::D)),
            0x43 => self.ld_regu8(Register::B, self.get_regu8(Register::E)),
            0x44 => self.ld_regu8(Register::B, self.get_regu8(Register::H)),
            0x45 => self.ld_regu8(Register::B, self.get_regu8(Register::L)),
            0x46 => self.ld_regu8(Register::B, self.get_regu8(Register::PHL)),
            0x47 => self.ld_regu8(Register::B, self.get_regu8(Register::A)),
            0x48 => self.ld_regu8(Register::C, self.get_regu8(Register::B)),
            0x49 => self.ld_regu8(Register::C, self.get_regu8(Register::C)),
            0x4A => self.ld_regu8(Register::C, self.get_regu8(Register::D)),
            0x4B => self.ld_regu8(Register::C, self.get_regu8(Register::E)),
            0x4C => self.ld_regu8(Register::C, self.get_regu8(Register::H)),
            0x4D => self.ld_regu8(Register::C, self.get_regu8(Register::L)),
            0x4E => self.ld_regu8(Register::C, self.get_regu8(Register::PHL)),
            0x4F => self.ld_regu8(Register::C, self.get_regu8(Register::A)),
            0x50 => self.ld_regu8(Register::D, self.get_regu8(Register::B)),
            0x51 => self.ld_regu8(Register::D, self.get_regu8(Register::C)),
            0x52 => self.ld_regu8(Register::D, self.get_regu8(Register::D)),
            0x53 => self.ld_regu8(Register::D, self.get_regu8(Register::E)),
            0x54 => self.ld_regu8(Register::D, self.get_regu8(Register::H)),
            0x55 => self.ld_regu8(Register::D, self.get_regu8(Register::L)),
            0x56 => self.ld_regu8(Register::D, self.get_regu8(Register::PHL)),
            0x57 => self.ld_regu8(Register::D, self.get_regu8(Register::A)),
            0x58 => self.ld_regu8(Register::E, self.get_regu8(Register::B)),
            0x59 => self.ld_regu8(Register::E, self.get_regu8(Register::C)),
            0x5A => self.ld_regu8(Register::E, self.get_regu8(Register::D)),
            0x5B => self.ld_regu8(Register::E, self.get_regu8(Register::E)),
            0x5C => self.ld_regu8(Register::E, self.get_regu8(Register::H)),
            0x5D => self.ld_regu8(Register::E, self.get_regu8(Register::L)),
            0x5E => self.ld_regu8(Register::E, self.get_regu8(Register::PHL)),
            0x5F => self.ld_regu8(Register::E, self.get_regu8(Register::A)),
            0x60 => self.ld_regu8(Register::H, self.get_regu8(Register::B)),
            0x61 => self.ld_regu8(Register::H, self.get_regu8(Register::C)),
            0x62 => self.ld_regu8(Register::H, self.get_regu8(Register::D)),
            0x63 => self.ld_regu8(Register::H, self.get_regu8(Register::E)),
            0x64 => self.ld_regu8(Register::H, self.get_regu8(Register::H)),
            0x65 => self.ld_regu8(Register::H, self.get_regu8(Register::L)),
            0x66 => self.ld_regu8(Register::H, self.get_regu8(Register::PHL)),
            0x67 => self.ld_regu8(Register::H, self.get_regu8(Register::A)),
            0x68 => self.ld_regu8(Register::L, self.get_regu8(Register::B)),
            0x69 => self.ld_regu8(Register::L, self.get_regu8(Register::C)),
            0x6A => self.ld_regu8(Register::L, self.get_regu8(Register::D)),
            0x6B => self.ld_regu8(Register::L, self.get_regu8(Register::E)),
            0x6C => self.ld_regu8(Register::L, self.get_regu8(Register::H)),
            0x6D => self.ld_regu8(Register::L, self.get_regu8(Register::L)),
            0x6E => self.ld_regu8(Register::L, self.get_regu8(Register::PHL)),
            0x6F => self.ld_regu8(Register::L, self.get_regu8(Register::A)),
            0x70 => self.ld_regu8(Register::PHL, self.get_regu8(Register::B)),
            0x71 => self.ld_regu8(Register::PHL, self.get_regu8(Register::C)),
            0x72 => self.ld_regu8(Register::PHL, self.get_regu8(Register::D)),
            0x73 => self.ld_regu8(Register::PHL, self.get_regu8(Register::E)),
            0x74 => self.ld_regu8(Register::PHL, self.get_regu8(Register::H)),
            0x75 => self.ld_regu8(Register::PHL, self.get_regu8(Register::L)),
            0x76 => self.halt(),
            0x77 => self.ld_regu8(Register::PHL, self.get_regu8(Register::A)),
            0x78 => self.ld_regu8(Register::A, self.get_regu8(Register::B)),
            0x79 => self.ld_regu8(Register::A, self.get_regu8(Register::C)),
            0x7A => self.ld_regu8(Register::A, self.get_regu8(Register::D)),
            0x7B => self.ld_regu8(Register::A, self.get_regu8(Register::E)),
            0x7C => self.ld_regu8(Register::A, self.get_regu8(Register::H)),
            0x7D => self.ld_regu8(Register::A, self.get_regu8(Register::L)),
            0x7E => self.ld_regu8(Register::A, self.get_regu8(Register::PHL)),
            0x7F => self.ld_regu8(Register::A, self.get_regu8(Register::A)),
            0x80 => self.add_u8(Register::A, self.get_regu8(Register::B)),
            0x81 => self.add_u8(Register::A, self.get_regu8(Register::C)),
            0x82 => self.add_u8(Register::A, self.get_regu8(Register::D)),
            0x83 => self.add_u8(Register::A, self.get_regu8(Register::E)),
            0x84 => self.add_u8(Register::A, self.get_regu8(Register::H)),
            0x85 => self.add_u8(Register::A, self.get_regu8(Register::L)),
            0x86 => self.add_u8(Register::A, self.get_regu8(Register::PHL)),
            0x87 => self.add_u8(Register::A, self.get_regu8(Register::A)),
            // TODO
            // Unimplemented adc
            // 0x88 => self.adc(Register::A, self.get_regu8(Register::B)),
            // 0x89 => self.adc(Register::A, self.get_regu8(Register::C)),
            // 0x8A => self.adc(Register::A, self.get_regu8(Register::D)),
            // 0x8B => self.adc(Register::A, self.get_regu8(Register::E)),
            // 0x8C => self.adc(Register::A, self.get_regu8(Register::H)),
            // 0x8D => self.adc(Register::A, self.get_regu8(Register::L)),
            // 0x8E => self.adc(Register::A, self.get_regu8(Register::PHL)),
            // 0x8F => self.adc(Register::A, self.get_regu8(Register::A)),
            // TODO
            // Unimplemented sub
            // 0x90 => self.sub(Register::A, self.get_regu8(Register::B)),
            // 0x91 => self.sub(Register::A, self.get_regu8(Register::C)),
            // 0x92 => self.sub(Register::A, self.get_regu8(Register::D)),
            // 0x93 => self.sub(Register::A, self.get_regu8(Register::E)),
            // 0x94 => self.sub(Register::A, self.get_regu8(Register::H)),
            // 0x95 => self.sub(Register::A, self.get_regu8(Register::L)),
            // 0x96 => self.sub(Register::A, self.get_regu8(Register::PHL)),
            // 0x97 => self.sub(Register::A, self.get_regu8(Register::A)),
            // TODO
            // Unimplemented sbc
            // 0x98 => self.sbc(Register::A, self.get_regu8(Register::B)),
            // 0x99 => self.sbc(Register::A, self.get_regu8(Register::C)),
            // 0x9A => self.sbc(Register::A, self.get_regu8(Register::D)),
            // 0x9B => self.sbc(Register::A, self.get_regu8(Register::E)),
            // 0x9C => self.sbc(Register::A, self.get_regu8(Register::H)),
            // 0x9D => self.sbc(Register::A, self.get_regu8(Register::L)),
            // 0x9E => self.sbc(Register::A, self.get_regu8(Register::PHL)),
            // 0x9F => self.sbc(Register::A, self.get_regu8(Register::A)),
            // TODO
            // Unimplemented and
            // 0xA0 => self.and(Register::A, self.get_regu8(Register::B)),
            // 0xA1 => self.and(Register::A, self.get_regu8(Register::C)),
            // 0xA2 => self.and(Register::A, self.get_regu8(Register::D)),
            // 0xA3 => self.and(Register::A, self.get_regu8(Register::E)),
            // 0xA4 => self.and(Register::A, self.get_regu8(Register::H)),
            // 0xA5 => self.and(Register::A, self.get_regu8(Register::L)),
            // 0xA6 => self.and(Register::A, self.get_regu8(Register::PHL)),
            // 0xA7 => self.and(Register::A, self.get_regu8(Register::A)),
            0xA8 => self.xor(self.get_regu8(Register::B)),
            0xA9 => self.xor(self.get_regu8(Register::C)),
            0xAA => self.xor(self.get_regu8(Register::D)),
            0xAB => self.xor(self.get_regu8(Register::E)),
            0xAC => self.xor(self.get_regu8(Register::H)),
            0xAD => self.xor(self.get_regu8(Register::L)),
            0xAE => self.xor(self.get_regu8(Register::PHL)),
            0xAF => self.xor(self.get_regu8(Register::A)),
            0xB0 => self.or(self.get_regu8(Register::B)),
            0xB1 => self.or(self.get_regu8(Register::C)),
            0xB2 => self.or(self.get_regu8(Register::D)),
            0xB3 => self.or(self.get_regu8(Register::E)),
            0xB4 => self.or(self.get_regu8(Register::H)),
            0xB5 => self.or(self.get_regu8(Register::L)),
            0xB6 => self.or(self.get_regu8(Register::PHL)),
            0xB7 => self.or(self.get_regu8(Register::A)),
            0xB8 => self.cp(self.get_regu8(Register::B)),
            0xB9 => self.cp(self.get_regu8(Register::C)),
            0xBA => self.cp(self.get_regu8(Register::D)),
            0xBB => self.cp(self.get_regu8(Register::E)),
            0xBC => self.cp(self.get_regu8(Register::H)),
            0xBD => self.cp(self.get_regu8(Register::L)),
            0xBE => self.cp(self.get_regu8(Register::PHL)),
            0xBF => self.cp(self.get_regu8(Register::A)),
            0xC0 => self.retc(Condition::NZ),
            0xC1 => self.pop(Register::BC),
            0xC2 => {
                let b1 = self.memory.get_address(self.pc);
                self.pc = self.pc.wrapping_add(1);
                let b2 = self.memory.get_address(self.pc);
                let addr = u16::from_ne_bytes([b2, b1]);
                self.jpc(Condition::NZ, addr);
            }
            0xC3 => {
                let b1 = self.memory.get_address(self.pc);
                self.pc = self.pc.wrapping_add(1);
                let b2 = self.memory.get_address(self.pc);
                let addr = u16::from_ne_bytes([b2, b1]);
                self.jp(addr);
            }
            0xC4 => {
                let b1 = self.memory.get_address(self.pc);
                self.pc = self.pc.wrapping_add(1);
                let b2 = self.memory.get_address(self.pc);
                let addr = u16::from_ne_bytes([b2, b1]);
                self.callc(Condition::NZ, addr);
            }
            0xC5 => self.push(Register::BC),
            0xC6 => {
                let b = self.memory.get_address(self.pc);
                self.pc = self.pc.wrapping_add(1);
                self.add_u8(Register::A, b);
            }
            0xC7 => self.rst(),
            0xC8 => self.retc(Condition::C),
            0xC9 => self.retc(Condition::Z),
            0xCA => self.ret(),
            0xCB => {
                let cb = self.memory.get_address(self.pc);
                self.pc = self.pc.wrapping_add(1);

                match cb {
                    0x00 => self.rlc(Register::B),
                    0x01 => self.rlc(Register::C),
                    0x02 => self.rlc(Register::D),
                    0x03 => self.rlc(Register::E),
                    0x04 => self.rlc(Register::H),
                    0x05 => self.rlc(Register::L),
                    0x06 => self.rlc(Register::PHL),
                    0x07 => self.rlc(Register::A),
                    0x08 => self.rrc(Register::B),
                    0x09 => self.rrc(Register::C),
                    0x0A => self.rrc(Register::D),
                    0x0B => self.rrc(Register::E),
                    0x0C => self.rrc(Register::H),
                    0x0D => self.rrc(Register::L),
                    0x0E => self.rrc(Register::PHL),
                    0x0F => self.rrc(Register::A),
                    0x10 => self.rl(Register::B),
                    0x11 => self.rl(Register::C),
                    0x12 => self.rl(Register::D),
                    0x13 => self.rl(Register::E),
                    0x14 => self.rl(Register::H),
                    0x15 => self.rl(Register::L),
                    0x16 => self.rl(Register::PHL),
                    0x17 => self.rl(Register::A),
                    0x18 => self.rr(Register::B),
                    0x19 => self.rr(Register::C),
                    0x1A => self.rr(Register::D),
                    0x1B => self.rr(Register::E),
                    0x1C => self.rr(Register::H),
                    0x1D => self.rr(Register::L),
                    0x1E => self.rr(Register::PHL),
                    0x1F => self.rr(Register::A),
                    0x20 => self.sla(Register::B),
                    0x21 => self.sla(Register::C),
                    0x22 => self.sla(Register::D),
                    0x23 => self.sla(Register::E),
                    0x24 => self.sla(Register::H),
                    0x25 => self.sla(Register::L),
                    0x26 => self.sla(Register::PHL),
                    0x27 => self.sla(Register::A),
                    0x28 => self.sra(Register::B),
                    0x29 => self.sra(Register::C),
                    0x2A => self.sra(Register::D),
                    0x2B => self.sra(Register::E),
                    0x2C => self.sra(Register::H),
                    0x2D => self.sra(Register::L),
                    0x2E => self.sra(Register::PHL),
                    0x2F => self.sra(Register::A),
                    0x30 => self.swap(Register::B),
                    0x31 => self.swap(Register::C),
                    0x32 => self.swap(Register::D),
                    0x33 => self.swap(Register::E),
                    0x34 => self.swap(Register::H),
                    0x35 => self.swap(Register::L),
                    0x36 => self.swap(Register::PHL),
                    0x37 => self.swap(Register::A),
                    0x38 => self.srl(Register::B),
                    0x39 => self.srl(Register::C),
                    0x3A => self.srl(Register::D),
                    0x3B => self.srl(Register::E),
                    0x3C => self.srl(Register::H),
                    0x3D => self.srl(Register::L),
                    0x3E => self.srl(Register::PHL),
                    0x3F => self.srl(Register::A),
                    0x40 => self.bit(0, Register::B),
                    0x41 => self.bit(0, Register::C),
                    0x42 => self.bit(0, Register::D),
                    0x43 => self.bit(0, Register::E),
                    0x44 => self.bit(0, Register::H),
                    0x45 => self.bit(0, Register::L),
                    0x46 => self.bit(0, Register::PHL),
                    0x47 => self.bit(0, Register::A),
                    0x48 => self.bit(1, Register::B),
                    0x49 => self.bit(1, Register::C),
                    0x4A => self.bit(1, Register::D),
                    0x4B => self.bit(1, Register::E),
                    0x4C => self.bit(1, Register::H),
                    0x4D => self.bit(1, Register::L),
                    0x4E => self.bit(1, Register::PHL),
                    0x4F => self.bit(1, Register::A),
                    0x50 => self.bit(2, Register::B),
                    0x51 => self.bit(2, Register::C),
                    0x52 => self.bit(2, Register::D),
                    0x53 => self.bit(2, Register::E),
                    0x54 => self.bit(2, Register::H),
                    0x55 => self.bit(2, Register::L),
                    0x56 => self.bit(2, Register::PHL),
                    0x57 => self.bit(2, Register::A),
                    0x58 => self.bit(3, Register::B),
                    0x59 => self.bit(3, Register::C),
                    0x5A => self.bit(3, Register::D),
                    0x5B => self.bit(3, Register::E),
                    0x5C => self.bit(3, Register::H),
                    0x5D => self.bit(3, Register::L),
                    0x5E => self.bit(3, Register::PHL),
                    0x5F => self.bit(3, Register::A),
                    0x60 => self.bit(4, Register::B),
                    0x61 => self.bit(4, Register::C),
                    0x62 => self.bit(4, Register::D),
                    0x63 => self.bit(4, Register::E),
                    0x64 => self.bit(4, Register::H),
                    0x65 => self.bit(4, Register::L),
                    0x66 => self.bit(4, Register::PHL),
                    0x67 => self.bit(4, Register::A),
                    0x68 => self.bit(5, Register::B),
                    0x69 => self.bit(5, Register::C),
                    0x6A => self.bit(5, Register::D),
                    0x6B => self.bit(5, Register::E),
                    0x6C => self.bit(5, Register::H),
                    0x6D => self.bit(5, Register::L),
                    0x6E => self.bit(5, Register::PHL),
                    0x6F => self.bit(5, Register::A),
                    0x70 => self.bit(6, Register::B),
                    0x71 => self.bit(6, Register::C),
                    0x72 => self.bit(6, Register::D),
                    0x73 => self.bit(6, Register::E),
                    0x74 => self.bit(6, Register::H),
                    0x75 => self.bit(6, Register::L),
                    0x76 => self.bit(6, Register::PHL),
                    0x77 => self.bit(6, Register::A),
                    0x78 => self.bit(7, Register::B),
                    0x79 => self.bit(7, Register::C),
                    0x7A => self.bit(7, Register::D),
                    0x7B => self.bit(7, Register::E),
                    0x7C => self.bit(7, Register::H),
                    0x7D => self.bit(7, Register::L),
                    0x7E => self.bit(7, Register::PHL),
                    0x7F => self.bit(7, Register::A),
                    0x80 => self.res(0, Register::B),
                    0x81 => self.res(0, Register::C),
                    0x82 => self.res(0, Register::D),
                    0x83 => self.res(0, Register::E),
                    0x84 => self.res(0, Register::H),
                    0x85 => self.res(0, Register::L),
                    0x86 => self.res(0, Register::PHL),
                    0x87 => self.res(0, Register::A),
                    0x88 => self.res(1, Register::B),
                    0x89 => self.res(1, Register::C),
                    0x8A => self.res(1, Register::D),
                    0x8B => self.res(1, Register::E),
                    0x8C => self.res(1, Register::H),
                    0x8D => self.res(1, Register::L),
                    0x8E => self.res(1, Register::PHL),
                    0x8F => self.res(1, Register::A),
                    0x90 => self.res(2, Register::B),
                    0x91 => self.res(2, Register::C),
                    0x92 => self.res(2, Register::D),
                    0x93 => self.res(2, Register::E),
                    0x94 => self.res(2, Register::H),
                    0x95 => self.res(2, Register::L),
                    0x96 => self.res(2, Register::PHL),
                    0x97 => self.res(2, Register::A),
                    0x98 => self.res(3, Register::B),
                    0x99 => self.res(3, Register::C),
                    0x9A => self.res(3, Register::D),
                    0x9B => self.res(3, Register::E),
                    0x9C => self.res(3, Register::H),
                    0x9D => self.res(3, Register::L),
                    0x9E => self.res(3, Register::PHL),
                    0x9F => self.res(3, Register::A),
                    0xA0 => self.res(4, Register::B),
                    0xA1 => self.res(4, Register::C),
                    0xA2 => self.res(4, Register::D),
                    0xA3 => self.res(4, Register::E),
                    0xA4 => self.res(4, Register::H),
                    0xA5 => self.res(4, Register::L),
                    0xA6 => self.res(4, Register::PHL),
                    0xA7 => self.res(4, Register::A),
                    0xA8 => self.res(5, Register::B),
                    0xA9 => self.res(5, Register::C),
                    0xAA => self.res(5, Register::D),
                    0xAB => self.res(5, Register::E),
                    0xAC => self.res(5, Register::H),
                    0xAD => self.res(5, Register::L),
                    0xAE => self.res(5, Register::PHL),
                    0xAF => self.res(5, Register::A),
                    0xB0 => self.res(6, Register::B),
                    0xB1 => self.res(6, Register::C),
                    0xB2 => self.res(6, Register::D),
                    0xB3 => self.res(6, Register::E),
                    0xB4 => self.res(6, Register::H),
                    0xB5 => self.res(6, Register::L),
                    0xB6 => self.res(6, Register::PHL),
                    0xB7 => self.res(6, Register::A),
                    0xB8 => self.res(7, Register::B),
                    0xB9 => self.res(7, Register::C),
                    0xBA => self.res(7, Register::D),
                    0xBB => self.res(7, Register::E),
                    0xBC => self.res(7, Register::H),
                    0xBD => self.res(7, Register::L),
                    0xBE => self.res(7, Register::PHL),
                    0xBF => self.res(7, Register::A),
                    0xC0 => self.set(0, Register::B),
                    0xC1 => self.set(0, Register::C),
                    0xC2 => self.set(0, Register::D),
                    0xC3 => self.set(0, Register::E),
                    0xC4 => self.set(0, Register::H),
                    0xC5 => self.set(0, Register::L),
                    0xC6 => self.set(0, Register::PHL),
                    0xC7 => self.set(0, Register::A),
                    0xC8 => self.set(1, Register::B),
                    0xC9 => self.set(1, Register::C),
                    0xCA => self.set(1, Register::D),
                    0xCB => self.set(1, Register::E),
                    0xCC => self.set(1, Register::H),
                    0xCD => self.set(1, Register::L),
                    0xCE => self.set(1, Register::PHL),
                    0xCF => self.set(1, Register::A),
                    0xD0 => self.set(2, Register::B),
                    0xD1 => self.set(2, Register::C),
                    0xD2 => self.set(2, Register::D),
                    0xD3 => self.set(2, Register::E),
                    0xD4 => self.set(2, Register::H),
                    0xD5 => self.set(2, Register::L),
                    0xD6 => self.set(2, Register::PHL),
                    0xD7 => self.set(2, Register::A),
                    0xD8 => self.set(3, Register::B),
                    0xD9 => self.set(3, Register::C),
                    0xDA => self.set(3, Register::D),
                    0xDB => self.set(3, Register::E),
                    0xDC => self.set(3, Register::H),
                    0xDD => self.set(3, Register::L),
                    0xDE => self.set(3, Register::PHL),
                    0xDF => self.set(3, Register::A),
                    0xE0 => self.set(4, Register::B),
                    0xE1 => self.set(4, Register::C),
                    0xE2 => self.set(4, Register::D),
                    0xE3 => self.set(4, Register::E),
                    0xE4 => self.set(4, Register::H),
                    0xE5 => self.set(4, Register::L),
                    0xE6 => self.set(4, Register::PHL),
                    0xE7 => self.set(4, Register::A),
                    0xE8 => self.set(5, Register::B),
                    0xE9 => self.set(5, Register::C),
                    0xEA => self.set(5, Register::D),
                    0xEB => self.set(5, Register::E),
                    0xEC => self.set(5, Register::H),
                    0xED => self.set(5, Register::L),
                    0xEE => self.set(5, Register::PHL),
                    0xEF => self.set(5, Register::A),
                    0xF0 => self.set(6, Register::B),
                    0xF1 => self.set(6, Register::C),
                    0xF2 => self.set(6, Register::D),
                    0xF3 => self.set(6, Register::E),
                    0xF4 => self.set(6, Register::H),
                    0xF5 => self.set(6, Register::L),
                    0xF6 => self.set(6, Register::PHL),
                    0xF7 => self.set(6, Register::A),
                    0xF8 => self.set(7, Register::B),
                    0xF9 => self.set(7, Register::C),
                    0xFA => self.set(7, Register::D),
                    0xFB => self.set(7, Register::E),
                    0xFC => self.set(7, Register::H),
                    0xFD => self.set(7, Register::L),
                    0xFE => self.set(7, Register::PHL),
                    0xFF => self.set(7, Register::A),
                }
            }
            0xCC => {
                let b1 = self.memory.get_address(self.pc);
                self.pc = self.pc.wrapping_add(1);
                let b2 = self.memory.get_address(self.pc);
                let addr = u16::from_ne_bytes([b2, b1]);
                self.callc(Condition::Z, addr);
            }
            0xCD => {
                let b1 = self.memory.get_address(self.pc);
                self.pc = self.pc.wrapping_add(1);
                let b2 = self.memory.get_address(self.pc);
                let addr = u16::from_ne_bytes([b2, b1]);
                self.call(addr);
            }
            // TODO
            // Unimplemented ADC
            // 0xCE => {
            //     let b = self.memory.get_address(self.pc);
            //     self.pc = self.pc.wrapping_add(1);
            //     self.adc(Register::A, b);
            // }
            0xCF => self.rst(),
            0xD0 => self.retc(Condition::NC),
            0xD1 => self.pop(Register::DE),
            0xD2 => {
                let b1 = self.memory.get_address(self.pc);
                self.pc = self.pc.wrapping_add(1);
                let b2 = self.memory.get_address(self.pc);
                let addr = u16::from_ne_bytes([b2, b1]);
                self.jpc(Condition::NZ, addr);
            }
            0xD4 => {
                let b1 = self.memory.get_address(self.pc);
                self.pc = self.pc.wrapping_add(1);
                let b2 = self.memory.get_address(self.pc);
                let addr = u16::from_ne_bytes([b2, b1]);
                self.jp(addr);
            }
            0xD5 => self.push(Register::DE),
            // TODO
            // Unimplemented SUB
            // 0xD6 => {
            //     let b = self.memory.get_address(self.pc);
            //     self.pc = self.pc.wrapping_add(1);
            //     self.sub_u8(Register::A, b);
            // }
            0xD7 => self.rst(),
            0xD8 => self.retc(Condition::C),
            0xD9 => self.reti(),
            0xDA => {
                let b1 = self.memory.get_address(self.pc);
                self.pc = self.pc.wrapping_add(1);
                let b2 = self.memory.get_address(self.pc);
                let addr = u16::from_ne_bytes([b2, b1]);
                self.jpc(Condition::C, addr);
            }
            0xDC => {
                let b1 = self.memory.get_address(self.pc);
                self.pc = self.pc.wrapping_add(1);
                let b2 = self.memory.get_address(self.pc);
                let addr = u16::from_ne_bytes([b2, b1]);
                self.callc(Condition::C, addr);
            }
            // TODO
            // Unimplemented SBC
            // 0xDE => {
            //     let b = self.memory.get_address(self.pc);
            //     self.pc = self.pc.wrapping_add(1);
            //     self.sbc(Condition::C, addr);
            // }
            0xDF => self.rst(),
            0xE0 => {
                let b = self.memory.get_address(self.pc);
                self.pc = self.pc.wrapping_add(1);
                self.memory
                    .write_byte(0xFF00 + b as u16, self.get_regu8(Register::A))
            }
            0xE1 => self.pop(Register::HL),
            0xE2 => self.memory.write_byte(
                0xFF00 + self.get_regu8(Register::C) as u16,
                self.get_regu8(Register::A),
            ),
            0xE5 => self.push(Register::HL),
            // TODO
            // Unimplemented AND
            // 0xE6 => {
            //     let b = self.memory.get_address(self.pc);
            //     self.pc = self.pc.wrapping_add(1);
            //     self.and(b);
            // }
            0xE7 => self.rst(),
            // TODO
            // 0xE8 => {
            //     self.add_u16(Register::SP, self.memory.get_address(self.pc) as i16);
            //     self.pc = self.pc.wrapping_add(1);
            // }
            0xE9 => self.jp(self.get_regu16(Register::HL)),
            0xEA => {
                let b1 = self.memory.get_address(self.pc);
                self.pc = self.pc.wrapping_add(1);
                let b2 = self.memory.get_address(self.pc);
                self.pc = self.pc.wrapping_add(1);
                let addr = u16::from_ne_bytes([b2, b1]);
                self.memory.write_byte(addr, self.get_regu8(Register::A));
            }
            0xEE => {
                let b = self.memory.get_address(self.pc);
                self.pc = self.pc.wrapping_add(1);
                self.xor(b);
            }
            // TODO
            0xEF => self.rst(),
            0xF0 => {
                let b = self.memory.get_address(self.pc);
                self.pc = self.pc.wrapping_add(1);
                self.ld_regu8(Register::A, self.memory.get_address(0xFF00 + b as u16))
            }
            0xF1 => self.pop(Register::AF),
            0xF2 => {
                let b = self
                    .memory
                    .get_address(0xFF00 + self.get_regu8(Register::C) as u16);
                self.ld_regu8(Register::A, b)
            }
            0xF3 => self.di(),
            0xF5 => self.push(Register::AF),
            0xF6 => {
                let b = self.memory.get_address(self.pc);
                self.pc = self.pc.wrapping_add(1);
                self.or(b);
            }
            // TODO
            0xF7 => self.rst(),
            0xF8 => {
                let b = self.memory.get_address(self.pc) as i8;
                self.pc = self.pc.wrapping_add(1);

                // https://stackoverflow.com/a/53455823
                let sp = self.get_regu16(Register::SP).wrapping_add(b as u16);
                self.ld_regu16(Register::HL, sp)
            }
            0xF9 => self.ld_regu16(Register::SP, self.get_regu16(Register::HL)),
            0xFA => {
                let b1 = self.memory.get_address(self.pc);
                self.pc = self.pc.wrapping_add(1);
                let b2 = self.memory.get_address(self.pc);
                self.pc = self.pc.wrapping_add(1);
                let addr = u16::from_ne_bytes([b2, b1]);
                self.ld_regu8(Register::A, self.memory.get_address(addr));
            }
            0xFB => self.ei(),
            0xFE => {
                let b = self.memory.get_address(self.pc);
                self.pc = self.pc.wrapping_add(1);
                self.cp(b);
            }
            // TODO
            0xFF => self.rst(),

            _ => unimplemented!("Unhandled opcode {:#x}", opcode),
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
            PHL | PBC | PDE => self.memory.get_address(self.get_regu16(reg)),
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
            PHL | PBC | PDE => self.memory.get_address(self.get_regu16(reg)),
            _ => unreachable!("Attempted to get u16 from set_regu8"),
        };
        *reg = val;
    }
    fn get_regu16(&self, reg: Register) -> u16 {
        use Register::*;
        match reg {
            AF | PAF => self.af.into(),
            BC | PBC => self.bc.into(),
            DE | PDE => self.de.into(),
            HL | PHL => self.hl.into(),
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
            SP => self.sp,
            PC => self.pc,
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

        let zero = match reg {
            A => {
                let res = self.af.a().wrapping_add(1);
                self.af.set_a(res);
                res == 0
            }
            B => {
                let res = self.bc[0].wrapping_add(1);
                self.bc[0] = res;
                res == 0
            }
            C => {
                let res = self.bc[1].wrapping_add(1);
                self.bc[1] = res;
                res == 0
            }
            D => {
                let res = self.de[0].wrapping_add(1);
                self.de[0] = res;
                res == 0
            }
            E => {
                let res = self.de[1].wrapping_add(1);
                self.de[1] = res;
                res == 0
            }
            H => {
                let res = self.hl[0].wrapping_add(1);
                self.hl[0] = res;
                res == 0
            }
            L => {
                let res = self.hl[1].wrapping_add(1);
                self.hl[1] = res;
                res == 0
            }
            BC => {
                let res = u16::from(self.bc).wrapping_add(1);
                self.bc = GPReg::from(res);
                res == 0
            }
            DE => {
                let res = u16::from(self.de).wrapping_add(1);
                self.de = GPReg::from(res);
                res == 0
            }
            HL => {
                let res = u16::from(self.hl).wrapping_add(1);
                self.hl = GPReg::from(res);
                res == 0
            }
            PBC => {
                let res = self.memory.get_address(self.bc.into()).wrapping_add(1);
                self.memory.write_byte(self.bc.into(), res);
                res == 0
            }
            PDE => {
                let res = self.memory.get_address(self.de.into()).wrapping_add(1);
                self.memory.write_byte(self.de.into(), res);
                res == 0
            }
            PHL => {
                let res = self.memory.get_address(self.hl.into()).wrapping_add(1);
                self.memory.write_byte(self.hl.into(), res);
                res == 0
            }
            _ => unimplemented!(),
        };

        self.af.set_z(zero);
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

        let ref mut reg = match reg {
            AF => self.af.into_bytes(),
            BC => self.bc.0,
            DE => self.de.0,
            HL => self.hl.0,
            _ => unreachable!("Attempted to pop single byte register onto stack"),
        };

        reg[1] = self.memory.get_address(self.sp);
        self.sp = self.sp.wrapping_add(1);
        reg[0] = self.memory.get_address(self.sp);
        self.sp = self.sp.wrapping_add(1);
    }

    fn add_u8(&mut self, reg: Register, n: u8) {
        let (val, over) = self.get_regu8(reg).overflowing_add(n);
        self.set_regu8(reg, val);
        self.af.set_z(over);
        unimplemented!("Rest of flags")
    }

    fn add_u16(&mut self, reg: Register, n: u16) {
        let (val, over) = self.get_regu16(reg).overflowing_add(n);
        self.set_regu16(reg, val);
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

    fn daa(&mut self) {
        unimplemented!()
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

    fn rlc(&mut self, reg: Register) {
        unimplemented!()
    }

    fn rl(&mut self, reg: Register) {
        unimplemented!()
    }

    fn rrc(&mut self, reg: Register) {
        unimplemented!()
    }

    fn rr(&mut self, reg: Register) {
        unimplemented!()
    }

    fn sla(&mut self, reg: Register) {
        unimplemented!()
    }

    fn sra(&mut self, reg: Register) {
        unimplemented!()
    }

    fn srl(&mut self, reg: Register) {
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
            _ => self.memory.get_address(self.get_regu16(reg)),
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
            A => self.af.set_a(self.af.a() | 1 << bit),
            B => self.bc[0] |= val,
            C => self.bc[1] |= val,
            D => self.de[0] |= val,
            E => self.de[1] |= val,
            H => self.hl[0] |= val,
            L => self.hl[1] |= val,
            _ => {
                let b = self.memory.get_address(self.get_regu16(reg));
                self.memory.write_byte(self.get_regu16(reg), b | val);
            }
        };
    }

    fn res(&mut self, bit: u8, reg: Register) {
        assert!(bit <= 7);
        let val = !(1 << bit);

        use Register::*;
        match reg {
            A => self.af.set_a(self.af.a() & val),
            B => self.bc[0] &= val,
            C => self.bc[1] &= val,
            D => self.de[0] &= val,
            E => self.de[1] &= val,
            H => self.hl[0] &= val,
            L => self.hl[1] &= val,
            _ => {
                let b = self.memory.get_address(self.get_regu16(reg));
                self.memory.write_byte(self.get_regu16(reg), b & val);
            }
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

    fn jrc(&mut self, cond: Condition, n: i8) {
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
        self.pc = self.pc.wrapping_add(self.pc);
        let b2 = self.memory.get_address(self.pc);
        self.pc = self.pc.wrapping_add(self.pc);

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
            self.pc = self.pc.wrapping_add(self.pc);
            let b2 = self.memory.get_address(self.pc);
            self.pc = self.pc.wrapping_add(self.pc);

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
        self.pc = self.pc.wrapping_add(self.pc);
        let b2 = self.memory.get_address(self.pc);
        self.pc = self.pc.wrapping_add(self.pc);

        // Push PC to stack
        self.pc = u16::from_ne_bytes([b1, b2]);
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
            self.pc = self.pc.wrapping_add(self.pc);
            let b2 = self.memory.get_address(self.pc);
            self.pc = self.pc.wrapping_add(self.pc);

            // Push PC to stack
            self.pc = u16::from_ne_bytes([b2, b1]);
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

impl GPReg {
    fn wrapping_add(&self, n: u16) -> Self {
        let x: u16 = u16::from(*self).wrapping_add(n);
        GPReg::from(x)
    }
    fn wrapping_sub(&self, n: u16) -> Self {
        let x: u16 = u16::from(*self).wrapping_sub(n);
        GPReg::from(x)
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
}
