use log::{debug, trace};
use modular_bitfield::prelude::*;
use std::ops::{Deref, DerefMut};

use crate::memory::Bus;
use crate::utils::{Condition, InterruptStatus, Register};

pub struct Cpu {
    pub af: AFReg,
    bc: GPReg,
    de: GPReg,
    hl: GPReg,
    sp: u16,
    pc: u16,
    memory: Bus,
    interrupt_enable: InterruptStatus,

    finished_boot: bool,

    pub clocks: u32,
}

impl Cpu {
    pub fn new(bus: Bus) -> Self {
        Cpu {
            af: 0.into(),
            bc: 0.into(),
            de: 0.into(),
            hl: 0.into(),
            sp: 0x0000,
            pc: 0x0000,
            memory: bus,
            interrupt_enable: InterruptStatus::Unset,
            finished_boot: false,
            clocks: 0,
        }
    }

    fn next_instruction(&mut self) -> u8 {
        debug!(
            "Finished boot: {}, PC: {:#06X}",
            self.finished_boot, self.pc
        );

        if self.pc == 0xFF {
            self.finished_boot = true;
        }

        let opcode = if self.finished_boot {
            self.memory.get_address(self.pc)
        } else {
            self.memory.get_bootrom(self.pc)
        };

        self.pc = self.pc.wrapping_add(1);
        self.clocks += 1;
        opcode
    }

    fn imm_u8(&mut self) -> u8 {
        let opcode = self.next_instruction();
        self.clocks += 1;
        opcode
    }

    fn imm_u16(&mut self) -> u16 {
        let b1 = self.next_instruction();
        self.clocks += 1;
        let b2 = self.next_instruction();
        self.clocks += 1;
        u16::from_ne_bytes([b1, b2])
    }

    fn isr(&mut self, int_addr: u16) {
        self.interrupt_enable = InterruptStatus::Unset;
        self.clocks += 2; // 2 noops
        self.push(Register::PC);
        self.clocks += 2; // Push PC
        self.pc = int_addr;
        self.clocks += 1; // Set PC
    }

