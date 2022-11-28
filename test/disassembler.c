#include "src/decoder.h"
#include "src/instruction.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define TEST_OPCODES_SIZE 827
#define TEST_OPCODES_NUM 512

/* clang-format off */
const uint8_t TEST_OPCODES[TEST_OPCODES_SIZE] = {
                      /* MNEMONIC        | TIMING */
    0x00,             /* NOOP            |     1m */
    0x01, 0xFE, 0xCA, /* LD BC, u16      |     3m */
    0x02,             /* LD (BC), A      |     2m */
    0x03,             /* INC BC          |     2m */
    0x04,             /* INC B           |     1m */
    0x05,             /* DEC B           |     1m */
    0x06, 0xAA,       /* LD B, u8        |     2m */
    0x07,             /* RLCA            |     1m */
    0x08, 0xFE, 0xCA, /* LD (u16), SP    |     5m */
    0x09,             /* ADD HL, BC      |     2m */
    0x0A,             /* LD A, (BC)      |     2m */
    0x0B,             /* DEC BC          |     2m */
    0x0C,             /* INC C           |     1m */
    0x0D,             /* DEC C           |     1m */
    0x0E, 0xAA,       /* LD C, u8        |     2m */
    0x0F,             /* RRCA            |     1m */
    0x10,             /* STOP            |     1m */
    0x11, 0xFE, 0xCA, /* LD DE, u16      |     3m */
    0x12,             /* LD (DE), A      |     2m */
    0x13,             /* INC DE          |     2m */
    0x14,             /* INC D           |     1m */
    0x15,             /* DEC D           |     1m */
    0x16, 0xAA,       /* LD D, u8        |     2m */
    0x17,             /* RLA             |     1m */
    0x18, 0x00,       /* JR i8           |     3m */
    0x19,             /* ADD HL, DE      |     2m */
    0x1A,             /* LD A, (DE)      |     2m */
    0x1B,             /* DEC DE          |     2m */
    0x1C,             /* INC E           |     1m */
    0x1D,             /* DEC E           |     1m */
    0x1E, 0xAA,       /* LD E, u8        |     2m */
    0x1F,             /* RRA             |     1m */
    0x20, 0x00,       /* JR NZ, i8       |  2m-3m */
    0x21, 0xFE, 0xCA, /* LD HL, u16      |     3m */
    0x22,             /* LD (HL+), A     |     2m */
    0x23,             /* INC HL,         |     2m */
    0x24,             /* INC H           |     1m */
    0x25,             /* DEC H           |     1m */
    0x26, 0xAA,       /* LD H, u8        |     2m */
    0x27,             /* DAA             |     1m */
    0x28, 0x00,       /* JR Z, i8        |  2m-3m */
    0x29,             /* ADD  HL, HL     |     2m */
    0x2A,             /* LD A, (HL+)     |     2m */
    0x2B,             /* DEC HL          |     2m */
    0x2C,             /* INC L           |     1m */
    0x2D,             /* DEC L           |     1m */
    0x2E, 0xAA,       /* LD L, u8        |     2m */
    0x2F,             /* CPL             |     1m */
    0x30, 0x00,       /* JR NC, i8       |  2m-3m */
    0x31, 0xFE, 0xCA, /* LD SP, u16      |     3m */
    0x32,             /* LD (HL-), A     |     2m */
    0x33,             /* INC SP          |     2m */
    0x34,             /* INC (HL)        |     3m */
    0x35,             /* DEC (HL)        |     3m */
    0x36, 0xAA,       /* LD (HL), u8     |     3m */
    0x37,             /* SCF             |     1m */
    0x38, 0x00,       /* JR C, i8        |  2m-3m */
    0x39,             /* ADD HL, SP      |     2m */
    0x3A,             /* LD A, (HL-)     |     2m */
    0x3B,             /* DEC SP          |     2m */
    0x3C,             /* INC A           |     1m */
    0x3D,             /* DEC A           |     1m */
    0x3E, 0xAA,       /* LD A, u8        |     2m */
    0x3F,             /* CCF             |     1m */
    0x40,             /* LD B, B         |     1m */
    0x41,             /* LD B, C         |     1m */
    0x42,             /* LD B, D         |     1m */
    0x43,             /* LD B, E         |     1m */
    0x44,             /* LD B, H         |     1m */
    0x45,             /* LD B, L         |     1m */
    0x46,             /* LD B, (HL)      |     2m */
    0x47,             /* LD B, A         |     1m */
    0x48,             /* LD C, B         |     1m */
    0x49,             /* LD C, C         |     1m */
    0x4A,             /* LD C, D         |     1m */
    0x4B,             /* LD C, E         |     1m */
    0x4C,             /* LD C, H         |     1m */
    0x4D,             /* LD C, L         |     1m */
    0x4E,             /* LD C, (HL)      |     2m */
    0x4F,             /* LD C, A         |     1m */
    0x50,             /* LD D, B         |     1m */
    0x51,             /* LD D, C         |     1m */
    0x52,             /* LD D, D         |     1m */
    0x53,             /* LD D, E         |     1m */
    0x54,             /* LD D, H         |     1m */
    0x55,             /* LD D, L         |     1m */
    0x56,             /* LD D, (HL)      |     2m */
    0x57,             /* LD D, A         |     1m */
    0x58,             /* LD E, B         |     1m */
    0x59,             /* LD E, C         |     1m */
    0x5A,             /* LD E, D         |     1m */
    0x5B,             /* LD E, E         |     1m */
    0x5C,             /* LD E, H         |     1m */
    0x5D,             /* LD E, L         |     1m */
    0x5E,             /* LD E, (HL)      |     2m */
    0x5F,             /* LD E, A         |     1m */
    0x60,             /* LD H, B         |     1m */
    0x61,             /* LD H, C         |     1m */
    0x62,             /* LD H, D         |     1m */
    0x63,             /* LD H, E         |     1m */
    0x64,             /* LD H, H         |     1m */
    0x65,             /* LD H, L         |     1m */
    0x66,             /* LD H, (HL)      |     2m */
    0x67,             /* LD H, A         |     1m */
    0x68,             /* LD L, B         |     1m */
    0x69,             /* LD L, C         |     1m */
    0x6A,             /* LD L, D         |     1m */
    0x6B,             /* LD L, E         |     1m */
    0x6C,             /* LD L, H         |     1m */
    0x6D,             /* LD L, L         |     1m */
    0x6E,             /* LD L, (HL)      |     2m */
    0x6F,             /* LD L, A         |     1m */
    0x70,             /* LD (HL), B      |     2m */
    0x71,             /* LD (HL), C      |     2m */
    0x72,             /* LD (HL), D      |     2m */
    0x73,             /* LD (HL), E      |     2m */
    0x74,             /* LD (HL), H      |     2m */
    0x75,             /* LD (HL), L      |     2m */
    0x76,             /* HALT            |     1m */
    0x77,             /* LD (HL), A      |     2m */
    0x78,             /* LD A, B         |     1m */
    0x79,             /* LD A, C         |     1m */
    0x7A,             /* LD A, D         |     1m */
    0x7B,             /* LD A, E         |     1m */
    0x7C,             /* LD A, H         |     1m */
    0x7D,             /* LD A, L         |     1m */
    0x7E,             /* LD A, (HL)      |     2m */
    0x7F,             /* LD A, A         |     1m */
    0x80,             /* ADD A, B        |     1m */
    0x81,             /* ADD A, C        |     1m */
    0x82,             /* ADD A, D        |     1m */
    0x83,             /* ADD A, E        |     1m */
    0x84,             /* ADD A, H        |     1m */
    0x85,             /* ADD A, L        |     1m */
    0x86,             /* ADD A, (HL)     |     2m */
    0x87,             /* ADD A, A        |     1m */
    0x88,             /* ADC A, B        |     1m */
    0x89,             /* ADC A, C        |     1m */
    0x8A,             /* ADC A, D        |     1m */
    0x8B,             /* ADC A, E        |     1m */
    0x8C,             /* ADC A, H        |     1m */
    0x8D,             /* ADC A, L        |     1m */
    0x8E,             /* ADC A, (HL)     |     2m */
    0x8F,             /* ADC A, A        |     1m */
    0x90,             /* SUB A, B        |     1m */
    0x91,             /* SUB A, C        |     1m */
    0x92,             /* SUB A, D        |     1m */
    0x93,             /* SUB A, E        |     1m */
    0x94,             /* SUB A, H        |     1m */
    0x95,             /* SUB A, L        |     1m */
    0x96,             /* SUB A, (HL)     |     2m */
    0x97,             /* SUB A, A        |     1m */
    0x98,             /* SBC A, B        |     1m */
    0x99,             /* SBC A, C        |     1m */
    0x9A,             /* SBC A, D        |     1m */
    0x9B,             /* SBC A, E        |     1m */
    0x9C,             /* SBC A, H        |     1m */
    0x9D,             /* SBC A, L        |     1m */
    0x9E,             /* SBC A, (HL)     |     2m */
    0x9F,             /* SBC A, A        |     1m */
    0xA0,             /* AND A, B        |     1m */
    0xA1,             /* AND A, C        |     1m */
    0xA2,             /* AND A, D        |     1m */
    0xA3,             /* AND A, E        |     1m */
    0xA4,             /* AND A, H        |     1m */
    0xA5,             /* AND A, L        |     1m */
    0xA6,             /* AND A, (HL)     |     2m */
    0xA7,             /* AND A, A        |     1m */
    0xA8,             /* XOR A, B        |     1m */
    0xA9,             /* XOR A, C        |     1m */
    0xAA,             /* XOR A, D        |     1m */
    0xAB,             /* XOR A, E        |     1m */
    0xAC,             /* XOR A, H        |     1m */
    0xAD,             /* XOR A, L        |     1m */
    0xAE,             /* XOR A, (HL)     |     2m */
    0xAF,             /* XOR A, A        |     1m */
    0xB0,             /* OR A, B         |     1m */
    0xB1,             /* OR A, C         |     1m */
    0xB2,             /* OR A, D         |     1m */
    0xB3,             /* OR A, E         |     1m */
    0xB4,             /* OR A, H         |     1m */
    0xB5,             /* OR A, L         |     1m */
    0xB6,             /* OR A, (HL)      |     2m */
    0xB7,             /* OR A, A         |     1m */
    0xB8,             /* CP A, B         |     1m */
    0xB9,             /* CP A, C         |     1m */
    0xBA,             /* CP A, D         |     1m */
    0xBB,             /* CP A, E         |     1m */
    0xBC,             /* CP A, H         |     1m */
    0xBD,             /* CP A, L         |     1m */
    0xBE,             /* CP A, (HL)      |     2m */
    0xBF,             /* CP A, A         |     1m */
    0xC0,             /* RET NZ          |  2m-5m */
    0xC1,             /* POP BC          |     3m */
    0xC2, 0xFE, 0xCA, /* JP NZ, u16      |  3m-4m */
    0xC3, 0xFE, 0xCA, /* JP u16          |     4m */
    0xC4, 0xFE, 0xCA, /* CALL NZ, u16    |  3m-6m */
    0xC5,             /* PUSH BC         |     4m */
    0xC6, 0xAA,       /* ADD A, u8       |     2m */
    0xC7,             /* RST 00h         |     4m */
    0xC8,             /* RET Z           |  2m-5m */
    0xC9,             /* RET             |     4m */
    0xCA, 0xFE, 0xCA, /* JP Z, u16       |  3m-4m */
    0xCC, 0xFE, 0xCA, /* CALL Z, u16     |  3m-6m */
    0xCD, 0xFE, 0xCA, /* CALL u16        |     6m */
    0xCE, 0xAA,       /* ADC A, u8       |     2m */
    0xCF,             /* RST 08h         |     4m */
    0xD0,             /* RET NC          |  2m-5m */
    0xD1,             /* POP DE          |     3m */
    0xD2, 0xFE, 0xCA, /* JP NC. u16      |  3m-4m */
    0xD3,             /* ILLEGAL         |     1m */
    0xD4, 0xFE, 0xCA, /* CALL NC, u16    |  3m-6m */
    0xD5,             /* PUSH DE         |     4m */
    0xD6, 0xAA,       /* SUB A, u8       |     2m */
    0xD7,             /* RST 10h         |     4m */
    0xD8,             /* RET C           |  2m-5m */
    0xD9,             /* RETI            |     4m */
    0xDA, 0xFE, 0xCA, /* JP C, u16       |  3m-4m */
    0xDB,             /* ILLEGAL         |     1m */
    0xDC, 0xFE, 0xCA, /* CALL C, u16     |  3m-6m */
    0xDD,             /* ILLEGAL         |     1m */
    0xDE, 0xAA,       /* SBC A, u8       |     2m */
    0xDF,             /* RST 18h         |     4m */
    0xE0, 0xAA,       /* LD (FF00+u8), A |     3m */
    0xE1,             /* POP HL          |     3m */
    0xE2,             /* LD (FF00+C), A  |     2m */
    0xE3,             /* ILLEGAL         |     1m */
    0xE4,             /* ILLEGAL         |     1m */
    0xE5,             /* PUSH HL         |     4m */
    0xE6, 0xAA,       /* AND A, u8       |     2m */
    0xE7,             /* RST 20h         |     4m */
    0xE8, 0xAA,       /* ADD SP, i8      |     4m */
    0xE9,             /* JP HL           |     1m */
    0xEA, 0xFE, 0xCA, /* LD (u16), A     |     4m */
    0xEB,             /* ILLEGAL         |     1m */
    0xEC,             /* ILLEGAL         |     1m */
    0xED,             /* ILLEGAL         |     1m */
    0xEE, 0xAA,       /* XOR A, u8       |     2m */
    0xEF,             /* RST 28h         |     4m */
    0xF0, 0xAA,       /* LD A, (FF00+u8) |     3m */
    0xF1,             /* POP AF          |     3m */
    0xF2,             /* LD A, (FF00+C)  |     2m */
    0xF3,             /* DI              |     1m */
    0xF4,             /* ILLEGAL         |     1m */
    0xF5,             /* PUSH AF         |     4m */
    0xF6, 0xAA,       /* OR A, u8        |     2m */
    0xF7,             /* RST 30h         |     4m */
    0xF8, 0xAA,       /* LD HL, SP+i8    |     3m */
    0xF9,             /* LD SP, HL       |     2m */
    0xFA, 0xFE, 0xCA, /* LD A, (u16)     |     4m */
    0xFB,             /* EI              |     1m */
    0xFC,             /* ILLEGAL         |     1m */
    0xFD,             /* ILLEGAL         |     1m */
    0xFE, 0xAA,       /* CP A, u8        |     2m */
    0xFF,             /* RST 38h         |     4m */
    0xCB, 0x00,       /* RLC B           |     2m */
    0xCB, 0x01,       /* RLC C           |     2m */
    0xCB, 0x02,       /* RLC D           |     2m */
    0xCB, 0x03,       /* RLC E           |     2m */
    0xCB, 0x04,       /* RLC H           |     2m */
    0xCB, 0x05,       /* RLC L           |     2m */
    0xCB, 0x06,       /* RLC (HL)        |     4m */
    0xCB, 0x07,       /* RLC A           |     2m */
    0xCB, 0x08,       /* RRC B           |     2m */
    0xCB, 0x09,       /* RRC C           |     2m */
    0xCB, 0x0A,       /* RRC D           |     2m */
    0xCB, 0x0B,       /* RRC E           |     2m */
    0xCB, 0x0C,       /* RRC H           |     2m */
    0xCB, 0x0D,       /* RRC L           |     2m */
    0xCB, 0x0E,       /* RRC (HL)        |     4m */
    0xCB, 0x0F,       /* RRC A           |     2m */
    0xCB, 0x10,       /* RL B            |     2m */
    0xCB, 0x11,       /* RL C            |     2m */
    0xCB, 0x12,       /* RL D            |     2m */
    0xCB, 0x13,       /* RL E            |     2m */
    0xCB, 0x14,       /* RL H            |     2m */
    0xCB, 0x15,       /* RL L            |     2m */
    0xCB, 0x16,       /* RL (HL)         |     4m */
    0xCB, 0x17,       /* RL A            |     2m */
    0xCB, 0x18,       /* RL B            |     2m */
    0xCB, 0x19,       /* RR C            |     2m */
    0xCB, 0x1A,       /* RR D            |     2m */
    0xCB, 0x1B,       /* RR E            |     2m */
    0xCB, 0x1C,       /* RR H            |     2m */
    0xCB, 0x1D,       /* RR L            |     2m */
    0xCB, 0x1E,       /* RR (HL)         |     4m */
    0xCB, 0x1F,       /* RR A            |     2m */
    0xCB, 0x20,       /* SLA B           |     2m */
    0xCB, 0x21,       /* SLA C           |     2m */
    0xCB, 0x22,       /* SLA D           |     2m */
    0xCB, 0x23,       /* SLA E           |     2m */
    0xCB, 0x24,       /* SLA H           |     2m */
    0xCB, 0x25,       /* SLA L           |     2m */
    0xCB, 0x26,       /* SLA (HL)        |     4m */
    0xCB, 0x27,       /* SLA A           |     2m */
    0xCB, 0x28,       /* SRA B           |     2m */
    0xCB, 0x29,       /* SRA C           |     2m */
    0xCB, 0x2A,       /* SRA D           |     2m */
    0xCB, 0x2B,       /* SRA E           |     2m */
    0xCB, 0x2C,       /* SRA H           |     2m */
    0xCB, 0x2D,       /* SRA L           |     2m */
    0xCB, 0x2E,       /* SRA (HL)        |     4m */
    0xCB, 0x2F,       /* SRA A           |     2m */
    0xCB, 0x30,       /* SWAP B          |     2m */
    0xCB, 0x31,       /* SWAP C          |     2m */
    0xCB, 0x32,       /* SWAP D          |     2m */
    0xCB, 0x33,       /* SWAP E          |     2m */
    0xCB, 0x34,       /* SWAP H          |     2m */
    0xCB, 0x35,       /* SWAP L          |     2m */
    0xCB, 0x36,       /* SWAP (HL)       |     4m */
    0xCB, 0x37,       /* SWAP A          |     2m */
    0xCB, 0x38,       /* SRL B           |     2m */
    0xCB, 0x39,       /* SRL C           |     2m */
    0xCB, 0x3A,       /* SRL D           |     2m */
    0xCB, 0x3B,       /* SRL E           |     2m */
    0xCB, 0x3C,       /* SRL H           |     2m */
    0xCB, 0x3D,       /* SRL L           |     2m */
    0xCB, 0x3E,       /* SRL (HL)        |     4m */
    0xCB, 0x3F,       /* SRL A           |     2m */
    0xCB, 0x40,       /* BIT 0, B        |     2m */
    0xCB, 0x41,       /* BIT 0, C        |     2m */
    0xCB, 0x42,       /* BIT 0, D        |     2m */
    0xCB, 0x43,       /* BIT 0, E        |     2m */
    0xCB, 0x44,       /* BIT 0, H        |     2m */
    0xCB, 0x45,       /* BIT 0, L        |     2m */
    0xCB, 0x46,       /* BIT 0, (HL)     |     4m */
    0xCB, 0x47,       /* BIT 0, A        |     2m */
    0xCB, 0x48,       /* BIT 1, B        |     2m */
    0xCB, 0x49,       /* BIT 1, C        |     2m */
    0xCB, 0x4A,       /* BIT 1, D        |     2m */
    0xCB, 0x4B,       /* BIT 1, E        |     2m */
    0xCB, 0x4C,       /* BIT 1, H        |     2m */
    0xCB, 0x4D,       /* BIT 1, L        |     2m */
    0xCB, 0x4E,       /* BIT 1, (HL)     |     4m */
    0xCB, 0x4F,       /* BIT 1, A        |     2m */
    0xCB, 0x50,       /* BIT 2, B        |     2m */
    0xCB, 0x51,       /* BIT 2, C        |     2m */
    0xCB, 0x52,       /* BIT 2, D        |     2m */
    0xCB, 0x53,       /* BIT 2, E        |     2m */
    0xCB, 0x54,       /* BIT 2, H        |     2m */
    0xCB, 0x55,       /* BIT 2, L        |     2m */
    0xCB, 0x56,       /* BIT 2, (HL)     |     4m */
    0xCB, 0x57,       /* BIT 2, A        |     2m */
    0xCB, 0x58,       /* BIT 3, B        |     2m */
    0xCB, 0x59,       /* BIT 3, C        |     2m */
    0xCB, 0x5A,       /* BIT 3, D        |     2m */
    0xCB, 0x5B,       /* BIT 3, E        |     2m */
    0xCB, 0x5C,       /* BIT 3, H        |     2m */
    0xCB, 0x5D,       /* BIT 3, L        |     2m */
    0xCB, 0x5E,       /* BIT 3, (HL)     |     4m */
    0xCB, 0x5F,       /* BIT 3, A        |     2m */
    0xCB, 0x60,       /* BIT 4, B        |     2m */
    0xCB, 0x61,       /* BIT 4, C        |     2m */
    0xCB, 0x62,       /* BIT 4, D        |     2m */
    0xCB, 0x63,       /* BIT 4, E        |     2m */
    0xCB, 0x64,       /* BIT 4, H        |     2m */
    0xCB, 0x65,       /* BIT 4, L        |     2m */
    0xCB, 0x66,       /* BIT 4, (HL)     |     4m */
    0xCB, 0x67,       /* BIT 4, A        |     2m */
    0xCB, 0x68,       /* BIT 5, B        |     2m */
    0xCB, 0x69,       /* BIT 5, C        |     2m */
    0xCB, 0x6A,       /* BIT 5, D        |     2m */
    0xCB, 0x6B,       /* BIT 5, E        |     2m */
    0xCB, 0x6C,       /* BIT 5, H        |     2m */
    0xCB, 0x6D,       /* BIT 5, L        |     2m */
    0xCB, 0x6E,       /* BIT 5, (HL)     |     4m */
    0xCB, 0x6F,       /* BIT 5, A        |     2m */
    0xCB, 0x70,       /* BIT 6, B        |     2m */
    0xCB, 0x71,       /* BIT 6, C        |     2m */
    0xCB, 0x72,       /* BIT 6, D        |     2m */
    0xCB, 0x73,       /* BIT 6, E        |     2m */
    0xCB, 0x74,       /* BIT 6, H        |     2m */
    0xCB, 0x75,       /* BIT 6, L        |     2m */
    0xCB, 0x76,       /* BIT 6, (HL)     |     4m */
    0xCB, 0x77,       /* BIT 6, A        |     2m */
    0xCB, 0x78,       /* BIT 7, B        |     2m */
    0xCB, 0x79,       /* BIT 7, C        |     2m */
    0xCB, 0x7A,       /* BIT 7, D        |     2m */
    0xCB, 0x7B,       /* BIT 7, E        |     2m */
    0xCB, 0x7C,       /* BIT 7, H        |     2m */
    0xCB, 0x7D,       /* BIT 7, L        |     2m */
    0xCB, 0x7E,       /* BIT 7, (HL)     |     4m */
    0xCB, 0x7F,       /* BIT 7, A        |     2m */
    0xCB, 0x80,       /* RES 0, B        |     2m */
    0xCB, 0x81,       /* RES 0, C        |     2m */
    0xCB, 0x82,       /* RES 0, D        |     2m */
    0xCB, 0x83,       /* RES 0, E        |     2m */
    0xCB, 0x84,       /* RES 0, H        |     2m */
    0xCB, 0x85,       /* RES 0, L        |     2m */
    0xCB, 0x86,       /* RES 0, (HL)     |     4m */
    0xCB, 0x87,       /* RES 0, A        |     2m */
    0xCB, 0x88,       /* RES 1, B        |     2m */
    0xCB, 0x89,       /* RES 1, C        |     2m */
    0xCB, 0x8A,       /* RES 1, D        |     2m */
    0xCB, 0x8B,       /* RES 1, E        |     2m */
    0xCB, 0x8C,       /* RES 1, H        |     2m */
    0xCB, 0x8D,       /* RES 1, L        |     2m */
    0xCB, 0x8E,       /* RES 1, (HL)     |     4m */
    0xCB, 0x8F,       /* RES 1, A        |     2m */
    0xCB, 0x90,       /* RES 2, B        |     2m */
    0xCB, 0x91,       /* RES 2, C        |     2m */
    0xCB, 0x92,       /* RES 2, D        |     2m */
    0xCB, 0x93,       /* RES 2, E        |     2m */
    0xCB, 0x94,       /* RES 2, H        |     2m */
    0xCB, 0x95,       /* RES 2, L        |     2m */
    0xCB, 0x96,       /* RES 2, (HL)     |     4m */
    0xCB, 0x97,       /* RES 2, A        |     2m */
    0xCB, 0x98,       /* RES 3, B        |     2m */
    0xCB, 0x99,       /* RES 3, C        |     2m */
    0xCB, 0x9A,       /* RES 3, D        |     2m */
    0xCB, 0x9B,       /* RES 3, E        |     2m */
    0xCB, 0x9C,       /* RES 3, H        |     2m */
    0xCB, 0x9D,       /* RES 3, L        |     2m */
    0xCB, 0x9E,       /* RES 3, (HL)     |     4m */
    0xCB, 0x9F,       /* RES 3, A        |     2m */
    0xCB, 0xA0,       /* RES 4, B        |     2m */
    0xCB, 0xA1,       /* RES 4, C        |     2m */
    0xCB, 0xA2,       /* RES 4, D        |     2m */
    0xCB, 0xA3,       /* RES 4, E        |     2m */
    0xCB, 0xA4,       /* RES 4, H        |     2m */
    0xCB, 0xA5,       /* RES 4, L        |     2m */
    0xCB, 0xA6,       /* RES 4, (HL)     |     4m */
    0xCB, 0xA7,       /* RES 4, A        |     2m */
    0xCB, 0xA8,       /* RES 5, B        |     2m */
    0xCB, 0xA9,       /* RES 5, C        |     2m */
    0xCB, 0xAA,       /* RES 5, D        |     2m */
    0xCB, 0xAB,       /* RES 5, E        |     2m */
    0xCB, 0xAC,       /* RES 5, H        |     2m */
    0xCB, 0xAD,       /* RES 5, L        |     2m */
    0xCB, 0xAE,       /* RES 5, (HL)     |     4m */
    0xCB, 0xAF,       /* RES 5, A        |     2m */
    0xCB, 0xB0,       /* RES 6, B        |     2m */
    0xCB, 0xB1,       /* RES 6, C        |     2m */
    0xCB, 0xB2,       /* RES 6, D        |     2m */
    0xCB, 0xB3,       /* RES 6, E        |     2m */
    0xCB, 0xB4,       /* RES 6, H        |     2m */
    0xCB, 0xB5,       /* RES 6, L        |     2m */
    0xCB, 0xB6,       /* RES 6, (HL)     |     4m */
    0xCB, 0xB7,       /* RES 6, A        |     2m */
    0xCB, 0xB8,       /* RES 7, B        |     2m */
    0xCB, 0xB9,       /* RES 7, C        |     2m */
    0xCB, 0xBA,       /* RES 7, D        |     2m */
    0xCB, 0xBB,       /* RES 7, E        |     2m */
    0xCB, 0xBC,       /* RES 7, H        |     2m */
    0xCB, 0xBD,       /* RES 7, L        |     2m */
    0xCB, 0xBE,       /* RES 7, (HL)     |     4m */
    0xCB, 0xBF,       /* RES 7, A        |     2m */
    0xCB, 0xC0,       /* SET 0, B        |     2m */
    0xCB, 0xC1,       /* SET 0, C        |     2m */
    0xCB, 0xC2,       /* SET 0, D        |     2m */
    0xCB, 0xC3,       /* SET 0, E        |     2m */
    0xCB, 0xC4,       /* SET 0, H        |     2m */
    0xCB, 0xC5,       /* SET 0, L        |     2m */
    0xCB, 0xC6,       /* SET 0, (HL)     |     4m */
    0xCB, 0xC7,       /* SET 0, A        |     2m */
    0xCB, 0xC8,       /* SET 1, B        |     2m */
    0xCB, 0xC9,       /* SET 1, C        |     2m */
    0xCB, 0xCA,       /* SET 1, D        |     2m */
    0xCB, 0xCB,       /* SET 1, E        |     2m */
    0xCB, 0xCC,       /* SET 1, H        |     2m */
    0xCB, 0xCD,       /* SET 1, L        |     2m */
    0xCB, 0xCE,       /* SET 1, (HL)     |     4m */
    0xCB, 0xCF,       /* SET 1, A        |     2m */
    0xCB, 0xD0,       /* SET 2, B        |     2m */
    0xCB, 0xD1,       /* SET 2, C        |     2m */
    0xCB, 0xD2,       /* SET 2, D        |     2m */
    0xCB, 0xD3,       /* SET 2, E        |     2m */
    0xCB, 0xD4,       /* SET 2, H        |     2m */
    0xCB, 0xD5,       /* SET 2, L        |     2m */
    0xCB, 0xD6,       /* SET 2, (HL)     |     4m */
    0xCB, 0xD7,       /* SET 2, A        |     2m */
    0xCB, 0xD8,       /* SET 3, B        |     2m */
    0xCB, 0xD9,       /* SET 3, C        |     2m */
    0xCB, 0xDA,       /* SET 3, D        |     2m */
    0xCB, 0xDB,       /* SET 3, E        |     2m */
    0xCB, 0xDC,       /* SET 3, H        |     2m */
    0xCB, 0xDD,       /* SET 3, L        |     2m */
    0xCB, 0xDE,       /* SET 3, (HL)     |     4m */
    0xCB, 0xDF,       /* SET 3, A        |     2m */
    0xCB, 0xE0,       /* SET 4, B        |     2m */
    0xCB, 0xE1,       /* SET 4, C        |     2m */
    0xCB, 0xE2,       /* SET 4, D        |     2m */
    0xCB, 0xE3,       /* SET 4, E        |     2m */
    0xCB, 0xE4,       /* SET 4, H        |     2m */
    0xCB, 0xE5,       /* SET 4, L        |     2m */
    0xCB, 0xE6,       /* SET 4, (HL)     |     4m */
    0xCB, 0xE7,       /* SET 4, A        |     2m */
    0xCB, 0xE8,       /* SET 5, B        |     2m */
    0xCB, 0xE9,       /* SET 5, C        |     2m */
    0xCB, 0xEA,       /* SET 5, D        |     2m */
    0xCB, 0xEB,       /* SET 5, E        |     2m */
    0xCB, 0xEC,       /* SET 5, H        |     2m */
    0xCB, 0xED,       /* SET 5, L        |     2m */
    0xCB, 0xEE,       /* SET 5, (HL)     |     4m */
    0xCB, 0xEF,       /* SET 5, A        |     2m */
    0xCB, 0xF0,       /* SET 6, B        |     2m */
    0xCB, 0xF1,       /* SET 6, C        |     2m */
    0xCB, 0xF2,       /* SET 6, D        |     2m */
    0xCB, 0xF3,       /* SET 6, E        |     2m */
    0xCB, 0xF4,       /* SET 6, H        |     2m */
    0xCB, 0xF5,       /* SET 6, L        |     2m */
    0xCB, 0xF6,       /* SET 6, (HL)     |     4m */
    0xCB, 0xF7,       /* SET 6, A        |     2m */
    0xCB, 0xF8,       /* SET 7, B        |     2m */
    0xCB, 0xF9,       /* SET 7, C        |     2m */
    0xCB, 0xFA,       /* SET 7, D        |     2m */
    0xCB, 0xFB,       /* SET 7, E        |     2m */
    0xCB, 0xFC,       /* SET 7, H        |     2m */
    0xCB, 0xFD,       /* SET 7, L        |     2m */
    0xCB, 0xFE,       /* SET 7, (HL)     |     4m */
    0xCB, 0xFF,       /* SET 7, A        |     2m */
};
const char *INSTR_STRINGS[TEST_OPCODES_NUM] = {
    "NOOP",
    "LD BC, 0xCAFE",
    "LD (BC), A",
    "INC BC",
    "INC B",
    "DEC B",
    "LD B, 0xAA",
    "RLCA",
    "LD (0xCAFE), SP",
    "ADD HL, BC",
    "LD A, (BC)",
    "DEC BC",
    "INC C",
    "DEC C",
    "LD C, 0xAA",
    "RRCA",
    "STOP",
    "LD DE, 0xCAFE",
    "LD (DE), A",
    "INC DE",
    "INC D",
    "DEC D",
    "LD D, 0xAA",
    "RLA",
    "JR 0xAA",
    "ADD HL, DE",
    "LD A, (DE)",
    "DEC DE",
    "INC E",
    "DEC E",
    "LD E, 0xAA",
    "RRA",
    "JR NZ, 0xAA",
    "LD HL, 0xCAFE",
    "LD (HL+), A",
    "INC HL,",
    "INC H",
    "DEC H",
    "LD H, 0xAA",
    "DAA",
    "JR Z, 0xAA",
    "ADD  HL, HL",
    "LD A, (HL+)",
    "DEC HL",
    "INC L",
    "DEC L",
    "LD L, 0xAA",
    "CPL",
    "JR NC, 0xAA",
    "LD SP, 0xCAFE",
    "LD (HL-), A",
    "INC SP",
    "INC (HL)",
    "DEC (HL)",
    "LD (HL), 0xAA",
    "SCF",
    "JR C, 0xAA",
    "ADD HL, SP",
    "LD A, (HL-)",
    "DEC SP",
    "INC A",
    "DEC A",
    "LD A, 0xAA",
    "CCF",
    "LD B, B",
    "LD B, C",
    "LD B, D",
    "LD B, E",
    "LD B, H",
    "LD B, L",
    "LD B, (HL)",
    "LD B, A",
    "LD C, B",
    "LD C, C",
    "LD C, D",
    "LD C, E",
    "LD C, H",
    "LD C, L",
    "LD C, (HL)",
    "LD C, A",
    "LD D, B",
    "LD D, C",
    "LD D, D",
    "LD D, E",
    "LD D, H",
    "LD D, L",
    "LD D, (HL)",
    "LD D, A",
    "LD E, B",
    "LD E, C",
    "LD E, D",
    "LD E, E",
    "LD E, H",
    "LD E, L",
    "LD E, (HL)",
    "LD E, A",
    "LD H, B",
    "LD H, C",
    "LD H, D",
    "LD H, E",
    "LD H, H",
    "LD H, L",
    "LD H, (HL)",
    "LD H, A",
    "LD L, B",
    "LD L, C",
    "LD L, D",
    "LD L, E",
    "LD L, H",
    "LD L, L",
    "LD L, (HL)",
    "LD L, A",
    "LD (HL), B",
    "LD (HL), C",
    "LD (HL), D",
    "LD (HL), E",
    "LD (HL), H",
    "LD (HL), L",
    "HALT",
    "LD (HL), A",
    "LD A, B",
    "LD A, C",
    "LD A, D",
    "LD A, E",
    "LD A, H",
    "LD A, L",
    "LD A, (HL)",
    "LD A, A",
    "ADD A, B",
    "ADD A, C",
    "ADD A, D",
    "ADD A, E",
    "ADD A, H",
    "ADD A, L",
    "ADD A, (HL)",
    "ADD A, A",
    "ADC A, B",
    "ADC A, C",
    "ADC A, D",
    "ADC A, E",
    "ADC A, H",
    "ADC A, L",
    "ADC A, (HL)",
    "ADC A, A",
    "SUB A, B",
    "SUB A, C",
    "SUB A, D",
    "SUB A, E",
    "SUB A, H",
    "SUB A, L",
    "SUB A, (HL)",
    "SUB A, A",
    "SBC A, B",
    "SBC A, C",
    "SBC A, D",
    "SBC A, E",
    "SBC A, H",
    "SBC A, L",
    "SBC A, (HL)",
    "SBC A, A",
    "AND A, B",
    "AND A, C",
    "AND A, D",
    "AND A, E",
    "AND A, H",
    "AND A, L",
    "AND A, (HL)",
    "AND A, A",
    "XOR A, B",
    "XOR A, C",
    "XOR A, D",
    "XOR A, E",
    "XOR A, H",
    "XOR A, L",
    "XOR A, (HL)",
    "XOR A, A",
    "OR A, B",
    "OR A, C",
    "OR A, D",
    "OR A, E",
    "OR A, H",
    "OR A, L",
    "OR A, (HL)",
    "OR A, A",
    "CP A, B",
    "CP A, C",
    "CP A, D",
    "CP A, E",
    "CP A, H",
    "CP A, L",
    "CP A, (HL)",
    "CP A, A",
    "CP A, A",
    "RET NZ",
    "POP BC",
    "JP NZ, 0xCAFE",
    "JP 0xCAFE",
    "CALL NZ, 0xCAFE",
    "PUSH BC",
    "ADD A, 0xAA",
    "RST 00h",
    "RET Z",
    "RET",
    "JP Z, 0xCAFE",
    "CALL Z, 0xCAFE",
    "CALL 0xCAFE",
    "ADC A, 0xAA",
    "RST 08h",
    "RET NC",
    "POP DE",
    "JP NC. 0xCAFE",
    "ILLEGAL",
    "CALL NC, 0xCAFE",
    "PUSH DE",
    "SUB A, 0xAA",
    "RST 10h",
    "RET C",
    "RETI",
    "JP C, 0xCAFE",
    "ILLEGAL",
    "CALL C, 0xCAFE",
    "ILLEGAL",
    "SBC A, 0xAA",
    "RST 18h",
    "LD (FF00+0xAA), A",
    "POP HL",
    "LD (FF00+C), A",
    "ILLEGAL",
    "ILLEGAL",
    "PUSH HL",
    "AND A, 0xAA",
    "RST 20h",
    "ADD SP, 0xAA",
    "JP HL",
    "LD (0xCAFE), A",
    "ILLEGAL",
    "ILLEGAL",
    "ILLEGAL",
    "XOR A, 0xAA",
    "RST 28h",
    "LD A, (FF00+0xAA)",
    "POP AF",
    "LD A, (FF00+C)",
    "DI",
    "ILLEGAL",
    "PUSH AF",
    "OR A, 0xAA",
    "RST 30h",
    "LD HL, SP+0xAA",
    "LD SP, HL",
    "LD A, (0xCAFE)",
    "EI",
    "ILLEGAL",
    "ILLEGAL",
    "CP A, 0xAA",
    "RST 38h",
    "RLC B",
    "RLC C",
    "RLC D",
    "RLC E",
    "RLC H",
    "RLC L",
    "RLC (HL)",
    "RLC A",
    "RRC B",
    "RRC C",
    "RRC D",
    "RRC E",
    "RRC H",
    "RRC L",
    "RRC (HL)",
    "RRC A",
    "RLC B",
    "RLC C",
    "RLC D",
    "RLC E",
    "RLC H",
    "RLC L",
    "RLC (HL)",
    "RLC A",
    "RRC B",
    "RRC C",
    "RRC D",
    "RRC E",
    "RRC H",
    "RRC L",
    "RRC (HL)",
    "RRC A",
    "RLC B",
    "RLC C",
    "RLC D",
    "RLC E",
    "RLC H",
    "RLC L",
    "RLC (HL)",
    "RLC A",
    "RRC B",
    "RRC C",
    "RRC D",
    "RRC E",
    "RRC H",
    "RRC L",
    "RRC (HL)",
    "RRC A",
    "RLC B",
    "RLC C",
    "RLC D",
    "RLC E",
    "RLC H",
    "RLC L",
    "RLC (HL)",
    "RLC A",
    "RRC B",
    "RRC C",
    "RRC D",
    "RRC E",
    "RRC H",
    "RRC L",
    "RRC (HL)",
    "RRC A",
    "RLC B",
    "RLC C",
    "RLC D",
    "RLC E",
    "RLC H",
    "RLC L",
    "RLC (HL)",
    "RLC A",
    "RRC B",
    "RRC C",
    "RRC D",
    "RRC E",
    "RRC H",
    "RRC L",
    "RRC (HL)",
    "RRC A",
    "RLC B",
    "RLC C",
    "RLC D",
    "RLC E",
    "RLC H",
    "RLC L",
    "RLC (HL)",
    "RLC A",
    "RRC B",
    "RRC C",
    "RRC D",
    "RRC E",
    "RRC H",
    "RRC L",
    "RRC (HL)",
    "RRC A",
    "RLC B",
    "RLC C",
    "RLC D",
    "RLC E",
    "RLC H",
    "RLC L",
    "RLC (HL)",
    "RLC A",
    "RRC B",
    "RRC C",
    "RRC D",
    "RRC E",
    "RRC H",
    "RRC L",
    "RRC (HL)",
    "RRC A",
    "RLC B",
    "RLC C",
    "RLC D",
    "RLC E",
    "RLC H",
    "RLC L",
    "RLC (HL)",
    "RLC A",
    "RRC B",
    "RRC C",
    "RRC D",
    "RRC E",
    "RRC H",
    "RRC L",
    "RRC (HL)",
    "RRC A",
    "RLC B",
    "RLC C",
    "RLC D",
    "RLC E",
    "RLC H",
    "RLC L",
    "RLC (HL)",
    "RLC A",
    "RRC B",
    "RRC C",
    "RRC D",
    "RRC E",
    "RRC H",
    "RRC L",
    "RRC (HL)",
    "RRC A",
    "RLC B",
    "RLC C",
    "RLC D",
    "RLC E",
    "RLC H",
    "RLC L",
    "RLC (HL)",
    "RLC A",
    "RRC B",
    "RRC C",
    "RRC D",
    "RRC E",
    "RRC H",
    "RRC L",
    "RRC (HL)",
    "RRC A",
    "RLC B",
    "RLC C",
    "RLC D",
    "RLC E",
    "RLC H",
    "RLC L",
    "RLC (HL)",
    "RLC A",
    "RRC B",
    "RRC C",
    "RRC D",
    "RRC E",
    "RRC H",
    "RRC L",
    "RRC (HL)",
    "RRC A",
    "RLC B",
    "RLC C",
    "RLC D",
    "RLC E",
    "RLC H",
    "RLC L",
    "RLC (HL)",
    "RLC A",
    "RRC B",
    "RRC C",
    "RRC D",
    "RRC E",
    "RRC H",
    "RRC L",
    "RRC (HL)",
    "RRC A",
    "RLC B",
    "RLC C",
    "RLC D",
    "RLC E",
    "RLC H",
    "RLC L",
    "RLC (HL)",
    "RLC A",
    "RRC B",
    "RRC C",
    "RRC D",
    "RRC E",
    "RRC H",
    "RRC L",
    "RRC (HL)",
    "RRC A",
    "RLC B",
    "RLC C",
    "RLC D",
    "RLC E",
    "RLC H",
    "RLC L",
    "RLC (HL)",
    "RLC A",
    "RRC B",
    "RRC C",
    "RRC D",
    "RRC E",
    "RRC H",
    "RRC L",
    "RRC (HL)",
    "RRC A",
    "RLC B",
    "RLC C",
    "RLC D",
    "RLC E",
    "RLC H",
    "RLC L",
    "RLC (HL)",
    "RLC A",
    "RRC B",
    "RRC C",
    "RRC D",
    "RRC E",
    "RRC H",
    "RRC L",
    "RRC (HL)",
    "RRC A",
    "RLC B",
    "RLC C",
    "RLC D",
    "RLC E",
    "RLC H",
    "RLC L",
    "RLC (HL)",
    "RLC A",
    "RRC B",
    "RRC C",
    "RRC D",
    "RRC E",
    "RRC H",
    "RRC L",
    "RRC (HL)",
    "RRC A"
};
/* clang-format on */
const uint8_t INSTR_CLOCKS[TEST_OPCODES_NUM] = {
    1, 3, 2, 2, 1, 1, 2, 1, 5, 2, 2, 2, 1, 1, 2, 1, 1, 3, 2, 2, 1, 1, 2, 1, 3, 2, 2, 2, 1, 1, 2, 1,
    2, 3, 2, 2, 1, 1, 2, 1, 2, 2, 2, 2, 1, 1, 2, 1, 2, 3, 2, 2, 3, 3, 3, 1, 2, 2, 2, 2, 1, 1, 2, 1,
    1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 2, 1,
    1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 2, 1, 2, 2, 2, 2, 2, 2, 1, 2, 1, 1, 1, 1, 1, 1, 2, 1,
    1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 2, 1,
    1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 2, 1,
    2, 3, 3, 4, 3, 4, 2, 4, 2, 4, 3, 3, 6, 2, 4, 2, 3, 3, 1, 3, 4, 2, 4, 2, 4, 3, 1, 3, 1, 2, 4, 3,
    3, 2, 1, 1, 4, 2, 4, 4, 1, 4, 1, 1, 1, 2, 4, 3, 3, 2, 1, 1, 4, 2, 4, 3, 2, 4, 1, 1, 1, 2, 4, 2,
    2, 2, 2, 2, 2, 4, 2, 2, 2, 2, 2, 2, 2, 4, 2, 2, 2, 2, 2, 2, 2, 4, 2, 2, 2, 2, 2, 2, 2, 4, 2, 2,
    2, 2, 2, 2, 2, 4, 2, 2, 2, 2, 2, 2, 2, 4, 2, 2, 2, 2, 2, 2, 2, 4, 2, 2, 2, 2, 2, 2, 2, 4, 2, 2,
    2, 2, 2, 2, 2, 3, 2, 2, 2, 2, 2, 2, 2, 3, 2, 2, 2, 2, 2, 2, 2, 3, 2, 2, 2, 2, 2, 2, 2, 3, 2, 2,
    2, 2, 2, 2, 2, 3, 2, 2, 2, 2, 2, 2, 2, 3, 2, 2, 2, 2, 2, 2, 2, 3, 2, 2, 2, 2, 2, 2, 2, 3, 2, 2,
    2, 2, 2, 2, 2, 4, 2, 2, 2, 2, 2, 2, 2, 4, 2, 2, 2, 2, 2, 2, 2, 4, 2, 2, 2, 2, 2, 2, 2, 4, 2, 2,
    2, 2, 2, 2, 2, 4, 2, 2, 2, 2, 2, 2, 2, 4, 2, 2, 2, 2, 2, 2, 2, 4, 2, 2, 2, 2, 2, 2, 2, 4, 2, 2,
    2, 2, 2, 2, 2, 4, 2, 2, 2, 2, 2, 2, 2, 4, 2, 2, 2, 2, 2, 2, 2, 4, 2, 2, 2, 2, 2, 2, 2, 4, 2, 2,
    2, 2, 2, 2, 2, 4, 2, 2, 2, 2, 2, 2, 2, 4, 2, 2, 2, 2, 2, 2, 2, 4, 2, 2, 2, 2, 2, 2, 2, 4, 2,
};
const uint8_t INSTR_LENGTH[TEST_OPCODES_NUM] = {
    1, 3, 1, 1, 1, 1, 2, 1, 3, 1, 1, 1, 1, 1, 2, 1, 1, 3, 1, 1, 1, 1, 2, 1, 2, 1, 1, 1, 1, 1, 2, 1,
    2, 3, 1, 1, 1, 1, 2, 1, 2, 1, 1, 1, 1, 1, 2, 1, 2, 3, 1, 1, 1, 1, 2, 1, 2, 1, 1, 1, 1, 1, 2, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 3, 3, 3, 1, 2, 1, 1, 1, 3, 3, 3, 2, 1, 1, 1, 3, 1, 3, 1, 2, 1, 1, 1, 3, 1, 3, 1, 2, 1, 2,
    1, 1, 1, 1, 1, 2, 1, 2, 1, 3, 1, 1, 1, 2, 1, 2, 1, 1, 1, 1, 1, 2, 1, 2, 1, 3, 1, 1, 1, 2, 1, 2,
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
};

int main() {
    int i;
    instruction_t instr;
    decoder_t d = decoder_new(TEST_OPCODES, TEST_OPCODES_SIZE);
    for (i = 0; i < TEST_OPCODES_NUM - 1; i++) {
        instr = decoder_next(&d);
        /* TODO printing */
        /* assert(strcmp(print_instruction(&instr), INSTR_STRINGS[i])); */
        assert(instr.clocks == INSTR_CLOCKS[i]);
        assert(instr.length == INSTR_LENGTH[i]);
    }
    return 0;
}
