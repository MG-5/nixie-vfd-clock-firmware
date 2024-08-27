#include "Font.hpp"

/*       A1   A2
        ----I-----
       | \  |  / |
      F|  H | J  |B
       |   \|/   |
     G1 ---- ---- G2
       |   /|\   |
      E|  M | K  |C
       | /  |  \ |
        ----L----
         D2   D1    */

// 0bAABCDDEFGGHIJKLM

Font::Glyph missingGlyph = 0b1111111111111111;

// digits
Font::Glyph dg[] = {
    0b1111111100001001, // 0
    0b1000110000010010, // 1
    0b1110111011000000, // 2
    0b1111110001000000, // 3
    0b0011000111000000, // 4
    0b1101110111000000, // 5
    0b1101111111000000, // 6
    0b1111000000000000, // 7
    0b1111111111000000, // 8
    0b1111110111000000  // 9
};

// lowercase letters
Font::Glyph lc[] = {
    //  0bAABCDDEFGGHIJKLM
    0b0000111010000010, // a
    0b0001111111000000, // b
    0b0000111011000000, // c
    0b0011111011000000, // d
    0b0000011010000001, // e
    0b0100000011010010, // f
    0b0111100001010000, // g
    0b0001001111000000, // h
    0b0000000000000010, // i
    0b0011100000000010, // j
    0b0000000000011110, // k
    0b0000100000010010, // l
    0b0001001011000010, // m
    0b0000001010000100, // n
    0b0001111011000000, // o
    0b0110000001010010, // p
    0b0111000001010000, // q
    0b0000000001000010, // r
    0b0000100001000100, // s
    0b0000100011010010, // t
    0b0001111000000000, // u
    0b0000001000000001, // v
    0b0001001000000101, // w
    0b0000000000101101, // x
    0b0011100001010000, // y
    0b0000010010000001  // z
};

// uppercase letters
Font::Glyph uc[] = {
    0b1111001111000000, // A
    0b1111110001010010, // B
    0b1100111100000000, // C
    0b1111110000010010, // D
    0b1100111111000000, // E
    0b1100001111000000, // F
    0b1101111101000000, // G
    0b0011001111000000, // H
    0b1100110000010010, // I
    0b0011110000000000, // J
    0b0000001110001100, // K
    0b0000111100000000, // L
    0b0011001100101000, // M
    0b0011001100100100, // N
    0b1111111100000000, // O
    0b1110001111000000, // P
    0b1111111100000100, // Q
    0b1110001111000100, // R
    0b1101110001100000, // S
    0b1100000000010010, // T
    0b0011111100000000, // U
    0b0000001100001001, // V
    0b0011001100000101, // W
    0b0000000000101101, // X
    0b0000000000101010, // Y
    0b1100110000001001  // Z
};

// special characters
Font::Glyph sc[] = {
    0b0,                // 032_space
    0b0,                // 033_excl
    0b0,                // 034_quot
    0b0,                // 035_hash
    0b1101110111010010, // 036_dsig
    0b0,                // 037_perc
    0b0,                // 038_amp
    0b0000000000010000, // 039_apos
    0b0,                // 040_rbrl
    0b0,                // 041_rbrr
    0b0000000000111111, // 042_star
    0b0000000011010010, // 043_plus
    0b0000000000000001, // 044_comm
    0b0000000011000000, // 045_min
    0b0,                // 046_dot
    0b0000000000001001, // 047_slsh
    0b0,                // 058_coln
    0b0,                // 059_smcl
    0b0,                // 060_abrl
    0b0000110011000000, // 061_equ
    0b0,                // 062_abrr
    0b0,                // 063_quest
    0b0,                // 064_at
    0b0,                // 091_sbrl
    0b0000000000100100, // 092_bsls
    0b0,                // 093_sbrr
    0b0,                // 094_circ
    0b0000110000000000, // 095_uscr
    0b0000000000100000, // 096_acc
    0b0100100010010010, // 123_cbrl
    0b0000000000010010, // 124_vert
    0b1000010001010010, // 125_cbrr
};

// clang-format off
Font::GlyphTable gt =
{
    // control characters
    &missingGlyph,  // 0
    &missingGlyph,
    &missingGlyph,
    &missingGlyph,
    &missingGlyph,
    &missingGlyph,
    &missingGlyph,
    &missingGlyph,
    &missingGlyph,
    &missingGlyph,
    &missingGlyph,
    &missingGlyph,
    &missingGlyph,
    &missingGlyph,
    &missingGlyph,
    &missingGlyph,
    &missingGlyph,  // 16
    &missingGlyph,
    &missingGlyph,
    &missingGlyph,
    &missingGlyph,
    &missingGlyph,
    &missingGlyph,
    &missingGlyph,
    &missingGlyph,
    &missingGlyph,
    &missingGlyph,
    &missingGlyph,
    &missingGlyph,
    &missingGlyph,
    &missingGlyph,
    &missingGlyph,

    // special characters
    sc+0,           // 32
    sc+1,
    sc+2,
    sc+3,
    sc+4,
    sc+5,
    sc+6,
    sc+7,
    sc+8,
    sc+9,
    sc+10,
    sc+11,
    sc+12,
    sc+13,
    sc+14,
    sc+15,

    // digits
    dg+0,           // 48
    dg+1,
    dg+2,
    dg+3,
    dg+4,
    dg+5,
    dg+6,
    dg+7,
    dg+8,
    dg+9,

    // special characters
    sc+16,
    sc+17,
    sc+18,
    sc+19,
    sc+20,
    sc+21,
    sc+22,          // 64

    // uppercase letters
    uc+0,
    uc+1,
    uc+2,
    uc+3,
    uc+4,
    uc+5,
    uc+6,
    uc+7,
    uc+8,
    uc+9,
    uc+10,
    uc+11,
    uc+12,
    uc+13,
    uc+14,
    uc+15,          // 80
    uc+16,
    uc+17,
    uc+18,
    uc+19,
    uc+20,
    uc+21,
    uc+22,
    uc+23,
    uc+24,
    uc+25,

    // special characters
    sc+23,
    sc+24,
    sc+25,
    sc+26,
    sc+27,
    sc+28,          // 96

    // lowercase letters
    lc+0,
    lc+1,
    lc+2,
    lc+3,
    lc+4,
    lc+5,
    lc+6,
    lc+7,
    lc+8,
    lc+9,
    lc+10,
    lc+11,
    lc+12,
    lc+13,
    lc+14,
    lc+15,          // 112
    lc+16,
    lc+17,
    lc+18,
    lc+19,
    lc+20,
    lc+21,
    lc+22,
    lc+23,
    lc+24,
    lc+25,

    // special characters
    sc+29,
    sc+30,
    sc+31,
    &missingGlyph,
    &missingGlyph,

    // ----- ASCII limit -----  
};

Font font(gt);