    pub fn clock(&mut self) -> i64 {
        let old_clocks = self.clocks;

        // Handle interrupts
        if let InterruptStatus::Set = self.interrupt_enable {
            let inte = self.memory.get_address(0xFFFF);
            let intf = self.memory.get_address(0xFF0F) & inte; // we only care about interrupts which are actually enabled

            if intf & 0x1 == 0x1 {
                self.isr(0x40);
            }
            if intf & 0x2 == 0x2 {
                self.isr(0x48);
            }
            if intf & 0x4 == 0x4 {
                self.isr(0x50);
            }
            if intf & 0x8 == 0x8 {
                self.isr(0x58);
            }
            if intf & 0x10 == 0x10 {
                self.isr(0x60);
            }
        }

        let opcode = self.next_instruction();
        debug!("Matching opcode: {:#X}", opcode);

        match opcode {
            0x00 => {
                self.nop();

                trace!("{:#04X} NOP", opcode)
            }
            0x01 => {
                let byte = self.imm_u16();
                self.ld_regu16(Register::BC, byte);

                trace!("{:#04X} LD BC, {:#06X}", opcode, byte);
            }
            0x02 => {
                self.memory.write_byte(self.bc.into(), self.af.a());

                trace!("{:#04X} LD (BC), A", opcode);
            }
            0x03 => {
                self.inc(Register::BC);

                trace!("{:#04X} INC BC", opcode);
            }
            0x04 => {
                self.inc(Register::B);

                trace!("{:#04X} INC B", opcode);
            }
            0x05 => {
                self.dec(Register::B);

                trace!("{:#04X} DEC B", opcode);
            }
            0x06 => {
                let b = self.imm_u8();
                self.ld_regu8(Register::B, b);

                trace!("{:#04X} LD B, {:#04X}", opcode, b);
            }
            0x07 => {
                self.rlca();

                trace!("{:#04X} RLCA", opcode);
            }
            0x08 => {
                let b = self.imm_u16();
                let sp = self.sp.to_le_bytes();
                self.memory.write_byte(b, sp[0]);
                self.memory.write_byte(b.wrapping_add(1), sp[1]);

                trace!("{:#04X} LD {:#06X}, SP", opcode, b);
            }
            0x09 => {
                self.add_u16(Register::HL, self.get_regu16(Register::BC));

                trace!("{:#04X} ADD HL, BC", opcode);
            }
            0x0A => {
                let b = self.memory.get_address(self.get_regu16(Register::BC));
                self.ld_regu8(Register::A, b);

                trace!("{:#04X} LD A, (BC)", opcode);
            }
            0x0B => {
                self.dec(Register::BC);

                trace!("{:#04X} DEC BC", opcode);
            }
            0x0C => {
                self.inc(Register::C);

                trace!("{:#04X} INC C", opcode);
            }
            0x0D => {
                self.dec(Register::C);

                trace!("{:#04X} DEC C", opcode);
            }
            0x0E => {
                let b = self.imm_u8();
                self.ld_regu8(Register::C, b);

                trace!("{:#04X} LD C, {:#04X}", opcode, b);
            }
            0x0F => {
                self.rrca();

                trace!("{:#04X} RRCA", opcode);
            }
            0x10 => {
                self.stop();

                trace!("{:#04X} STOP", opcode);
            }
            0x11 => {
                let b = self.imm_u16();
                self.ld_regu16(Register::DE, b);

                trace!("{:#04X} LD DE, {:#06X}", opcode, b);
            }
            0x12 => {
                self.memory.write_byte(self.de.into(), self.af.a());

                trace!("{:#04X} LD (DE), A", opcode);
            }
            0x13 => {
                self.inc(Register::DE);

                trace!("{:#04X} INC DE", opcode);
            }
            0x14 => {
                self.inc(Register::D);

                trace!("{:#04X} INC D", opcode);
            }
            0x15 => {
                self.dec(Register::D);

                trace!("{:#04X} DEC D", opcode);
            }
            0x16 => {
                let b = self.imm_u8();
                self.ld_regu8(Register::D, b);

                trace!("{:#04X} LD D, {:#04X}", opcode, b);
            }
            0x17 => {
                self.rla();

                trace!("{:#04X} RLA", opcode);
            }
            0x18 => {
                let b = self.imm_u8();
                self.jr(b as i8);

                trace!("{:#04X} JR {:#04X}", opcode, b);
            }
            0x19 => {
                self.add_u16(Register::HL, self.get_regu16(Register::DE));

                trace!("{:#04X} ADD HL, DE", opcode);
            }
            0x1A => {
                let b = self.memory.get_address(self.get_regu16(Register::DE));
                self.ld_regu8(Register::A, b);

                trace!("{:#04X} LD A, (DE)", opcode);
            }
            0x1B => {
                self.dec(Register::DE);

                trace!("{:#04X} DEC DE", opcode);
            }
            0x1C => {
                self.inc(Register::E);

                trace!("{:#04X} INC E", opcode);
            }
            0x1D => {
                self.dec(Register::E);

                trace!("{:#04X} DEC E", opcode);
            }
            0x1E => {
                let b = self.imm_u8();
                self.ld_regu8(Register::E, b);

                trace!("{:#04X} LD E {:#06X}", opcode, b);
            }
            0x1F => {
                self.rra();

                trace!("{:#04X} RRA", opcode);
            }
            0x20 => {
                let b = self.imm_u8();
                self.jrc(Condition::NZ, b as i8);

                trace!("{:#04X} JR NZ {:#06X}", opcode, b);
            }
            0x21 => {
                let b = self.imm_u16();
                self.ld_regu16(Register::HL, b);

                trace!("{:#04X} LD HL {:#06X}", opcode, b);
            }
            0x22 => {
                self.memory
                    .write_byte(self.get_regu16(Register::HL), self.get_regu8(Register::A));
                self.hl = GPReg::from(self.get_regu16(Register::HL).wrapping_add(1));

                trace!("{:#04X} LD (HL+), A", opcode);
            }
            0x23 => {
                self.inc(Register::HL);

                trace!("{:#04X} INC HL", opcode);
            }
            0x24 => {
                self.inc(Register::H);

                trace!("{:#04X} INC H", opcode);
            }
            0x25 => {
                self.dec(Register::H);

                trace!("{:#04X} DEC H", opcode);
            }
            0x26 => {
                let b = self.imm_u8();
                self.ld_regu8(Register::H, b);

                trace!("{:#04X} LD H, {:#04X}", opcode, b);
            }
            0x27 => {
                self.daa();

                trace!("{:#04X} DAA", opcode);
            }
            0x28 => {
                let b = self.imm_u8();
                self.jrc(Condition::Z, b as i8);

                trace!("{:#04X} JR Z {:#04X}", opcode, b);
            }
            0x29 => self.add_u16(Register::HL, self.get_regu16(Register::HL)),
            0x2A => {
                let b = self.memory.get_address(self.get_regu16(Register::HL));
                self.ld_regu8(Register::A, b);

                self.hl = GPReg::from(self.get_regu16(Register::HL).wrapping_add(1));
            }
            0x2B => {
                // dec(sp)
                unimplemented!()
            }
            0x2C => self.inc(Register::L),
            0x2D => self.dec(Register::L),
            0x2E => {
                let b = self.imm_u8();
                self.ld_regu8(Register::L, b);
            }
            0x2F => self.cpl(),
            0x30 => {
                let b = self.imm_u8();
                self.jrc(Condition::NC, b as i8);

                trace!("{:#04X} JR NC {:#06X}", opcode, b);
            }
            0x31 => {
                let b = self.imm_u16();
                self.ld_regu16(Register::SP, b);

                trace!("{:#04X} LD {:?} {:#06X}", opcode, Register::SP, b);
            }
            0x32 => {
                self.memory
                    .write_byte(self.get_regu16(Register::HL), self.get_regu8(Register::A));
                self.hl = GPReg::from(self.get_regu16(Register::HL).wrapping_sub(1));

                trace!("{:#04X} LD (HL-) A", opcode);
            }
            0x33 => {
                self.inc(Register::SP);

                trace!("{:#04X} INC SP", opcode);
            }
            0x34 => {
                self.inc(Register::PHL);

                trace!("{:#04X} INC (HL)", opcode);
            }
            0x35 => {
                self.dec(Register::PHL);

                trace!("{:#04X} DEC (HL)", opcode);
            }
            0x36 => {
                let b = self.imm_u8();
                self.ld_regu8(Register::PHL, b);
            }
            0x37 => self.scf(),
            0x38 => {
                let b = self.imm_u8();
                self.jrc(Condition::C, b as i8);
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
                let b = self.imm_u8();
                self.ld_regu8(Register::A, b);

                trace!("{:#04X} LD A, {:#04X}", opcode, b);
            }
            0x3F => self.ccf(),
            0x40 => {
                self.ld_regu8(Register::B, self.get_regu8(Register::B));

                trace!("{:#04X} LD {:?} {:?}", opcode, Register::B, Register::B);
            }
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
            0x88 => self.adc(self.get_regu8(Register::B)),
            0x89 => self.adc(self.get_regu8(Register::C)),
            0x8A => self.adc(self.get_regu8(Register::D)),
            0x8B => self.adc(self.get_regu8(Register::E)),
            0x8C => self.adc(self.get_regu8(Register::H)),
            0x8D => self.adc(self.get_regu8(Register::L)),
            0x8E => self.adc(self.get_regu8(Register::PHL)),
            0x8F => self.adc(self.get_regu8(Register::A)),
            0x90 => self.sub(self.get_regu8(Register::B)),
            0x91 => self.sub(self.get_regu8(Register::C)),
            0x92 => self.sub(self.get_regu8(Register::D)),
            0x93 => self.sub(self.get_regu8(Register::E)),
            0x94 => self.sub(self.get_regu8(Register::H)),
            0x95 => self.sub(self.get_regu8(Register::L)),
            0x96 => self.sub(self.get_regu8(Register::PHL)),
            0x97 => self.sub(self.get_regu8(Register::A)),
            0x98 => {
                self.sbc(self.get_regu8(Register::B));
            }
            0x99 => {
                self.sbc(self.get_regu8(Register::C));
            }
            0x9A => {
                self.sbc(self.get_regu8(Register::D));
            }
            0x9B => {
                self.sbc(self.get_regu8(Register::E));
            }
            0x9C => {
                self.sbc(self.get_regu8(Register::H));
            }
            0x9D => {
                self.sbc(self.get_regu8(Register::L));
            }
            0x9E => {
                self.sbc(self.get_regu8(Register::PHL));
            }
            0x9F => {
                self.sbc(self.get_regu8(Register::A));
            }
            0xA0 => {
                self.and(self.get_regu8(Register::B));
            }
            0xA1 => {
                self.and(self.get_regu8(Register::C));
            }
            0xA2 => {
                self.and(self.get_regu8(Register::D));
            }
            0xA3 => {
                self.and(self.get_regu8(Register::E));
            }
            0xA4 => {
                self.and(self.get_regu8(Register::H));
            }
            0xA5 => {
                self.and(self.get_regu8(Register::L));
            }
            0xA6 => {
                self.and(self.get_regu8(Register::PHL));
            }
            0xA7 => {
                self.and(self.get_regu8(Register::A));
            }
            0xA8 => {
                self.xor(self.get_regu8(Register::B));
                trace!("{:#04X} XOR B", opcode);
            }
            0xA9 => {
                self.xor(self.get_regu8(Register::C));
                trace!("{:#04X} XOR C", opcode);
            }
            0xAA => {
                self.xor(self.get_regu8(Register::D));
                trace!("{:#04X} XOR D", opcode);
            }
            0xAB => {
                self.xor(self.get_regu8(Register::E));
                trace!("{:#04X} XOR E", opcode);
            }
            0xAC => {
                self.xor(self.get_regu8(Register::H));
                trace!("{:#04X} XOR H", opcode);
            }
            0xAD => {
                self.xor(self.get_regu8(Register::L));
                trace!("{:#04X} XOR L", opcode);
            }
            0xAE => {
                self.xor(self.get_regu8(Register::PHL));
                trace!("{:#04X} XOR (HL)", opcode,);
            }
            0xAF => {
                self.xor(self.get_regu8(Register::A));
                trace!("{:#04X} XOR A", opcode);
            }
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
                let b = self.imm_u16();
                self.jpc(Condition::NZ, b);
            }
            0xC3 => {
                let b = self.imm_u16();
                self.jp(b);
            }
            0xC4 => {
                let b = self.imm_u16();
                self.callc(Condition::NZ, b);
            }
            0xC5 => self.push(Register::BC),
            0xC6 => {
                let b = self.imm_u8();
                self.add_u8(Register::A, b);
            }
            0xC7 => self.rst(0x0000),
            0xC8 => self.retc(Condition::C),
            0xC9 => self.retc(Condition::Z),
            0xCA => self.ret(),
            0xCB => {
                let opcode = self.next_instruction();
                match opcode {
                    0x00 => {
                        self.rlc(Register::B);
                        trace!("0xCB{:X} RLC B", opcode);
                    }
                    0x01 => {
                        self.rlc(Register::C);
                        trace!("0xCB{:X} RLC C", opcode);
                    }
                    0x02 => {
                        self.rlc(Register::D);
                        trace!("0xCB{:X} RLC D", opcode);
                    }
                    0x03 => {
                        self.rlc(Register::E);
                        trace!("0xCB{:X} RLC E", opcode);
                    }
                    0x04 => {
                        self.rlc(Register::H);
                        trace!("0xCB{:X} RLC H", opcode);
                    }
                    0x05 => {
                        self.rlc(Register::L);
                        trace!("0xCB{:X} RLC L", opcode);
                    }
                    0x06 => {
                        self.rlc(Register::PHL);
                        trace!("0xCB{:X} RLC (HL)", opcode);
                    }
                    0x07 => {
                        self.rlc(Register::A);
                        trace!("0xCB{:X} RLC A", opcode);
                    }
                    0x08 => {
                        self.rrc(Register::B);
                        trace!("0xCB{:X} RRC B", opcode);
                    }
                    0x09 => {
                        self.rrc(Register::C);
                        trace!("0xCB{:X} RRC C", opcode);
                    }
                    0x0A => {
                        self.rrc(Register::D);
                        trace!("0xCB{:X} RRC D", opcode);
                    }
                    0x0B => {
                        self.rrc(Register::E);
                        trace!("0xCB{:X} RRC E", opcode);
                    }
                    0x0C => {
                        self.rrc(Register::H);
                        trace!("0xCB{:X} RRC H", opcode);
                    }
                    0x0D => {
                        self.rrc(Register::L);
                        trace!("0xCB{:X} RRC L", opcode);
                    }
                    0x0E => {
                        self.rrc(Register::PHL);
                        trace!("0xCB{:X} RRC (HL)", opcode);
                    }
                    0x0F => {
                        self.rrc(Register::A);
                        trace!("0xCB{:X} RRC A", opcode);
                    }
                    0x10 => {
                        self.rl(Register::B);
                        trace!("0xCB{:X} RL B", opcode);
                    }
                    0x11 => {
                        self.rl(Register::C);
                        trace!("0xCB{:X} RL C", opcode);
                    }
                    0x12 => {
                        self.rl(Register::D);
                        trace!("0xCB{:X} RL D", opcode);
                    }
                    0x13 => {
                        self.rl(Register::E);
                        trace!("0xCB{:X} RL E", opcode);
                    }
                    0x14 => {
                        self.rl(Register::H);
                        trace!("0xCB{:X} RL H", opcode);
                    }
                    0x15 => {
                        self.rl(Register::L);
                        trace!("0xCB{:X} RL L", opcode);
                    }
                    0x16 => {
                        self.rl(Register::PHL);
                        trace!("0xCB{:X} RL (HL)", opcode);
                    }
                    0x17 => {
                        self.rl(Register::A);
                        trace!("0xCB{:X} RL A", opcode);
                    }
                    0x18 => {
                        self.rr(Register::B);
                        trace!("0xCB{:X} RR B", opcode);
                    }
                    0x19 => {
                        self.rr(Register::C);
                        trace!("0xCB{:X} RR C", opcode);
                    }
                    0x1A => {
                        self.rr(Register::D);
                        trace!("0xCB{:X} RR D", opcode);
                    }
                    0x1B => {
                        self.rr(Register::E);
                        trace!("0xCB{:X} RR E", opcode);
                    }
                    0x1C => {
                        self.rr(Register::H);
                        trace!("0xCB{:X} RR H", opcode);
                    }
                    0x1D => {
                        self.rr(Register::L);
                        trace!("0xCB{:X} RR L", opcode);
                    }
                    0x1E => {
                        self.rr(Register::PHL);
                        trace!("0xCB{:X} RR (HL)", opcode);
                    }
                    0x1F => {
                        self.rr(Register::A);
                        trace!("0xCB{:X} RR A", opcode);
                    }
                    0x20 => {
                        self.sla(Register::B);
                        trace!("0xCB{:X} SLA B", opcode);
                    }
                    0x21 => {
                        self.sla(Register::C);
                        trace!("0xCB{:X} SLA C", opcode);
                    }
                    0x22 => {
                        self.sla(Register::D);
                        trace!("0xCB{:X} SLA D", opcode);
                    }
                    0x23 => {
                        self.sla(Register::E);
                        trace!("0xCB{:X} SLA E", opcode);
                    }
                    0x24 => {
                        self.sla(Register::H);
                        trace!("0xCB{:X} SLA H", opcode);
                    }
                    0x25 => {
                        self.sla(Register::L);
                        trace!("0xCB{:X} SLA L", opcode);
                    }
                    0x26 => {
                        self.sla(Register::PHL);
                        trace!("0xCB{:X} SLA (HL)", opcode);
                    }
                    0x27 => {
                        self.sla(Register::A);
                        trace!("0xCB{:X} SLA A", opcode);
                    }
                    0x28 => {
                        self.sra(Register::B);
                        trace!("0xCB{:X} SRA B", opcode);
                    }
                    0x29 => {
                        self.sra(Register::C);
                        trace!("0xCB{:X} SRA C", opcode);
                    }
                    0x2A => {
                        self.sra(Register::D);
                        trace!("0xCB{:X} SRA D", opcode);
                    }
                    0x2B => {
                        self.sra(Register::E);
                        trace!("0xCB{:X} SRA E", opcode);
                    }
                    0x2C => {
                        self.sra(Register::H);
                        trace!("0xCB{:X} SRA H", opcode);
                    }
                    0x2D => {
                        self.sra(Register::L);
                        trace!("0xCB{:X} SRA L", opcode);
                    }
                    0x2E => {
                        self.sra(Register::PHL);
                        trace!("0xCB{:X} SRA (HL)", opcode);
                    }
                    0x2F => {
                        self.sra(Register::A);
                        trace!("0xCB{:X} SRA A", opcode);
                    }
                    0x30 => {
                        self.swap(Register::B);
                        trace!("0xCB{:X} SWAP B", opcode);
                    }
                    0x31 => {
                        self.swap(Register::C);
                        trace!("0xCB{:X} SWAP C", opcode);
                    }
                    0x32 => {
                        self.swap(Register::D);
                        trace!("0xCB{:X} SWAP D", opcode);
                    }
                    0x33 => {
                        self.swap(Register::E);
                        trace!("0xCB{:X} SWAP E", opcode);
                    }
                    0x34 => {
                        self.swap(Register::H);
                        trace!("0xCB{:X} SWAP H", opcode);
                    }
                    0x35 => {
                        self.swap(Register::L);
                        trace!("0xCB{:X} SWAP L", opcode);
                    }
                    0x36 => {
                        self.swap(Register::PHL);
                        trace!("0xCB{:X} SWAP (HL)", opcode);
                    }
                    0x37 => {
                        self.swap(Register::A);
                        trace!("0xCB{:X} SWAP A", opcode);
                    }
                    0x38 => {
                        self.srl(Register::B);
                        trace!("0xCB{:X} SRL B", opcode);
                    }
                    0x39 => {
                        self.srl(Register::C);
                        trace!("0xCB{:X} SRL C", opcode);
                    }
                    0x3A => {
                        self.srl(Register::D);
                        trace!("0xCB{:X} SRL D", opcode);
                    }
                    0x3B => {
                        self.srl(Register::E);
                        trace!("0xCB{:X} SRL E", opcode);
                    }
                    0x3C => {
                        self.srl(Register::H);
                        trace!("0xCB{:X} SRL H", opcode);
                    }
                    0x3D => {
                        self.srl(Register::L);
                        trace!("0xCB{:X} SRL L", opcode);
                    }
                    0x3E => {
                        self.srl(Register::PHL);
                        trace!("0xCB{:X} SRL (HL)", opcode);
                    }
                    0x3F => {
                        self.srl(Register::A);
                        trace!("0xCB{:X} SRL A", opcode);
                    }
                    0x40 => {
                        self.bit(3, Register::B);
                        trace!("0xCB{:X} BIT 0, B", opcode);
                    }
                    0x41 => {
                        self.bit(3, Register::C);
                        trace!("0xCB{:X} BIT 0, C", opcode);
                    }
                    0x42 => {
                        self.bit(3, Register::D);
                        trace!("0xCB{:X} BIT 0, D", opcode);
                    }
                    0x43 => {
                        self.bit(3, Register::E);
                        trace!("0xCB{:X} BIT 0, E", opcode);
                    }
                    0x44 => {
                        self.bit(3, Register::H);
                        trace!("0xCB{:X} BIT 0, H", opcode);
                    }
                    0x45 => {
                        self.bit(3, Register::L);
                        trace!("0xCB{:X} BIT 0, L", opcode);
                    }
                    0x46 => {
                        self.bit(3, Register::PHL);
                        trace!("0xCB{:X} BIT 0, (HL)", opcode);
                    }
                    0x47 => {
                        self.bit(3, Register::A);
                        trace!("0xCB{:X} BIT 0, A", opcode);
                    }
                    0x48 => {
                        self.bit(3, Register::B);
                        trace!("0xCB{:X} BIT 1, B", opcode);
                    }
                    0x49 => {
                        self.bit(3, Register::C);
                        trace!("0xCB{:X} BIT 1, C", opcode);
                    }
                    0x4A => {
                        self.bit(3, Register::D);
                        trace!("0xCB{:X} BIT 1, D", opcode);
                    }
                    0x4B => {
                        self.bit(3, Register::E);
                        trace!("0xCB{:X} BIT 1, E", opcode);
                    }
                    0x4C => {
                        self.bit(3, Register::H);
                        trace!("0xCB{:X} BIT 1, H", opcode);
                    }
                    0x4D => {
                        self.bit(3, Register::L);
                        trace!("0xCB{:X} BIT 1, L", opcode);
                    }
                    0x4E => {
                        self.bit(3, Register::PHL);
                        trace!("0xCB{:X} BIT 1, (HL)", opcode);
                    }
                    0x4F => {
                        self.bit(3, Register::A);
                        trace!("0xCB{:X} BIT 1, A", opcode);
                    }
                    0x50 => {
                        self.bit(3, Register::B);
                        trace!("0xCB{:X} BIT 2, B", opcode);
                    }
                    0x51 => {
                        self.bit(3, Register::C);
                        trace!("0xCB{:X} BIT 2, C", opcode);
                    }
                    0x52 => {
                        self.bit(3, Register::D);
                        trace!("0xCB{:X} BIT 2, D", opcode);
                    }
                    0x53 => {
                        self.bit(3, Register::E);
                        trace!("0xCB{:X} BIT 2, E", opcode);
                    }
                    0x54 => {
                        self.bit(3, Register::H);
                        trace!("0xCB{:X} BIT 2, H", opcode);
                    }
                    0x55 => {
                        self.bit(3, Register::L);
                        trace!("0xCB{:X} BIT 2, L", opcode);
                    }
                    0x56 => {
                        self.bit(3, Register::PHL);
                        trace!("0xCB{:X} BIT 2, (HL)", opcode);
                    }
                    0x57 => {
                        self.bit(3, Register::A);
                        trace!("0xCB{:X} BIT 2, A", opcode);
                    }
                    0x58 => {
                        self.bit(3, Register::B);
                        trace!("0xCB{:X} BIT 3, B", opcode);
                    }
                    0x59 => {
                        self.bit(3, Register::C);
                        trace!("0xCB{:X} BIT 3, C", opcode);
                    }
                    0x5A => {
                        self.bit(3, Register::D);
                        trace!("0xCB{:X} BIT 3, D", opcode);
                    }
                    0x5B => {
                        self.bit(3, Register::E);
                        trace!("0xCB{:X} BIT 3, E", opcode);
                    }
                    0x5C => {
                        self.bit(3, Register::H);
                        trace!("0xCB{:X} BIT 3, H", opcode);
                    }
                    0x5D => {
                        self.bit(3, Register::L);
                        trace!("0xCB{:X} BIT 3, L", opcode);
                    }
                    0x5E => {
                        self.bit(3, Register::PHL);
                        trace!("0xCB{:X} BIT 3, (HL)", opcode);
                    }
                    0x5F => {
                        self.bit(3, Register::A);
                        trace!("0xCB{:X} BIT 3, A", opcode);
                    }
                    0x60 => {
                        self.bit(3, Register::B);
                        trace!("0xCB{:X} BIT 4, B", opcode);
                    }
                    0x61 => {
                        self.bit(3, Register::C);
                        trace!("0xCB{:X} BIT 4, C", opcode);
                    }
                    0x62 => {
                        self.bit(3, Register::D);
                        trace!("0xCB{:X} BIT 4, D", opcode);
                    }
                    0x63 => {
                        self.bit(3, Register::E);
                        trace!("0xCB{:X} BIT 4, E", opcode);
                    }
                    0x64 => {
                        self.bit(3, Register::H);
                        trace!("0xCB{:X} BIT 4, H", opcode);
                    }
                    0x65 => {
                        self.bit(3, Register::L);
                        trace!("0xCB{:X} BIT 4, L", opcode);
                    }
                    0x66 => {
                        self.bit(3, Register::PHL);
                        trace!("0xCB{:X} BIT 4, (HL)", opcode);
                    }
                    0x67 => {
                        self.bit(3, Register::A);
                        trace!("0xCB{:X} BIT 4, A", opcode);
                    }
                    0x68 => {
                        self.bit(3, Register::B);
                        trace!("0xCB{:X} BIT 5, B", opcode);
                    }
                    0x69 => {
                        self.bit(3, Register::C);
                        trace!("0xCB{:X} BIT 5, C", opcode);
                    }
                    0x6A => {
                        self.bit(3, Register::D);
                        trace!("0xCB{:X} BIT 5, D", opcode);
                    }
                    0x6B => {
                        self.bit(3, Register::E);
                        trace!("0xCB{:X} BIT 5, E", opcode);
                    }
                    0x6C => {
                        self.bit(3, Register::H);
                        trace!("0xCB{:X} BIT 5, H", opcode);
                    }
                    0x6D => {
                        self.bit(3, Register::L);
                        trace!("0xCB{:X} BIT 5, L", opcode);
                    }
                    0x6E => {
                        self.bit(3, Register::PHL);
                        trace!("0xCB{:X} BIT 5, (HL)", opcode);
                    }
                    0x6F => {
                        self.bit(3, Register::A);
                        trace!("0xCB{:X} BIT 5, A", opcode);
                    }
                    0x70 => {
                        self.bit(3, Register::B);
                        trace!("0xCB{:X} BIT 6, B", opcode);
                    }
                    0x71 => {
                        self.bit(3, Register::C);
                        trace!("0xCB{:X} BIT 6, C", opcode);
                    }
                    0x72 => {
                        self.bit(3, Register::D);
                        trace!("0xCB{:X} BIT 6, D", opcode);
                    }
                    0x73 => {
                        self.bit(3, Register::E);
                        trace!("0xCB{:X} BIT 6, E", opcode);
                    }
                    0x74 => {
                        self.bit(3, Register::H);
                        trace!("0xCB{:X} BIT 6, H", opcode);
                    }
                    0x75 => {
                        self.bit(3, Register::L);
                        trace!("0xCB{:X} BIT 6, L", opcode);
                    }
                    0x76 => {
                        self.bit(3, Register::PHL);
                        trace!("0xCB{:X} BIT 6, (HL)", opcode);
                    }
                    0x77 => {
                        self.bit(3, Register::A);
                        trace!("0xCB{:X} BIT 6, A", opcode);
                    }
                    0x78 => {
                        self.bit(3, Register::B);
                        trace!("0xCB{:X} BIT 7, B", opcode);
                    }
                    0x79 => {
                        self.bit(3, Register::C);
                        trace!("0xCB{:X} BIT 7, C", opcode);
                    }
                    0x7A => {
                        self.bit(3, Register::D);
                        trace!("0xCB{:X} BIT 7, D", opcode);
                    }
                    0x7B => {
                        self.bit(3, Register::E);
                        trace!("0xCB{:X} BIT 7, E", opcode);
                    }
                    0x7C => {
                        self.bit(3, Register::H);
                        trace!("0xCB{:X} BIT 7, H", opcode);
                    }
                    0x7D => {
                        self.bit(3, Register::L);
                        trace!("0xCB{:X} BIT 7, L", opcode);
                    }
                    0x7E => {
                        self.bit(3, Register::PHL);
                        trace!("0xCB{:X} BIT 7, (HL)", opcode);
                    }
                    0x7F => {
                        self.bit(3, Register::A);
                        trace!("0xCB{:X} BIT 7, A", opcode);
                    }
                    0x80 => {
                        self.res(3, Register::B);
                        trace!("0xCB{:X} RES 0, B", opcode);
                    }
                    0x81 => {
                        self.res(3, Register::C);
                        trace!("0xCB{:X} RES 0, C", opcode);
                    }
                    0x82 => {
                        self.res(3, Register::D);
                        trace!("0xCB{:X} RES 0, D", opcode);
                    }
                    0x83 => {
                        self.res(3, Register::E);
                        trace!("0xCB{:X} RES 0, E", opcode);
                    }
                    0x84 => {
                        self.res(3, Register::H);
                        trace!("0xCB{:X} RES 0, H", opcode);
                    }
                    0x85 => {
                        self.res(3, Register::L);
                        trace!("0xCB{:X} RES 0, L", opcode);
                    }
                    0x86 => {
                        self.res(3, Register::PHL);
                        trace!("0xCB{:X} RES 0, (HL)", opcode);
                    }
                    0x87 => {
                        self.res(3, Register::A);
                        trace!("0xCB{:X} RES 0, A", opcode);
                    }
                    0x88 => {
                        self.res(3, Register::B);
                        trace!("0xCB{:X} RES 1, B", opcode);
                    }
                    0x89 => {
                        self.res(3, Register::C);
                        trace!("0xCB{:X} RES 1, C", opcode);
                    }
                    0x8A => {
                        self.res(3, Register::D);
                        trace!("0xCB{:X} RES 1, D", opcode);
                    }
                    0x8B => {
                        self.res(3, Register::E);
                        trace!("0xCB{:X} RES 1, E", opcode);
                    }
                    0x8C => {
                        self.res(3, Register::H);
                        trace!("0xCB{:X} RES 1, H", opcode);
                    }
                    0x8D => {
                        self.res(3, Register::L);
                        trace!("0xCB{:X} RES 1, L", opcode);
                    }
                    0x8E => {
                        self.res(3, Register::PHL);
                        trace!("0xCB{:X} RES 1, (HL)", opcode);
                    }
                    0x8F => {
                        self.res(3, Register::A);
                        trace!("0xCB{:X} RES 1, A", opcode);
                    }
                    0x90 => {
                        self.res(3, Register::B);
                        trace!("0xCB{:X} RES 2, B", opcode);
                    }
                    0x91 => {
                        self.res(3, Register::C);
                        trace!("0xCB{:X} RES 2, C", opcode);
                    }
                    0x92 => {
                        self.res(3, Register::D);
                        trace!("0xCB{:X} RES 2, D", opcode);
                    }
                    0x93 => {
                        self.res(3, Register::E);
                        trace!("0xCB{:X} RES 2, E", opcode);
                    }
                    0x94 => {
                        self.res(3, Register::H);
                        trace!("0xCB{:X} RES 2, H", opcode);
                    }
                    0x95 => {
                        self.res(3, Register::L);
                        trace!("0xCB{:X} RES 2, L", opcode);
                    }
                    0x96 => {
                        self.res(3, Register::PHL);
                        trace!("0xCB{:X} RES 2, (HL)", opcode);
                    }
                    0x97 => {
                        self.res(3, Register::A);
                        trace!("0xCB{:X} RES 2, A", opcode);
                    }
                    0x98 => {
                        self.res(3, Register::B);
                        trace!("0xCB{:X} RES 3, B", opcode);
                    }
                    0x99 => {
                        self.res(3, Register::C);
                        trace!("0xCB{:X} RES 3, C", opcode);
                    }
                    0x9A => {
                        self.res(3, Register::D);
                        trace!("0xCB{:X} RES 3, D", opcode);
                    }
                    0x9B => {
                        self.res(3, Register::E);
                        trace!("0xCB{:X} RES 3, E", opcode);
                    }
                    0x9C => {
                        self.res(3, Register::H);
                        trace!("0xCB{:X} RES 3, H", opcode);
                    }
                    0x9D => {
                        self.res(3, Register::L);
                        trace!("0xCB{:X} RES 3, L", opcode);
                    }
                    0x9E => {
                        self.res(3, Register::PHL);
                        trace!("0xCB{:X} RES 3, (HL)", opcode);
                    }
                    0x9F => {
                        self.res(3, Register::A);
                        trace!("0xCB{:X} RES 3, A", opcode);
                    }
                    0xA0 => {
                        self.res(3, Register::B);
                        trace!("0xCB{:X} RES 4, B", opcode);
                    }
                    0xA1 => {
                        self.res(3, Register::C);
                        trace!("0xCB{:X} RES 4, C", opcode);
                    }
                    0xA2 => {
                        self.res(3, Register::D);
                        trace!("0xCB{:X} RES 4, D", opcode);
                    }
                    0xA3 => {
                        self.res(3, Register::E);
                        trace!("0xCB{:X} RES 4, E", opcode);
                    }
                    0xA4 => {
                        self.res(3, Register::H);
                        trace!("0xCB{:X} RES 4, H", opcode);
                    }
                    0xA5 => {
                        self.res(3, Register::L);
                        trace!("0xCB{:X} RES 4, L", opcode);
                    }
                    0xA6 => {
                        self.res(3, Register::PHL);
                        trace!("0xCB{:X} RES 4, (HL)", opcode);
                    }
                    0xA7 => {
                        self.res(3, Register::A);
                        trace!("0xCB{:X} RES 4, A", opcode);
                    }
                    0xA8 => {
                        self.res(3, Register::B);
                        trace!("0xCB{:X} RES 5, B", opcode);
                    }
                    0xA9 => {
                        self.res(3, Register::C);
                        trace!("0xCB{:X} RES 5, C", opcode);
                    }
                    0xAA => {
                        self.res(3, Register::D);
                        trace!("0xCB{:X} RES 5, D", opcode);
                    }
                    0xAB => {
                        self.res(3, Register::E);
                        trace!("0xCB{:X} RES 5, E", opcode);
                    }
                    0xAC => {
                        self.res(3, Register::H);
                        trace!("0xCB{:X} RES 5, H", opcode);
                    }
                    0xAD => {
                        self.res(3, Register::L);
                        trace!("0xCB{:X} RES 5, L", opcode);
                    }
                    0xAE => {
                        self.res(3, Register::PHL);
                        trace!("0xCB{:X} RES 5, (HL)", opcode);
                    }
                    0xAF => {
                        self.res(3, Register::A);
                        trace!("0xCB{:X} RES 5, A", opcode);
                    }
                    0xB0 => {
                        self.res(3, Register::B);
                        trace!("0xCB{:X} RES 6, B", opcode);
                    }
                    0xB1 => {
                        self.res(3, Register::C);
                        trace!("0xCB{:X} RES 6, C", opcode);
                    }
                    0xB2 => {
                        self.res(3, Register::D);
                        trace!("0xCB{:X} RES 6, D", opcode);
                    }
                    0xB3 => {
                        self.res(3, Register::E);
                        trace!("0xCB{:X} RES 6, E", opcode);
                    }
                    0xB4 => {
                        self.res(3, Register::H);
                        trace!("0xCB{:X} RES 6, H", opcode);
                    }
                    0xB5 => {
                        self.res(3, Register::L);
                        trace!("0xCB{:X} RES 6, L", opcode);
                    }
                    0xB6 => {
                        self.res(3, Register::PHL);
                        trace!("0xCB{:X} RES 6, (HL)", opcode);
                    }
                    0xB7 => {
                        self.res(3, Register::A);
                        trace!("0xCB{:X} RES 6, A", opcode);
                    }
                    0xB8 => {
                        self.res(3, Register::B);
                        trace!("0xCB{:X} RES 7, B", opcode);
                    }
                    0xB9 => {
                        self.res(3, Register::C);
                        trace!("0xCB{:X} RES 7, C", opcode);
                    }
                    0xBA => {
                        self.res(3, Register::D);
                        trace!("0xCB{:X} RES 7, D", opcode);
                    }
                    0xBB => {
                        self.res(3, Register::E);
                        trace!("0xCB{:X} RES 7, E", opcode);
                    }
                    0xBC => {
                        self.res(3, Register::H);
                        trace!("0xCB{:X} RES 7, H", opcode);
                    }
                    0xBD => {
                        self.res(3, Register::L);
                        trace!("0xCB{:X} RES 7, L", opcode);
                    }
                    0xBE => {
                        self.res(3, Register::PHL);
                        trace!("0xCB{:X} RES 7, (HL)", opcode);
                    }
                    0xBF => {
                        self.res(3, Register::A);
                        trace!("0xCB{:X} RES 7, A", opcode);
                    }
                    0xC0 => {
                        self.set(3, Register::B);
                        trace!("0xCB{:X} SET 0, B", opcode);
                    }
                    0xC1 => {
                        self.set(3, Register::C);
                        trace!("0xCB{:X} SET 0, C", opcode);
                    }
                    0xC2 => {
                        self.set(3, Register::D);
                        trace!("0xCB{:X} SET 0, D", opcode);
                    }
                    0xC3 => {
                        self.set(3, Register::E);
                        trace!("0xCB{:X} SET 0, E", opcode);
                    }
                    0xC4 => {
                        self.set(3, Register::H);
                        trace!("0xCB{:X} SET 0, H", opcode);
                    }
                    0xC5 => {
                        self.set(3, Register::L);
                        trace!("0xCB{:X} SET 0, L", opcode);
                    }
                    0xC6 => {
                        self.set(3, Register::PHL);
                        trace!("0xCB{:X} SET 0, (HL)", opcode);
                    }
                    0xC7 => {
                        self.set(3, Register::A);
                        trace!("0xCB{:X} SET 0, A", opcode);
                    }
                    0xC8 => {
                        self.set(3, Register::B);
                        trace!("0xCB{:X} SET 1, B", opcode);
                    }
                    0xC9 => {
                        self.set(3, Register::C);
                        trace!("0xCB{:X} SET 1, C", opcode);
                    }
                    0xCA => {
                        self.set(3, Register::D);
                        trace!("0xCB{:X} SET 1, D", opcode);
                    }
                    0xCB => {
                        self.set(3, Register::E);
                        trace!("0xCB{:X} SET 1, E", opcode);
                    }
                    0xCC => {
                        self.set(3, Register::H);
                        trace!("0xCB{:X} SET 1, H", opcode);
                    }
                    0xCD => {
                        self.set(3, Register::L);
                        trace!("0xCB{:X} SET 1, L", opcode);
                    }
                    0xCE => {
                        self.set(3, Register::PHL);
                        trace!("0xCB{:X} SET 1, (HL)", opcode);
                    }
                    0xCF => {
                        self.set(3, Register::A);
                        trace!("0xCB{:X} SET 1, A", opcode);
                    }
                    0xD0 => {
                        self.set(3, Register::B);
                        trace!("0xCB{:X} SET 2, B", opcode);
                    }
                    0xD1 => {
                        self.set(3, Register::C);
                        trace!("0xCB{:X} SET 2, C", opcode);
                    }
                    0xD2 => {
                        self.set(3, Register::D);
                        trace!("0xCB{:X} SET 2, D", opcode);
                    }
                    0xD3 => {
                        self.set(3, Register::E);
                        trace!("0xCB{:X} SET 2, E", opcode);
                    }
                    0xD4 => {
                        self.set(3, Register::H);
                        trace!("0xCB{:X} SET 2, H", opcode);
                    }
                    0xD5 => {
                        self.set(3, Register::L);
                        trace!("0xCB{:X} SET 2, L", opcode);
                    }
                    0xD6 => {
                        self.set(3, Register::PHL);
                        trace!("0xCB{:X} SET 2, (HL)", opcode);
                    }
                    0xD7 => {
                        self.set(3, Register::A);
                        trace!("0xCB{:X} SET 2, A", opcode);
                    }
                    0xD8 => {
                        self.set(3, Register::B);
                        trace!("0xCB{:X} SET 3, B", opcode);
                    }
                    0xD9 => {
                        self.set(3, Register::C);
                        trace!("0xCB{:X} SET 3, C", opcode);
                    }
                    0xDA => {
                        self.set(3, Register::D);
                        trace!("0xCB{:X} SET 3, D", opcode);
                    }
                    0xDB => {
                        self.set(3, Register::E);
                        trace!("0xCB{:X} SET 3, E", opcode);
                    }
                    0xDC => {
                        self.set(3, Register::H);
                        trace!("0xCB{:X} SET 3, H", opcode);
                    }
                    0xDD => {
                        self.set(3, Register::L);
                        trace!("0xCB{:X} SET 3, L", opcode);
                    }
                    0xDE => {
                        self.set(3, Register::PHL);
                        trace!("0xCB{:X} SET 3, (HL)", opcode);
                    }
                    0xDF => {
                        self.set(3, Register::A);
                        trace!("0xCB{:X} SET 3, A", opcode);
                    }
                    0xE0 => {
                        self.set(3, Register::B);
                        trace!("0xCB{:X} SET 4, B", opcode);
                    }
                    0xE1 => {
                        self.set(3, Register::C);
                        trace!("0xCB{:X} SET 4, C", opcode);
                    }
                    0xE2 => {
                        self.set(3, Register::D);
                        trace!("0xCB{:X} SET 4, D", opcode);
                    }
                    0xE3 => {
                        self.set(3, Register::E);
                        trace!("0xCB{:X} SET 4, E", opcode);
                    }
                    0xE4 => {
                        self.set(3, Register::H);
                        trace!("0xCB{:X} SET 4, H", opcode);
                    }
                    0xE5 => {
                        self.set(3, Register::L);
                        trace!("0xCB{:X} SET 4, L", opcode);
                    }
                    0xE6 => {
                        self.set(3, Register::PHL);
                        trace!("0xCB{:X} SET 4, (HL)", opcode);
                    }
                    0xE7 => {
                        self.set(3, Register::A);
                        trace!("0xCB{:X} SET 4, A", opcode);
                    }
                    0xE8 => {
                        self.set(3, Register::B);
                        trace!("0xCB{:X} SET 5, B", opcode);
                    }
                    0xE9 => {
                        self.set(3, Register::C);
                        trace!("0xCB{:X} SET 5, C", opcode);
                    }
                    0xEA => {
                        self.set(3, Register::D);
                        trace!("0xCB{:X} SET 5, D", opcode);
                    }
                    0xEB => {
                        self.set(3, Register::E);
                        trace!("0xCB{:X} SET 5, E", opcode);
                    }
                    0xEC => {
                        self.set(3, Register::H);
                        trace!("0xCB{:X} SET 5, H", opcode);
                    }
                    0xED => {
                        self.set(3, Register::L);
                        trace!("0xCB{:X} SET 5, L", opcode);
                    }
                    0xEE => {
                        self.set(3, Register::PHL);
                        trace!("0xCB{:X} SET 5, (HL)", opcode);
                    }
                    0xEF => {
                        self.set(3, Register::A);
                        trace!("0xCB{:X} SET 5, A", opcode);
                    }
                    0xF0 => {
                        self.set(3, Register::B);
                        trace!("0xCB{:X} SET 6, B", opcode);
                    }
                    0xF1 => {
                        self.set(3, Register::C);
                        trace!("0xCB{:X} SET 6, C", opcode);
                    }
                    0xF2 => {
                        self.set(3, Register::D);
                        trace!("0xCB{:X} SET 6, D", opcode);
                    }
                    0xF3 => {
                        self.set(3, Register::E);
                        trace!("0xCB{:X} SET 6, E", opcode);
                    }
                    0xF4 => {
                        self.set(3, Register::H);
                        trace!("0xCB{:X} SET 6, H", opcode);
                    }
                    0xF5 => {
                        self.set(3, Register::L);
                        trace!("0xCB{:X} SET 6, L", opcode);
                    }
                    0xF6 => {
                        self.set(3, Register::PHL);
                        trace!("0xCB{:X} SET 6, (HL)", opcode);
                    }
                    0xF7 => {
                        self.set(3, Register::A);
                        trace!("0xCB{:X} SET 6, A", opcode);
                    }
                    0xF8 => {
                        self.set(3, Register::B);
                        trace!("0xCB{:X} SET 7, B", opcode);
                    }
                    0xF9 => {
                        self.set(3, Register::C);
                        trace!("0xCB{:X} SET 7, C", opcode);
                    }
                    0xFA => {
                        self.set(3, Register::D);
                        trace!("0xCB{:X} SET 7, D", opcode);
                    }
                    0xFB => {
                        self.set(3, Register::E);
                        trace!("0xCB{:X} SET 7, E", opcode);
                    }
                    0xFC => {
                        self.set(3, Register::H);
                        trace!("0xCB{:X} SET 7, H", opcode);
                    }
                    0xFD => {
                        self.set(3, Register::L);
                        trace!("0xCB{:X} SET 7, L", opcode);
                    }
                    0xFE => {
                        self.set(3, Register::PHL);
                        trace!("0xCB{:X} SET 7, (HL)", opcode);
                    }
                    0xFF => {
                        self.set(3, Register::A);
                        trace!("0xCB{:X} SET 7, A", opcode);
                    }
                }
            }
            0xCC => {
                let b = self.imm_u16();
                self.callc(Condition::Z, b);
            }
            0xCD => {
                let b = self.imm_u16();
                self.call(b);
            }
            0xCE => {
                let b = self.imm_u8();
                self.adc(b);
            }
            0xCF => self.rst(0x0008),
            0xD0 => self.retc(Condition::NC),
            0xD1 => self.pop(Register::DE),
            0xD2 => {
                let b = self.imm_u16();
                self.jpc(Condition::NZ, b);
            }
            0xD4 => {
                let b = self.imm_u16();
                self.jp(b);
            }
            0xD5 => self.push(Register::DE),
            0xD6 => {
                let b = self.imm_u8();
                self.sub(b);
            }
            0xD7 => self.rst(0x0010),
            0xD8 => self.retc(Condition::C),
            0xD9 => self.reti(),
            0xDA => {
                let b = self.imm_u16();
                self.jpc(Condition::C, b);
            }
            0xDC => {
                let b = self.imm_u16();
                self.callc(Condition::C, b);
            }
            // TODO
            // Unimplemented SBC
            // 0xDE => {
            //     let b = self.memory.get_address(self.pc);
            //     self.pc = self.pc.wrapping_add(1);
            //     self.sbc(Condition::C, addr);
            // }
            0xDF => self.rst(0x0018),
            0xE0 => {
                let b = self.imm_u8();
                self.memory
                    .write_byte(0xFF00 + b as u16, self.get_regu8(Register::A))
            }
            0xE1 => self.pop(Register::HL),
            0xE2 => {
                self.memory.write_byte(
                    0xFF00 + self.get_regu8(Register::C) as u16,
                    self.get_regu8(Register::A),
                );

                trace!("{:#04X} LD (0xFF00 + C), A", opcode);
            }
            0xE5 => self.push(Register::HL),
            0xE6 => {
                let b = self.imm_u8();
                self.and(b);
            }
            0xE7 => self.rst(0x0020),
            // TODO
            // 0xE8 => {
            //     self.add_u16(Register::SP, self.memory.get_address(self.pc) as i16);
            //     self.pc = self.pc.wrapping_add(1);
            // }
            0xE9 => self.jp(self.get_regu16(Register::HL)),
            0xEA => {
                let b = self.imm_u16();
                self.memory.write_byte(b, self.get_regu8(Register::A));
            }
            0xEE => {
                let b = self.imm_u8();
                self.xor(b);
            }
            // TODO
            0xEF => self.rst(0x0028),
            0xF0 => {
                let b = self.imm_u8();
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
                let b = self.imm_u8();
                self.or(b);
            }
            // TODO
            0xF7 => self.rst(0x0030),
            0xF8 => {
                let b = self.imm_u8() as i8;

                // https://stackoverflow.com/a/53455823
                let sp = self.get_regu16(Register::SP).wrapping_add(b as u16);
                self.ld_regu16(Register::HL, sp)
            }
            0xF9 => self.ld_regu16(Register::SP, self.get_regu16(Register::HL)),
            0xFA => {
                let b = self.imm_u16();
                self.ld_regu8(Register::A, self.memory.get_address(b));
            }
            0xFB => self.ei(),
            0xFE => {
                let b = self.imm_u8();
                self.cp(b);
            }
            // TODO
            0xFF => self.rst(0x0038),

            _ => unimplemented!("Unhandled opcode {:#x}", opcode),
        }

