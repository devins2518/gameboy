use crate::cpu::Cpu;

pub enum Condition {
    NZ,
    Z,
    NC,
    C,
}

impl Condition {
    pub fn check(&self, cpu: &Cpu) -> bool {
        match self {
            Condition::NZ => cpu.af.z() == false,
            Condition::Z => cpu.af.z() == true,
            Condition::NC => cpu.af.c() == false,
            Condition::C => cpu.af.c() == true,
        }
    }
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

pub enum InterruptStatus {
    Unset,
    PendingDisable,
    StartDisable,
    PendingEnable,
    StartEnable,
    Set,
}

#[derive(Clone, Debug)]
pub enum Event {
    Clock,
}
