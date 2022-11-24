#include "src/gameboy.h"
#include <assert.h>

/* clang-format off */
const uint8_t TEST_BOOTROM[256] = {
                      /* MNEMONIC     | TIMING | CURRENT CYCLES */
    0x00,             /* NOOP         |     1m |              1 */
    0x01, 0xFE, 0xCA, /* LD BC, u16   |     3m |              4 */
    0x02,             /* LD (BC), A   |     2m |              6 */
    0x03,             /* INC BC       |     2m |              8 */
    0x04,             /* INC B        |     1m |              9 */
    0x05,             /* DEC B        |     1m |             10 */
    0x06, 0xAA,       /* LD B, u8     |     2m |             12 */
    0x07,             /* RLCA         |     1m |             13 */
    0x08, 0xFE, 0xCA, /* LD {u16}, SP |     5m |             18 */
    0x09,             /* ADD HL, BC   |     2m |             20 */
    0x0A,             /* LD A, (BC)   |     2m |             22 */
    0x0B,             /* DEC BC       |     2m |             24 */
    0x0C,             /* INC C        |     1m |             25 */
    0x0D,             /* DEC C        |     1m |             26 */
    0x0E, 0xAA,       /* LD C, u8     |     2m |             28 */
    0x0F,             /* RRCA         |     1m |             29 */
    0x10,             /* STOP         |     1m |             30 */
    0x11, 0xFE, 0xCA, /* LD DE, u16   |     3m |             33 */
    0x12,             /* LD (DE), A   |     2m |             35 */
    0x13,             /* INC DE       |     2m |             37 */
    0x14,             /* INC D        |     1m |             38 */
    0x15,             /* DEC D        |     1m |             39 */
    0x16, 0xAA,       /* LD D, u8     |     2m |             41 */
    0x17,             /* RLA          |     1m |             42 */
    0x18, 0x00,       /* JR i8        |     3m |             45 */
    0x19,             /* ADD HL, DE   |     2m |             47 */
    0x1A,             /* LD A, (DE)   |     2m |             49 */
    0x1B,             /* DEC DE       |     2m |             51 */
    0x1C,             /* INC E        |     1m |             52 */
    0x1D,             /* DEC E        |     1m |             53 */
    0x1E, 0xAA,       /* LD E, u8     |     2m |             55 */
    0x1F,             /* RRA          |     1m |             56 */
    0x20, 0x00,       /* JR NZ, i8    |  2m-3m |             58 */
    0x21, 0xFE, 0xCA, /* LD HL, u16   |     3m |             61 */
    0x22,             /* LD (HL+), A  |     2m |             63 */
    0x23,             /* INC HL,      |     2m |             65 */
    0x24,             /* INC H        |     1m |             66 */
    0x25,             /* DEC H        |     1m |             67 */
    0x26, 0xAA,       /* LD H, u8     |     2m |             69 */
    0x27,             /* DAA          |     1m |             70 */
    0x28, 0x00,       /* JR Z, i8     |  2m-3m |             72 */
    0x29,             /* ADD  HL, HL  |     2m |             74 */
    0x2A,             /* LD A, (HL+)  |     2m |             76 */
    0x2B,             /* DEC HL       |     2m |             78 */
    0x2C,             /* INC L        |     1m |             79 */
    0x2D,             /* DEC L        |     1m |             80 */
    0x2E, 0xAA,       /* LD L, u8     |     2m |             82 */
    0x2F,             /* CPL          |     1m |             83 */
    0x30, 0x00,       /* JR NC, i8    |  2m-3m |             85 */
    0x31, 0xFE, 0xCA, /* LD SP, u16   |     3m |             88 */
    0x32,             /* LD (HL-), A  |     2m |             90 */
    0x33,             /* INC SP       |     2m |             92 */
    0x34,             /* INC (HL)     |     3m |             95 */
    0x35,             /* DEC (HL)     |     3m |             98 */
    0x36, 0xAA,       /* LD (HL), u8  |     3m |            101 */
    0x37,             /* SCF          |     1m |            102 */
    0x38, 0x00,       /* JR C, i8     |  2m-3m |            104 */
    0x39,             /* ADD HL, SP   |     2m |            106 */
    0x3A,             /* LD A, (HL-)  |     2m |            108 */
    0x3B,             /* DEC SP       |     2m |            110 */
    0x3C,             /* INC A        |     1m |            111 */
    0x3D,             /* DEC A        |     1m |            112 */
    0x3E, 0xAA,       /* LD A, u8     |     2m |            114 */
    0x3F,             /* CCF          |     1m |            115 */
    0x40,             /* LD B, B      |     1m |            116 */
    0x41,             /* LD B, C      |     1m |            117 */
    0x42,             /* LD B, D      |     1m |            118 */
    0x43,             /* LD B, E      |     1m |            119 */
    0x44,             /* LD B, H      |     1m |            120 */
    0x45,             /* LD B, L      |     1m |            121 */
    0x46,             /* LD B, (HL)   |     2m |            123 */
    0x47,             /* LD B, A      |     1m |            124 */
    0x48,             /* LD C, B      |     1m |            125 */
    0x49,             /* LD C, C      |     1m |            126 */
    0x4A,             /* LD C, D      |     1m |            127 */
    0x4B,             /* LD C, E      |     1m |            128 */
    0x4C,             /* LD C, H      |     1m |            129 */
    0x4D,             /* LD C, L      |     1m |            130 */
    0x4E,             /* LD C, (HL)   |     2m |            132 */
    0x4F,             /* LD C, A      |     1m |            133 */
    0x50,             /* LD D, B      |     1m |            134 */
    0x51,             /* LD D, C      |     1m |            135 */
    0x52,             /* LD D, D      |     1m |            136 */
    0x53,             /* LD D, E      |     1m |            137 */
    0x54,             /* LD D, H      |     1m |            138 */
    0x55,             /* LD D, L      |     1m |            139 */
    0x56,             /* LD D, (HL)   |     2m |            141 */
    0x57,             /* LD D, A      |     1m |            142 */
    0x58,             /* LD E, B      |     1m |            143 */
    0x59,             /* LD E, C      |     1m |            144 */
    0x5A,             /* LD E, D      |     1m |            145 */
    0x5B,             /* LD E, E      |     1m |            146 */
    0x5C,             /* LD E, H      |     1m |            147 */
    0x5D,             /* LD E, L      |     1m |            148 */
    0x5E,             /* LD E, (HL)   |     2m |            150 */
    0x5F,             /* LD E, A      |     1m |            151 */
    0x60,             /* LD H, B      |     1m |            152 */
    0x61,             /* LD H, C      |     1m |            153 */
    0x62,             /* LD H, D      |     1m |            154 */
    0x63,             /* LD H, E      |     1m |            155 */
    0x64,             /* LD H, H      |     1m |            156 */
    0x65,             /* LD H, L      |     1m |            157 */
    0x66,             /* LD H, (HL)   |     2m |            159 */
    0x67,             /* LD H, A      |     1m |            160 */
    0x68,             /* LD L, B      |     1m |            161 */
    0x69,             /* LD L, C      |     1m |            162 */
    0x6A,             /* LD L, D      |     1m |            163 */
    0x6B,             /* LD L, E      |     1m |            164 */
    0x6C,             /* LD L, H      |     1m |            165 */
    0x6D,             /* LD L, L      |     1m |            166 */
    0x6E,             /* LD L, (HL)   |     2m |            168 */
    0x6F,             /* LD L, A      |     1m |            169 */
    0x70,             /* LD (HL), B   |     2m |            171 */
    0x71,             /* LD (HL), C   |     2m |            173 */
    0x72,             /* LD (HL), D   |     2m |            175 */
    0x73,             /* LD (HL), E   |     2m |            177 */
    0x74,             /* LD (HL), H   |     2m |            179 */
    0x75,             /* LD (HL), L   |     2m |            181 */
    0x76,             /* HALT         |     1m |            182 */
    0x77,             /* LD (HL), A   |     2m |            184 */
    0x78,             /* LD A, B      |     1m |            185 */
    0x79,             /* LD A, C      |     1m |            186 */
    0x7A,             /* LD A, D      |     1m |            187 */
    0x7B,             /* LD A, E      |     1m |            188 */
    0x7C,             /* LD A, H      |     1m |            189 */
    0x7D,             /* LD A, L      |     1m |            190 */
    0x7E,             /* LD A, (HL)   |     2m |            192 */
    0x7F,             /* LD A, A      |     1m |            193 */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    /* TODO: test instrs where branch is taken */
};
/* clang-format on */

