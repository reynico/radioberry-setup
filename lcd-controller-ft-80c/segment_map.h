#ifndef SEGMENT_MAP_H
#define SEGMENT_MAP_H

// Segment encoding: (addr << 2) | bit
// Use SEG_ADDR(x) and SEG_BIT(x) to decode
#define SEG(addr, bit) (((addr) << 2) | (bit))
#define SEG_ADDR(seg) ((seg) >> 2)
#define SEG_BIT(seg) ((seg) & 0x03)

// 7-segment digit layout:
//      A
//     ---
//  F |   | B
//     -G-
//  E |   | C
//     ---
//      D

// Digit 7 (rightmost, frequency decimal)
#define DIG7_A SEG(7, 2)
#define DIG7_B SEG(6, 2)
#define DIG7_C SEG(8, 3)
#define DIG7_D SEG(6, 1)
#define DIG7_E SEG(7, 1)
#define DIG7_F SEG(7, 3)
#define DIG7_G SEG(6, 3)

// Digit 6
#define DIG6_A SEG(11, 2)
#define DIG6_B SEG(10, 2)
#define DIG6_C SEG(9, 3)
#define DIG6_D SEG(10, 1)
#define DIG6_E SEG(11, 1)
#define DIG6_F SEG(11, 3)
#define DIG6_G SEG(10, 3)

// Digit 5
#define DIG5_A SEG(13, 2)
#define DIG5_B SEG(12, 2)
#define DIG5_C SEG(14, 3)
#define DIG5_D SEG(12, 1)
#define DIG5_E SEG(13, 1)
#define DIG5_F SEG(13, 3)
#define DIG5_G SEG(12, 3)

// Digit 4
#define DIG4_A SEG(17, 2)
#define DIG4_B SEG(16, 2)
#define DIG4_C SEG(15, 3)
#define DIG4_D SEG(16, 1)
#define DIG4_E SEG(17, 1)
#define DIG4_F SEG(17, 3)
#define DIG4_G SEG(16, 3)

// Digit 3 (last before first decimal point)
#define DIG3_A SEG(19, 2)
#define DIG3_B SEG(18, 2)
#define DIG3_C SEG(20, 3)
#define DIG3_D SEG(18, 1)
#define DIG3_E SEG(19, 1)
#define DIG3_F SEG(19, 3)
#define DIG3_G SEG(18, 3)

// Digit 2
#define DIG2_A SEG(23, 2)
#define DIG2_B SEG(22, 2)
#define DIG2_C SEG(21, 3)
#define DIG2_D SEG(22, 1)
#define DIG2_E SEG(23, 1)
#define DIG2_F SEG(23, 3)
#define DIG2_G SEG(22, 3)

// Digit 1 (leftmost)
#define DIG1_A SEG(25, 2)
#define DIG1_B SEG(24, 2)
#define DIG1_C SEG(26, 3)
#define DIG1_D SEG(24, 1)
#define DIG1_E SEG(25, 1)
#define DIG1_F SEG(25, 3)
#define DIG1_G SEG(24, 3)

// Digit segment arrays for easy access
// Usage: DIGITS[digit_index][segment_index] where segment_index 0-6 = A-G
static const uint8_t DIGITS[7][7] = {
  { DIG1_A, DIG1_B, DIG1_C, DIG1_D, DIG1_E, DIG1_F, DIG1_G },
  { DIG2_A, DIG2_B, DIG2_C, DIG2_D, DIG2_E, DIG2_F, DIG2_G },
  { DIG3_A, DIG3_B, DIG3_C, DIG3_D, DIG3_E, DIG3_F, DIG3_G },
  { DIG4_A, DIG4_B, DIG4_C, DIG4_D, DIG4_E, DIG4_F, DIG4_G },
  { DIG5_A, DIG5_B, DIG5_C, DIG5_D, DIG5_E, DIG5_F, DIG5_G },
  { DIG6_A, DIG6_B, DIG6_C, DIG6_D, DIG6_E, DIG6_F, DIG6_G },
  { DIG7_A, DIG7_B, DIG7_C, DIG7_D, DIG7_E, DIG7_F, DIG7_G },
};

// 7-segment character patterns (bits: 0=A, 1=B, 2=C, 3=D, 4=E, 5=F, 6=G)
//      A(0)
//     ---
// F(5)|   | B(1)
//     -G(6)-
// E(4)|   | C(2)
//     ---
//      D(3)
#define CHAR_0 0b0111111  // ABCDEF
#define CHAR_1 0b0000110  // BC
#define CHAR_2 0b1011011  // ABDEG
#define CHAR_3 0b1001111  // ABCDG
#define CHAR_4 0b1100110  // BCFG
#define CHAR_5 0b1101101  // ACDFG
#define CHAR_6 0b1111101  // ACDEFG
#define CHAR_7 0b0000111  // ABC
#define CHAR_8 0b1111111  // ABCDEFG
#define CHAR_9 0b1101111  // ABCDFG
#define CHAR_BLANK 0b0000000
#define CHAR_DASH  0b1000000  // G

// Letters for callsign display
#define CHAR_A 0b1110111  // ABCEFG
#define CHAR_L 0b0111000  // DEF
#define CHAR_U 0b0111110  // BCDEF
#define CHAR_u 0b0011100  // CDE (lowercase)
#define CHAR_r 0b1010000  // EG (lowercase)
#define CHAR_n 0b1010100  // CEG (lowercase)

static const uint8_t CHAR_TABLE[12] = {
  CHAR_0, CHAR_1, CHAR_2, CHAR_3, CHAR_4,
  CHAR_5, CHAR_6, CHAR_7, CHAR_8, CHAR_9,
  CHAR_BLANK, CHAR_DASH
};

// Indicators
#define IND_PRI   SEG(3, 2)
#define IND_NAR   SEG(8, 2)
#define IND_FM    SEG(9, 2)
#define IND_AM    SEG(14, 2)
#define IND_CW    SEG(15, 2)
#define IND_USB   SEG(20, 2)
#define IND_LSB   SEG(21, 2)
#define IND_GEN   SEG(26, 2)
#define IND_LOCK  SEG(28, 1)
#define IND_MR    SEG(28, 2)
#define IND_FAST  SEG(28, 3)
#define IND_BUSY  SEG(29, 1)
#define IND_VFO_B SEG(29, 2)
#define IND_CLAR  SEG(29, 3)
#define IND_SCAN  SEG(30, 1)
#define IND_VFO_A SEG(30, 2)
#define IND_SPLIT SEG(30, 3)
#define IND_CAT   SEG(31, 1)
#define IND_BAND  SEG(31, 2)

#endif