        match self.interrupt_enable {
            InterruptStatus::StartDisable => {
                self.interrupt_enable = InterruptStatus::PendingDisable
            }
            InterruptStatus::StartEnable => self.interrupt_enable = InterruptStatus::PendingEnable,
            InterruptStatus::PendingDisable => self.interrupt_enable = InterruptStatus::Unset,
            InterruptStatus::PendingEnable => self.interrupt_enable = InterruptStatus::Set,
            _ => {}
        }

        self.clocks as i64 - old_clocks as i64
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
            PC => self.pc,
            _ => unreachable!("Attempted to set regu16 into regu8"),
        }
    }
    fn set_regu16(&mut self, reg: Register, val: u16) {
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
        #[allow(non_upper_case_globals)]
        const b: u8 = 1;

        match reg {
            A => {
                let a = self.af.a();
                let res = a.wrapping_add(b);
                self.af.set_a(res);
                self.half_carry(a, b);
                self.af.set_z(res == 0);
                self.af.set_n(false);
            }
            B => {
                let a = self.bc[0];
                let res = a.wrapping_add(b);
                self.bc[0] = res;
                self.half_carry(a, b);
                self.af.set_z(res == 0);
                self.af.set_n(false);
            }
            C => {
                let a = self.bc[1];
                let res = a.wrapping_add(b);
                self.bc[1] = res;
                self.half_carry(a, b);
                self.af.set_z(res == 0);
                self.af.set_n(false);
            }
            D => {
                let a = self.de[0];
                let res = a.wrapping_add(b);
                self.de[0] = res;
                self.half_carry(a, b);
                self.af.set_z(res == 0);
                self.af.set_n(false);
            }
            E => {
                let a = self.de[1];
                let res = a.wrapping_add(b);
                self.de[1] = res;
                self.half_carry(a, b);
                self.af.set_z(res == 0);
                self.af.set_n(false);
            }
            H => {
                let a = self.hl[0];
                let res = a.wrapping_add(b);
                self.hl[0] = res;
                self.half_carry(a, b);
                self.af.set_z(res == 0);
                self.af.set_n(false);
            }
            L => {
                let a = self.hl[1];
                let res = a.wrapping_add(b);
                self.hl[1] = res;
                self.half_carry(a, b);
                self.af.set_z(res == 0);
                self.af.set_n(false);
            }
            BC => {
                let a = u16::from(self.bc);
                let res = a.wrapping_add(b as u16);
                self.bc = GPReg::from(res);
            }
            DE => {
                let res = u16::from(self.de).wrapping_add(b as u16);
                self.de = GPReg::from(res);
            }
            HL => {
                let res = u16::from(self.hl).wrapping_add(b as u16);
                self.hl = GPReg::from(res);
            }
            _ => unreachable!(),
        }
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
    }

    fn and(&mut self, val: u8) {
        let res = self.af.a() & val;

        self.af.set_a(res);
        self.af.set_z(res == 0);
        self.af.set_n(false);
        self.af.set_h(true);
        self.af.set_c(false);
    }

    fn push(&mut self, reg: Register) {
        let reg = self.get_regu16(reg);

        self.memory.write_byte(self.sp, (reg & 0x0F) as u8);
        self.sp = self.sp.wrapping_sub(1);
        self.memory.write_byte(self.sp, (reg & 0xF0) as u8);
        self.sp = self.sp.wrapping_sub(1);
    }

    fn pop(&mut self, reg: Register) {
        let b1 = self.memory.get_address(self.sp);
        self.sp = self.sp.wrapping_add(1);
        let b2 = self.memory.get_address(self.sp);
        self.sp = self.sp.wrapping_add(1);

        self.set_regu16(reg, u16::from_le_bytes([b2, b1]))
    }

    fn add_u8(&mut self, reg: Register, n: u8) {
        let prev = self.get_regu8(reg);
        let (val, over) = prev.overflowing_add(n);
        self.set_regu8(reg, val);
        self.af.set_z(val == 0);
        self.af.set_n(false);
        self.half_carry(prev, n);
        self.af.set_c(over);
    }

    fn add_u16(&mut self, reg: Register, n: u16) {
        let (val, over) = self.get_regu16(reg).overflowing_add(n);
        self.set_regu16(reg, val);
        self.af.set_z(over);
        unimplemented!("Rest of flags")
    }

    fn adc(&mut self, n: u8) {
        let prev = self.get_regu8(Register::A);
        let (val, over) = prev.overflowing_add(n.wrapping_add(self.af.c() as u8));
        self.set_regu8(Register::A, val);
        self.af.set_z(val == 0);
        self.af.set_n(false);
        self.half_carry(prev, n);
        self.af.set_c(over);
    }

    fn sub(&mut self, n: u8) {
        let prev = self.get_regu8(Register::A);
        let (val, over) = prev.overflowing_sub(n);
        self.set_regu8(Register::A, val);
        self.af.set_z(val == 0);
        self.af.set_n(true);
        // TODO > Set if no borrow from bit 4
        self.half_carry(prev, n);
        self.af.set_c(over);
    }

    fn sbc(&mut self, n: u8) {
        let prev = self.get_regu8(Register::A);
        let (val, over) = prev.overflowing_sub(n.wrapping_sub(!(self.af.c() as u8)));
        self.set_regu8(Register::A, val);
        self.af.set_z(val == 0);
        self.af.set_n(true);
        // TODO > Set if no borrow from bit 4
        self.half_carry(prev, n);
        self.af.set_c(over);
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
        let a = self.af.a();
        self.af.set_c(a < n);
        self.half_carry(a, n);
        self.af.set_z(a == n);
        self.af.set_n(true);
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
            PHL => {
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
        self.interrupt_enable = InterruptStatus::StartDisable;
    }

    fn ei(&mut self) {
        self.interrupt_enable = InterruptStatus::StartEnable;
    }

    fn rlca(&mut self) {
        let a = self.af.a();
        let x = a.rotate_left(1);
        self.af.set_a(x);

        self.af.set_z(x == 0);
        self.af.set_n(false);
        self.af.set_h(false);
        self.af.set_c(a & 0x80 == 1);
    }

    fn rla(&mut self) {
        let a = self.af.a();

        let result = a << 1 | (self.af.c() as u8);
        self.set_regu8(Register::A, result);

        self.af.set_z(result == 0);
        self.af.set_n(false);
        self.af.set_h(false);
        self.af.set_c((a & 0x80) >> 7 == 1);
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
        let prev = self.get_regu8(reg);

        let result = prev << 1 | (self.af.c() as u8);
        self.set_regu8(reg, result);

        self.af.set_z(result == 0);
        self.af.set_n(false);
        self.af.set_h(false);
        self.af.set_c((prev & 0x80) >> 7 == 1);
    }

    fn rrc(&mut self, reg: Register) {
        unimplemented!()
    }

    fn rr(&mut self, reg: Register) {
        let prev = self.get_regu8(reg);

        let result = (self.af.c() as u8) << 7 | prev >> 1;
        self.set_regu8(reg, result);

        self.af.set_z(result == 0);
        self.af.set_n(false);
        self.af.set_h(false);
        self.af.set_c(prev & 0x1 == 1);
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
        let val = self.get_regu8(reg);

        self.af.set_z((val >> bit) == 0);
        self.af.set_n(false);
        self.af.set_h(true);
    }

    fn set(&mut self, bit: u8, reg: Register) {
        let val = 1 << bit;

        self.set_regu8(reg, self.get_regu8(reg) | val);
    }

    fn res(&mut self, bit: u8, reg: Register) {
        let val = !(1 << bit);

        self.set_regu8(reg, self.get_regu8(reg) & val);
    }

    fn jp(&mut self, val: u16) {
        // TODO check
        self.pc = val;
    }

    fn jpc(&mut self, cond: Condition, val: u16) {
        if cond.check(&self) {
            self.pc = val;
        }
    }

    fn jr(&mut self, n: i8) {
        self.pc = (self.pc as i16).wrapping_add(n as i16) as u16;
    }

    fn jrc(&mut self, cond: Condition, n: i8) {
        if cond.check(&self) {
            self.pc = (self.pc as i16).wrapping_add(n as i16) as u16;
        }
    }

    fn call(&mut self, addr: u16) {
        // Push PC to stack
        self.push(Register::PC);

        self.pc = addr;
    }

    fn callc(&mut self, cond: Condition, addr: u16) {
        if cond.check(&self) {
            self.call(addr)
        }
    }

    fn rst(&mut self, addr: u16) {
        self.push(Register::PC);
        self.pc = addr;
    }

    fn ret(&mut self) {
        self.pop(Register::PC);
    }

    fn retc(&mut self, cond: Condition) {
        if cond.check(&self) {
            self.ret()
        }
    }

    fn reti(&mut self) {
        unimplemented!()
    }

    // https://robdor.com/2016/08/10/gameboy-emulator-half-carry-flag/
    fn half_carry(&mut self, a: u8, b: u8) {
        self.af.set_h((((a & 0xf) + (b & 0xf)) & 0x10) == 0x10)
    }

    fn carry(&mut self, a: u16, b: u16) {
        self.af.set_c((((a & 0xf0) + (b & 0xf0)) & 0x100) == 0x100)
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
pub struct AFReg {
    a: u8,
    #[skip]
    __: B4,
    pub c: bool,
    h: bool,
    n: bool,
    pub z: bool,
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
    use env_logger::Env;
    env_logger::Builder::from_env(Env::default().default_filter_or("info")).init();

    let bus = Bus::new(concat!(
        env!("CARGO_MANIFEST_DIR"),
        "/roms/blargg/cpu_instrs/cpu_instrs.gb"
    ));
    let mut cpu = Cpu::new(bus);
    cpu.af.set_z(true);
    assert_eq!(cpu.af.z(), true);
    cpu.af.set_z(false);
    assert_eq!(cpu.af.z(), false);
    cpu.af.set_z(true);
    assert_eq!(cpu.get_regu8(Register::F), 0b10000000);
    loop {
        cpu.clock();
    }
}