int main() {
    gamegirl *gg = gamegirl_init(NULL);
    memcpy(gg->bus.bootrom, TEST_BOOTROM, 256);
    cpu_clock(&gg->cpu);
    assert(gg->cpu.clocks == 1);

    cpu_clock(&gg->cpu);
    assert(gg->cpu.clocks == 4);
    assert(gg->cpu.bc.u8.b == 0xCA);
    assert(gg->cpu.bc.u8.c == 0xFE);
    assert(gg->cpu.bc.u16 == 0xCAFE);

    cpu_clock(&gg->cpu);
    assert(gg->cpu.clocks == 6);
    assert(bus_read(&gg->bus, gg->cpu.bc.u16) == gg->cpu.af.u8.a);

    cpu_clock(&gg->cpu);
    assert(gg->cpu.clocks == 8);
    assert(gg->cpu.bc.u16 == 0xCAFF);

    cpu_clock(&gg->cpu);
    assert(gg->cpu.clocks == 9);
    assert(gg->cpu.bc.u8.b == 0xCB);

    cpu_clock(&gg->cpu);
    assert(gg->cpu.clocks == 10);
    assert(gg->cpu.bc.u8.b == 0xCA);

    cpu_clock(&gg->cpu);
    assert(gg->cpu.clocks == 12);
    assert(gg->cpu.bc.u8.b == 0xAA);

    /* TODO: Check side effects of this */
    cpu_clock(&gg->cpu);
    assert(gg->cpu.clocks == 13);

    cpu_clock(&gg->cpu);
    assert(gg->cpu.clocks == 18);
    assert(bus_read(&gg->bus, 0xCAFE) == (gg->cpu.sp & 0xFF));
    assert(bus_read(&gg->bus, 0xCAFE + 1) == ((gg->cpu.sp >> 8) & 0xFF));

    cpu_clock(&gg->cpu);
    assert(gg->cpu.clocks == 20);
    assert(gg->cpu.hl.u16 == 0xAAFF);

    cpu_clock(&gg->cpu);
    assert(gg->cpu.clocks == 22);
    assert(gg->cpu.af.u8.a == bus_read(&gg->bus, 0xAAFF));

    cpu_clock(&gg->cpu);
    assert(gg->cpu.clocks == 24);
    assert(gg->cpu.bc.u16 == 0xAAFE);

    cpu_clock(&gg->cpu);
    assert(gg->cpu.clocks == 25);
    assert(gg->cpu.bc.u8.c == 0xFF);

    cpu_clock(&gg->cpu);
    assert(gg->cpu.clocks == 26);
    assert(gg->cpu.bc.u8.c == 0xFE);

    cpu_clock(&gg->cpu);
    assert(gg->cpu.clocks == 28);
    assert(gg->cpu.bc.u8.c == 0xAA);

    cpu_clock(&gg->cpu);
    assert(gg->cpu.clocks == 29);

    cpu_clock(&gg->cpu);
    assert(gg->cpu.clocks == 30);
    gg->cpu.mode = cpu_running_mode_e;

    cpu_clock(&gg->cpu);
    assert(gg->cpu.clocks == 33);
    assert(gg->cpu.de.u16 == 0xCAFE);

    cpu_clock(&gg->cpu);
    assert(gg->cpu.clocks == 35);
    cpu_clock(&gg->cpu);
    assert(gg->cpu.clocks == 37);
    cpu_clock(&gg->cpu);
    assert(gg->cpu.clocks == 38);
    cpu_clock(&gg->cpu);
    assert(gg->cpu.clocks == 39);
    cpu_clock(&gg->cpu);
    assert(gg->cpu.clocks == 41);
    cpu_clock(&gg->cpu);
    assert(gg->cpu.clocks == 42);
    cpu_clock(&gg->cpu);
    assert(gg->cpu.clocks == 45);
    cpu_clock(&gg->cpu);
    assert(gg->cpu.clocks == 47);
    cpu_clock(&gg->cpu);
    assert(gg->cpu.clocks == 49);
    cpu_clock(&gg->cpu);
    assert(gg->cpu.clocks == 51);
    cpu_clock(&gg->cpu);
    assert(gg->cpu.clocks == 52);
    cpu_clock(&gg->cpu);
    assert(gg->cpu.clocks == 53);
    cpu_clock(&gg->cpu);
    assert(gg->cpu.clocks == 55);
    cpu_clock(&gg->cpu);
    assert(gg->cpu.clocks == 56);

    gg->cpu.af.u8.f.bits.z = true;
    cpu_clock(&gg->cpu);
    assert(gg->cpu.clocks == 58);

    cpu_clock(&gg->cpu);
    assert(gg->cpu.clocks == 61);
    cpu_clock(&gg->cpu);
    assert(gg->cpu.clocks == 63);
    cpu_clock(&gg->cpu);
    assert(gg->cpu.clocks == 65);
    cpu_clock(&gg->cpu);
    assert(gg->cpu.clocks == 66);
    cpu_clock(&gg->cpu);
    assert(gg->cpu.clocks == 67);
    cpu_clock(&gg->cpu);
    assert(gg->cpu.clocks == 69);
    cpu_clock(&gg->cpu);
    assert(gg->cpu.clocks == 70);

    gg->cpu.af.u8.f.bits.z = false;
    cpu_clock(&gg->cpu);
    assert(gg->cpu.clocks == 72);

    cpu_clock(&gg->cpu);
    assert(gg->cpu.clocks == 74);
    cpu_clock(&gg->cpu);
    assert(gg->cpu.clocks == 76);
    cpu_clock(&gg->cpu);
    assert(gg->cpu.clocks == 78);
    cpu_clock(&gg->cpu);
    assert(gg->cpu.clocks == 79);
    cpu_clock(&gg->cpu);
    assert(gg->cpu.clocks == 80);
    cpu_clock(&gg->cpu);
    assert(gg->cpu.clocks == 82);
    cpu_clock(&gg->cpu);
    assert(gg->cpu.clocks == 83);

    gg->cpu.af.u8.f.bits.c = true;
    cpu_clock(&gg->cpu);
    assert(gg->cpu.clocks == 85);

    cpu_clock(&gg->cpu);
    assert(gg->cpu.clocks == 88);
    cpu_clock(&gg->cpu);
    assert(gg->cpu.clocks == 90);
    cpu_clock(&gg->cpu);
    assert(gg->cpu.clocks == 92);
    cpu_clock(&gg->cpu);
    assert(gg->cpu.clocks == 95);
    cpu_clock(&gg->cpu);
    assert(gg->cpu.clocks == 98);
    cpu_clock(&gg->cpu);
    assert(gg->cpu.clocks == 101);
    cpu_clock(&gg->cpu);
    assert(gg->cpu.clocks == 102);

    gg->cpu.af.u8.f.bits.c = false;
    cpu_clock(&gg->cpu);
    assert(gg->cpu.clocks == 104);

    cpu_clock(&gg->cpu);
    assert(gg->cpu.clocks == 106);
    cpu_clock(&gg->cpu);
    assert(gg->cpu.clocks == 108);
    cpu_clock(&gg->cpu);
    assert(gg->cpu.clocks == 110);
    cpu_clock(&gg->cpu);
    assert(gg->cpu.clocks == 111);
    cpu_clock(&gg->cpu);
    assert(gg->cpu.clocks == 112);
    cpu_clock(&gg->cpu);
    assert(gg->cpu.clocks == 114);
    cpu_clock(&gg->cpu);
    assert(gg->cpu.clocks == 115);
    cpu_clock(&gg->cpu);
    assert(gg->cpu.clocks == 116);
    cpu_clock(&gg->cpu);
    assert(gg->cpu.clocks == 117);
    cpu_clock(&gg->cpu);
    assert(gg->cpu.clocks == 118);
    cpu_clock(&gg->cpu);
    assert(gg->cpu.clocks == 119);
    cpu_clock(&gg->cpu);
    assert(gg->cpu.clocks == 120);
    cpu_clock(&gg->cpu);
    assert(gg->cpu.clocks == 121);
    cpu_clock(&gg->cpu);
    assert(gg->cpu.clocks == 123);
    cpu_clock(&gg->cpu);
    assert(gg->cpu.clocks == 124);
    cpu_clock(&gg->cpu);
    assert(gg->cpu.clocks == 125);
    cpu_clock(&gg->cpu);
    assert(gg->cpu.clocks == 126);
    cpu_clock(&gg->cpu);
    assert(gg->cpu.clocks == 127);
    cpu_clock(&gg->cpu);
    assert(gg->cpu.clocks == 128);
    cpu_clock(&gg->cpu);
    assert(gg->cpu.clocks == 129);
    cpu_clock(&gg->cpu);
    assert(gg->cpu.clocks == 130);
    cpu_clock(&gg->cpu);
    assert(gg->cpu.clocks == 132);
    cpu_clock(&gg->cpu);
    assert(gg->cpu.clocks == 133);
    cpu_clock(&gg->cpu);
    assert(gg->cpu.clocks == 134);
    cpu_clock(&gg->cpu);
    assert(gg->cpu.clocks == 135);
    cpu_clock(&gg->cpu);
    assert(gg->cpu.clocks == 136);
    cpu_clock(&gg->cpu);
    assert(gg->cpu.clocks == 137);
    cpu_clock(&gg->cpu);
    assert(gg->cpu.clocks == 138);
    cpu_clock(&gg->cpu);
    assert(gg->cpu.clocks == 139);
    cpu_clock(&gg->cpu);
    assert(gg->cpu.clocks == 141);
    cpu_clock(&gg->cpu);
    assert(gg->cpu.clocks == 142);
    cpu_clock(&gg->cpu);
    assert(gg->cpu.clocks == 143);
    cpu_clock(&gg->cpu);
    assert(gg->cpu.clocks == 144);
    cpu_clock(&gg->cpu);
    assert(gg->cpu.clocks == 145);
    cpu_clock(&gg->cpu);
    assert(gg->cpu.clocks == 146);
    cpu_clock(&gg->cpu);
    assert(gg->cpu.clocks == 147);
    cpu_clock(&gg->cpu);
    assert(gg->cpu.clocks == 148);
    cpu_clock(&gg->cpu);
    assert(gg->cpu.clocks == 150);
    cpu_clock(&gg->cpu);
    assert(gg->cpu.clocks == 151);
    cpu_clock(&gg->cpu);
    assert(gg->cpu.clocks == 152);
    cpu_clock(&gg->cpu);
    assert(gg->cpu.clocks == 153);
    cpu_clock(&gg->cpu);
    assert(gg->cpu.clocks == 154);
    cpu_clock(&gg->cpu);
    assert(gg->cpu.clocks == 155);
    cpu_clock(&gg->cpu);
    assert(gg->cpu.clocks == 156);
    cpu_clock(&gg->cpu);
    assert(gg->cpu.clocks == 157);
    cpu_clock(&gg->cpu);
    assert(gg->cpu.clocks == 159);
    cpu_clock(&gg->cpu);
    assert(gg->cpu.clocks == 160);
    cpu_clock(&gg->cpu);
    assert(gg->cpu.clocks == 161);
    cpu_clock(&gg->cpu);
    assert(gg->cpu.clocks == 162);
    cpu_clock(&gg->cpu);
    assert(gg->cpu.clocks == 163);
    cpu_clock(&gg->cpu);
    assert(gg->cpu.clocks == 164);
    cpu_clock(&gg->cpu);
    assert(gg->cpu.clocks == 165);
    cpu_clock(&gg->cpu);
    assert(gg->cpu.clocks == 166);
    cpu_clock(&gg->cpu);
    assert(gg->cpu.clocks == 168);
    cpu_clock(&gg->cpu);
    assert(gg->cpu.clocks == 169);

    gg->cpu.hl.u16 = 0xC000;
    cpu_clock(&gg->cpu);
    assert(gg->cpu.clocks == 171);

    cpu_clock(&gg->cpu);
    assert(gg->cpu.clocks == 173);
    cpu_clock(&gg->cpu);
    assert(gg->cpu.clocks == 175);
    cpu_clock(&gg->cpu);
    assert(gg->cpu.clocks == 177);
    cpu_clock(&gg->cpu);
    assert(gg->cpu.clocks == 179);
    cpu_clock(&gg->cpu);
    assert(gg->cpu.clocks == 181);

    cpu_clock(&gg->cpu);
    assert(gg->cpu.clocks == 182);
    gg->cpu.mode = cpu_running_mode_e;

    cpu_clock(&gg->cpu);
    assert(gg->cpu.clocks == 184);
    cpu_clock(&gg->cpu);
    assert(gg->cpu.clocks == 185);
    cpu_clock(&gg->cpu);
    assert(gg->cpu.clocks == 186);
    cpu_clock(&gg->cpu);
    assert(gg->cpu.clocks == 187);
    cpu_clock(&gg->cpu);
    assert(gg->cpu.clocks == 188);
    cpu_clock(&gg->cpu);
    assert(gg->cpu.clocks == 189);
    cpu_clock(&gg->cpu);
    assert(gg->cpu.clocks == 190);
    cpu_clock(&gg->cpu);
    assert(gg->cpu.clocks == 192);
    cpu_clock(&gg->cpu);
    assert(gg->cpu.clocks == 193);

    LOG("Test", "test_cpu passed!");
}
