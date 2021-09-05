pub enum Condition {
    NZ,
    Z,
    NC,
    C,
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
