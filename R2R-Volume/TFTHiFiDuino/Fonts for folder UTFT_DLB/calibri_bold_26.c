// This comes with no warranty, implied or otherwise

// This data structure was designed to support Proportional fonts
// on Arduinos. It can however handle any ttf font that has been converted
// using the conversion program. These could be fixed width or proportional 
// fonts. Individual characters do not have to be multiples of 8 bits wide. 
// Any width is fine and does not need to be fixed.

// The data bits are packed to minimize data requirements, but the tradeoff
// is that a header is required per character.

// calibri_bold_26.c
// Point Size   : 26
// Memory usage : 2895 bytes
// # characters : 95

// Header Format (to make Arduino UTFT Compatible):
// ------------------------------------------------
// Character Width (Used as a marker to indicate use this format. i.e.: = 0x00)
// Character Height
// First Character (Reserved. 0x00)
// Number Of Characters (Reserved. 0x00)

#include <avr/pgmspace.h>

uint8_t calibri_bold_26[] PROGMEM = 
{
0x00, 0x18, 0x00, 0x00,

// Individual Character Format:
// ----------------------------
// Character Code
// Adjusted Y Offset
// Width
// Height
// xOffset
// xDelta (the distance to move the cursor. Effective width of the character.)
// Data[n]

// NOTE: You can remove any of these characters if they are not needed in
// your application. The first character number in each Glyph indicates
// the ASCII character code. Therefore, these do not have to be sequential.
// Just remove all the content for a particular character to save space.

// ' '
0x20,0x14,0x00,0x00,0x00,0x06,

// '!'
0x21,0x02,0x05,0x13,0x02,0x08,
0x33,0x9C,0xE7,0x39,0xCE,0x73,0x9C,0xE7,0x00,0x8E,0x73,0x80,
// '"'
0x22,0x02,0x09,0x07,0x01,0x0B,
0x63,0x33,0x99,0xCC,0x66,0x33,0x19,0x8C,
// '#'
0x23,0x03,0x0D,0x12,0x00,0x0D,
0x00,0x00,0xE3,0x07,0x18,0x39,0xC1,0xCE,0x3F,0xFD,0xFF,0xC3,0x1C,0x18,0xC1,0xC6,0x1F,0xF9,0xFF,0xE7,0xFF,0x18,0xE0,0xC7,0x06,0x38,0x31,0x80,0x00,0x00,
// '$'
0x24,0x00,0x0C,0x18,0x00,0x0D,
0x00,0x00,0x30,0x03,0x00,0x30,0x1F,0xE3,0xFE,0x78,0x67,0x80,0x78,0x07,0xC0,0x3F,0x01,0xFC,0x0F,0xE0,0x1F,0x00,0xF0,0x0F,0x40,0xF7,0xFE,0x7F,0xC3,0xF8,0x0C,0x00,0xC0,0x0C,0x00,0x00,
// '%'
0x25,0x03,0x13,0x12,0x00,0x13,
0x00,0x03,0x87,0xC0,0x61,0xDC,0x1C,0x31,0x87,0x06,0x31,0xC0,0xC6,0x30,0x19,0xCE,0x03,0xFB,0x80,0x3E,0xE0,0x00,0x19,0xF0,0x07,0x77,0x01,0xCC,0x60,0x31,0x8C,0x0E,0x31,0x83,0x86,0x70,0xE0,0xFE,0x18,0x0F,0x80,0x00,0x00,
// '&'
0x26,0x02,0x12,0x13,0x00,0x12,
0x00,0x00,0x01,0xFC,0x01,0xFF,0x80,0x79,0xE0,0x1C,0x38,0x07,0x0E,0x01,0xE7,0x80,0x7F,0xC0,0x0F,0xC0,0x07,0xE1,0xC3,0xFC,0x70,0xE7,0x9C,0x78,0xFF,0x1E,0x1F,0x87,0x83,0xE0,0xF1,0xFC,0x3F,0xFF,0x83,0xF8,0xE0,0x00,0x00,
// '''
0x27,0x02,0x04,0x07,0x01,0x06,
0x66,0x66,0x66,0x60,
// '('
0x28,0x01,0x06,0x18,0x01,0x08,
0x00,0x73,0x8E,0x39,0xC7,0x1C,0x71,0xCF,0x3C,0xF3,0xCF,0x1C,0x71,0xC7,0x0E,0x38,0xE1,0xC0,
// ')'
0x29,0x01,0x06,0x18,0x01,0x08,
0x03,0x87,0x1C,0x70,0xE3,0x8E,0x38,0xF3,0xCF,0x3C,0xF3,0xCE,0x38,0xE3,0x9E,0x71,0xCE,0x00,
// '*'
0x2A,0x01,0x0B,0x0B,0x01,0x0D,
0x00,0x01,0xC0,0x30,0x32,0x67,0xFC,0x3E,0x0F,0xC3,0xFE,0x6C,0xC1,0xC0,0x38,0x00,
// '+'
0x2B,0x06,0x0D,0x0D,0x00,0x0D,
0x00,0x00,0x38,0x01,0xC0,0x0E,0x00,0x70,0x3F,0xF9,0xFF,0xCF,0xFE,0x07,0x00,0x38,0x01,0xC0,0x0E,0x00,0x70,0x00,
// ','
0x2C,0x10,0x06,0x08,0x00,0x07,
0x18,0xE3,0x8E,0x39,0xC6,0x10,
// '-'
0x2D,0x0C,0x08,0x03,0x00,0x08,
0x7E,0x7E,0x7E,
// '.'
0x2E,0x10,0x05,0x05,0x01,0x07,
0x73,0x9C,0xE0,0x00,
// '/'
0x2F,0x01,0x0B,0x17,0x00,0x0B,
0x00,0xE0,0x3C,0x07,0x00,0xE0,0x3C,0x07,0x00,0xE0,0x38,0x07,0x00,0xE0,0x38,0x07,0x01,0xE0,0x38,0x07,0x01,0xE0,0x38,0x07,0x01,0xE0,0x38,0x07,0x01,0xC0,0x38,0x00,
// '0'
0x30,0x03,0x0D,0x12,0x00,0x0D,
0x07,0x00,0xFE,0x0F,0xF8,0x71,0xE7,0x87,0x38,0x39,0xC1,0xCE,0x0E,0x70,0x73,0x83,0x9C,0x1C,0xE0,0xE7,0x07,0x3C,0x78,0xF7,0x87,0xFC,0x1F,0xC0,0x00,0x00,
// '1'
0x31,0x03,0x0B,0x11,0x01,0x0D,
0x06,0x01,0xE0,0xFC,0x3F,0x86,0xF0,0x1E,0x03,0xC0,0x78,0x0F,0x01,0xE0,0x3C,0x07,0x80,0xF0,0x1E,0x1F,0xF3,0xFF,0x7F,0xE0,
// '2'
0x32,0x03,0x0C,0x11,0x01,0x0D,
0x0E,0x07,0xF8,0xFF,0xCE,0x7C,0x03,0xC0,0x3C,0x03,0xC0,0x3C,0x07,0x80,0x78,0x0F,0x01,0xE0,0x3C,0x07,0x80,0xFF,0xEF,0xFE,0xFF,0xE0,
// '3'
0x33,0x03,0x0C,0x12,0x01,0x0D,
0x1E,0x07,0xF8,0xFF,0xCC,0x3C,0x01,0xC0,0x1C,0x03,0xC3,0xF8,0x3F,0x03,0xFC,0x03,0xE0,0x1E,0x01,0xE0,0x1E,0xE7,0xCF,0xFC,0x7F,0x00,0x00,
// '4'
0x34,0x03,0x0D,0x12,0x00,0x0D,
0x00,0x00,0x1F,0x01,0xF8,0x0F,0xC0,0xFE,0x0E,0xF0,0x77,0x87,0x3C,0x39,0xE3,0x8F,0x18,0x79,0xFF,0xFF,0xFF,0xBF,0xF8,0x07,0x80,0x3C,0x00,0xE0,0x00,0x00,
// '5'
0x35,0x03,0x0B,0x12,0x01,0x0D,
0x00,0x0F,0xF9,0xFF,0x38,0x07,0x00,0xE0,0x1C,0x03,0xFC,0x7F,0xC0,0xFC,0x07,0x80,0xF0,0x1E,0x03,0xF9,0xF7,0xFE,0x7F,0x00,0x00,
// '6'
0x36,0x03,0x0C,0x12,0x01,0x0D,
0x07,0x81,0xFC,0x3F,0xC7,0x80,0x70,0x07,0x00,0xE0,0x0F,0xFC,0xFF,0xEF,0x1E,0xE0,0xEE,0x0E,0xF0,0xE7,0x0E,0x79,0xE7,0xFC,0x1F,0x80,0x00,
// '7'
0x37,0x03,0x0C,0x12,0x01,0x0D,
0x00,0x0F,0xFE,0xFF,0xE0,0x1E,0x01,0xE0,0x1C,0x03,0xC0,0x38,0x07,0x80,0x70,0x0F,0x00,0xF0,0x1E,0x01,0xE0,0x3C,0x03,0xC0,0x38,0x00,0x00,
// '8'
0x38,0x03,0x0D,0x12,0x00,0x0D,
0x07,0x80,0xFF,0x0F,0xFC,0xF1,0xE7,0x07,0x3C,0x78,0xF3,0x87,0xF8,0x0F,0xC0,0xFF,0x0F,0x7C,0xE0,0xE7,0x07,0x38,0x39,0xE3,0xC7,0xFC,0x1F,0xC0,0x00,0x00,
// '9'
0x39,0x03,0x0D,0x12,0x00,0x0D,
0x07,0x00,0xFE,0x0F,0xF8,0xF1,0xE7,0x0F,0x38,0x39,0xC1,0xCF,0x0E,0x7F,0xF1,0xFF,0x87,0xDC,0x00,0xE0,0x0F,0x00,0x70,0x8F,0x8F,0xF8,0x3F,0x80,0x00,0x00,
// ':'
0x3A,0x08,0x04,0x0D,0x02,0x07,
0xEF,0xF6,0x00,0x00,0x6F,0xFE,0x00,
// ';'
0x3B,0x08,0x06,0x10,0x00,0x07,
0x38,0xF3,0xC6,0x00,0x00,0x00,0x18,0xF3,0xCE,0x39,0xC7,0x18,
// '<'
0x3C,0x06,0x0C,0x0E,0x00,0x0D,
0x00,0x00,0x07,0x01,0xF0,0x7E,0x1F,0x07,0xC0,0x70,0x07,0xC0,0x3F,0x00,0xFC,0x03,0xE0,0x0F,0x00,0x30,0x00,
// '='
0x3D,0x08,0x0B,0x09,0x01,0x0D,
0x00,0x1F,0xFF,0xFF,0x80,0x00,0x00,0x00,0x3F,0xFF,0xFF,0x00,0x00,
// '>'
0x3E,0x06,0x0C,0x0D,0x01,0x0D,
0x00,0x0E,0x00,0xF8,0x07,0xE0,0x1F,0x80,0x3E,0x01,0xE0,0x3E,0x0F,0xC3,0xF0,0xFC,0x0F,0x00,0xC0,0x00,
// '?'
0x3F,0x02,0x0A,0x13,0x01,0x0C,
0x18,0x1F,0xCF,0xF9,0x0F,0x03,0xC0,0x70,0x3C,0x0F,0x1F,0x87,0xC1,0xC0,0x70,0x1C,0x00,0x00,0x80,0x70,0x1C,0x07,0x00,0x00,
// '@'
0x40,0x02,0x16,0x16,0x00,0x17,
0x00,0x00,0x00,0x01,0xFE,0x00,0x1F,0xFE,0x00,0xF8,0xFC,0x07,0x00,0x78,0x38,0x00,0xE1,0xC7,0xB1,0xC7,0x3F,0xC7,0x39,0xC7,0x1C,0xE7,0x1C,0x73,0xB8,0x71,0xCC,0xE1,0xCE,0x33,0x8E,0x38,0xCF,0x7D,0xC3,0x1F,0xFF,0x0E,0x78,0xF0,0x38,0x00,0x00,0xF0,0x00,0x01,0xF0,0x30,0x03,0xFF,0xC0,0x03,0xFE,0x00,0x00,0x00,0x00,
// 'A'
0x41,0x03,0x10,0x12,0x00,0x10,
0x00,0x00,0x07,0xC0,0x07,0xE0,0x07,0xE0,0x0E,0xE0,0x0E,0xF0,0x0E,0x70,0x1C,0x70,0x1C,0x78,0x1C,0x38,0x3C,0x38,0x3F,0xFC,0x3F,0xFC,0x78,0x1C,0x70,0x1E,0x70,0x0E,0xF0,0x0E,0x00,0x00,
// 'B'
0x42,0x03,0x0D,0x11,0x01,0x0F,
0x00,0x03,0xFE,0x1F,0xF8,0xE3,0xE7,0x0F,0x38,0x79,0xC3,0x8F,0xF8,0x7F,0xC3,0xFF,0x9C,0x3C,0xE0,0xF7,0x07,0xB8,0x39,0xFF,0xCF,0xFC,0x7F,0x80,
// 'C'
0x43,0x03,0x0D,0x12,0x01,0x0E,
0x03,0x80,0xFF,0x8F,0xFC,0xF8,0x67,0x80,0x38,0x03,0xC0,0x1E,0x00,0xF0,0x07,0x80,0x3C,0x01,0xE0,0x0F,0x00,0x3C,0x09,0xFF,0xC7,0xFE,0x0F,0xE0,0x00,0x00,
// 'D'
0x44,0x03,0x0F,0x11,0x01,0x10,
0x00,0x00,0xFF,0xC1,0xFF,0xC3,0x87,0xC7,0x03,0xCE,0x07,0x9C,0x07,0x38,0x0E,0x70,0x1C,0xE0,0x39,0xC0,0x73,0x80,0xE7,0x03,0xCE,0x0F,0x1F,0xFE,0x3F,0xF8,0x7F,0x80,
// 'E'
0x45,0x03,0x0B,0x11,0x01,0x0D,
0x00,0x0F,0xF9,0xFF,0x38,0x07,0x00,0xE0,0x1C,0x03,0xFC,0x7F,0xCF,0xF1,0xC0,0x38,0x07,0x00,0xE0,0x1F,0xF3,0xFF,0x7F,0xC0,
// 'F'
0x46,0x03,0x0A,0x12,0x01,0x0C,
0x00,0x1F,0xF7,0xFD,0xC0,0x70,0x1C,0x07,0x01,0xC0,0x7F,0xDF,0xF7,0x01,0xC0,0x70,0x1C,0x07,0x01,0xC0,0x70,0x00,0x00,
// 'G'
0x47,0x03,0x0F,0x12,0x00,0x11,
0x01,0xF0,0x0F,0xF8,0x7F,0xF8,0xF0,0x73,0xC0,0x07,0x00,0x1E,0x00,0x38,0x00,0x70,0xFE,0xE1,0xFD,0xE0,0x7B,0xC0,0x73,0x80,0xE7,0x81,0xC7,0xFF,0x87,0xFF,0x07,0xFC,0x00,0x00,
// 'H'
0x48,0x03,0x0E,0x12,0x01,0x10,
0x00,0x01,0xC0,0xF7,0x03,0xDC,0x0F,0x70,0x3D,0xC0,0xF7,0x03,0xDF,0xFF,0x7F,0xFD,0xFF,0xF7,0x03,0xDC,0x0F,0x70,0x3D,0xC0,0xF7,0x03,0xDC,0x0F,0x70,0x3C,0x00,0x00,
// 'I'
0x49,0x03,0x05,0x12,0x01,0x07,
0x03,0x9C,0xE7,0x39,0xCE,0x73,0x9C,0xE7,0x39,0xCE,0x70,0x00,
// 'J'
0x4A,0x03,0x07,0x12,0x00,0x09,
0x00,0x3C,0x78,0xF1,0xE3,0xC7,0x8F,0x1E,0x3C,0x78,0xF1,0xE3,0xFF,0xFE,0xF8,0x00,
// 'K'
0x4B,0x03,0x0D,0x12,0x01,0x0E,
0x00,0x03,0x83,0x9C,0x3C,0xE3,0xC7,0x1C,0x39,0xC1,0xDE,0x0F,0xE0,0x7F,0x03,0xF8,0x1D,0xE0,0xE7,0x87,0x1C,0x38,0xF1,0xC3,0xCE,0x1E,0x70,0x78,0x00,0x00,
// 'L'
0x4C,0x03,0x0A,0x11,0x01,0x0B,
0x00,0x1C,0x07,0x01,0xC0,0x70,0x1C,0x07,0x01,0xC0,0x70,0x1C,0x07,0x01,0xC0,0x70,0x1C,0x07,0xFD,0xFF,0x7F,0xC0,
// 'M'
0x4D,0x03,0x14,0x12,0x01,0x17,
0x00,0x00,0x07,0xC0,0x1F,0x7E,0x03,0xF7,0xE0,0x3F,0x7F,0x07,0x77,0x70,0x77,0x77,0x07,0x77,0x78,0xE7,0x73,0x8E,0x77,0x39,0xC7,0x71,0xDC,0x77,0x1D,0xC7,0x71,0xF8,0x77,0x0F,0x87,0x70,0xF8,0x77,0x0F,0x07,0x70,0x70,0x70,0x00,0x00,
// 'N'
0x4E,0x03,0x0F,0x12,0x01,0x11,
0x00,0x00,0xF8,0x39,0xF0,0x73,0xF0,0xE7,0xE1,0xCF,0xE3,0x9D,0xC7,0x3B,0xCE,0x73,0x9C,0xE7,0xB9,0xC7,0x73,0x8F,0xE7,0x0F,0xCE,0x1F,0x9C,0x1F,0x38,0x3E,0x70,0x3C,0x00,0x00,
// 'O'
0x4F,0x03,0x10,0x12,0x01,0x12,
0x03,0xC0,0x1F,0xF8,0x3F,0xFC,0x7C,0x3C,0x78,0x1E,0xF0,0x0E,0xF0,0x0E,0xF0,0x0E,0xF0,0x0F,0xF0,0x0F,0xF0,0x0E,0xF0,0x0E,0xF0,0x1E,0x78,0x1E,0x7E,0xFC,0x3F,0xF8,0x0F,0xE0,0x00,0x00,
// 'P'
0x50,0x03,0x0D,0x12,0x01,0x0E,
0x00,0x03,0xFE,0x1F,0xF8,0xE3,0xE7,0x0F,0x38,0x39,0xC3,0xCE,0x1E,0x71,0xE3,0xFF,0x1F,0xE0,0xE0,0x07,0x00,0x38,0x01,0xC0,0x0E,0x00,0x70,0x00,0x00,0x00,
// 'Q'
0x51,0x03,0x12,0x14,0x01,0x12,
0x03,0xC0,0x07,0xFE,0x03,0xFF,0xC1,0xF0,0xF0,0x78,0x1E,0x3C,0x03,0x8F,0x00,0xE3,0xC0,0x38,0xF0,0x0F,0x3C,0x03,0x8F,0x00,0xE3,0xC0,0x38,0xF0,0x1E,0x1E,0x07,0x87,0xEF,0xC0,0xFF,0xF0,0x0F,0xFE,0x00,0x03,0xE0,0x00,0x78,0x00,0x06,
// 'R'
0x52,0x03,0x0D,0x12,0x01,0x0F,
0x00,0x03,0xFE,0x1F,0xF8,0xE1,0xE7,0x0F,0x38,0x39,0xC3,0xCE,0x3C,0x7F,0xE3,0xFC,0x1C,0xF0,0xE3,0xC7,0x0E,0x38,0x79,0xC1,0xCE,0x0F,0x70,0x78,0x00,0x00,
// 'S'
0x53,0x03,0x0C,0x12,0x00,0x0C,
0x07,0x01,0xFC,0x3F,0xE7,0x84,0x78,0x07,0x80,0x7C,0x03,0xF0,0x1F,0xC0,0xFE,0x03,0xE0,0x0E,0x00,0xE4,0x0E,0x7B,0xE7,0xFC,0x3F,0x80,0x00,
// 'T'
0x54,0x03,0x0D,0x12,0x00,0x0D,
0x00,0x07,0xFF,0xFF,0xFE,0x0E,0x00,0x70,0x03,0x80,0x1C,0x00,0xE0,0x07,0x00,0x38,0x01,0xC0,0x0E,0x00,0x70,0x03,0x80,0x1C,0x00,0xE0,0x07,0x00,0x00,0x00,
// 'U'
0x55,0x03,0x0F,0x12,0x01,0x11,
0x00,0x00,0xE0,0x39,0xC0,0x73,0x80,0xE7,0x01,0xCE,0x03,0x9C,0x07,0x38,0x0E,0x70,0x1C,0xE0,0x39,0xC0,0x73,0x80,0xE7,0x01,0xCF,0x07,0x8F,0xBE,0x1F,0xF8,0x0F,0xE0,0x00,0x00,
// 'V'
0x56,0x03,0x10,0x12,0x00,0x0F,
0x00,0x00,0xF0,0x0E,0x70,0x1E,0x78,0x1C,0x78,0x1C,0x38,0x3C,0x3C,0x38,0x3C,0x38,0x1C,0x38,0x1C,0x70,0x1E,0x70,0x0E,0x70,0x0E,0xE0,0x0F,0xE0,0x07,0xE0,0x07,0xC0,0x07,0xC0,0x00,0x00,
// 'W'
0x57,0x03,0x17,0x12,0x00,0x18,
0x00,0x00,0x00,0xE0,0x78,0x1D,0xC0,0xF0,0x7B,0xC3,0xE0,0xE7,0x87,0xE1,0xC7,0x0F,0xC3,0x8E,0x1F,0x87,0x1C,0x77,0x1C,0x3C,0xEF,0x38,0x39,0xCE,0x70,0x73,0x9C,0xE0,0xEE,0x3B,0x81,0xFC,0x7F,0x01,0xF8,0x7E,0x03,0xF0,0xFC,0x07,0xC1,0xF0,0x07,0x81,0xE0,0x00,0x00,0x00,
// 'X'
0x58,0x03,0x0E,0x12,0x00,0x0E,
0x00,0x01,0xE0,0xE7,0x87,0x8F,0x1C,0x1C,0xF0,0x7B,0x80,0xFE,0x03,0xF0,0x07,0xC0,0x3F,0x00,0xFE,0x07,0xF8,0x1C,0xF0,0xF1,0xC7,0x87,0x9E,0x0E,0x70,0x3C,0x00,0x00,
// 'Y'
0x59,0x03,0x0E,0x12,0x00,0x0E,
0x00,0x03,0xC1,0xE7,0x87,0x9E,0x1C,0x38,0xF0,0xF3,0x81,0xDE,0x07,0xF0,0x0F,0xC0,0x3E,0x00,0x78,0x01,0xC0,0x07,0x00,0x1C,0x00,0x70,0x01,0xC0,0x07,0x00,0x00,0x00,
// 'Z'
0x5A,0x03,0x0C,0x11,0x00,0x0C,
0x00,0x07,0xFE,0x7F,0xE0,0x1E,0x01,0xE0,0x3C,0x03,0x80,0x78,0x0F,0x00,0xF0,0x1E,0x01,0xC0,0x3C,0x07,0x80,0x7F,0xE7,0xFF,0x7F,0xF0,
// '['
0x5B,0x02,0x06,0x17,0x02,0x08,
0xFB,0xEE,0x38,0xE3,0x8E,0x38,0xE3,0x8E,0x38,0xE3,0x8E,0x38,0xE3,0x8E,0x38,0xFB,0xE0,0x00,
// '\'
0x5C,0x01,0x0C,0x17,0x00,0x0B,
0x60,0x07,0x00,0x70,0x07,0x00,0x38,0x03,0x80,0x3C,0x01,0xC0,0x1C,0x01,0xE0,0x0E,0x00,0xE0,0x0F,0x00,0x70,0x07,0x00,0x38,0x03,0x80,0x38,0x01,0xC0,0x1C,0x01,0xE0,0x0E,0x00,0xE0,
// ']'
0x5D,0x02,0x06,0x17,0x01,0x08,
0xFB,0xE3,0x8E,0x38,0xE3,0x8E,0x38,0xE3,0x8E,0x38,0xE3,0x8E,0x38,0xE3,0x8E,0xFB,0xE0,0x00,
// '^'
0x5E,0x03,0x0D,0x0B,0x00,0x0D,
0x02,0x00,0x7C,0x03,0xE0,0x3F,0x81,0xDC,0x1C,0xF0,0xE3,0x8F,0x1C,0x70,0x73,0x83,0x90,0x00,
// '_'
0x5F,0x16,0x0F,0x03,0xFF,0x0D,
0x7F,0xFC,0xFF,0xF9,0xFF,0xF0,
// '`'
0x60,0x02,0x07,0x06,0x00,0x08,
0x30,0x70,0x70,0x60,0x00,0x00,
// 'a'
0x61,0x07,0x0C,0x0E,0x00,0x0D,
0x00,0x03,0xFC,0x3F,0xE3,0x0E,0x00,0xE0,0x0E,0x1F,0xE3,0xFE,0x70,0xE7,0x0E,0x71,0xE7,0xFE,0x3F,0x60,0x00,
// 'b'
0x62,0x02,0x0C,0x13,0x01,0x0E,
0x60,0x07,0x00,0x70,0x07,0x00,0x70,0x07,0x00,0x77,0xC7,0xFE,0x7D,0xE7,0x0F,0x70,0x77,0x07,0x70,0x77,0x0F,0x70,0xF7,0xDE,0x7F,0xE6,0x7C,0x00,0x00,
// 'c'
0x63,0x07,0x0B,0x0E,0x00,0x0B,
0x00,0x03,0xF8,0xFF,0x1E,0x67,0x80,0xE0,0x1C,0x03,0x80,0x70,0x0F,0x00,0xF7,0x1F,0xE1,0xF8,0x00,0x00,
// 'd'
0x64,0x02,0x0D,0x13,0x00,0x0E,
0x00,0x30,0x03,0x80,0x1C,0x00,0xE0,0x07,0x00,0x38,0x7F,0xC7,0xFE,0x3D,0xF3,0xC3,0x9C,0x1C,0xE0,0xE7,0x07,0x38,0x39,0xE1,0xC7,0xBE,0x3F,0xF0,0xF9,0x80,0x00,
// 'e'
0x65,0x07,0x0D,0x0E,0x00,0x0D,
0x00,0x00,0xFE,0x0F,0xF8,0x71,0xE7,0x07,0x3C,0x39,0xFF,0xCF,0xFE,0x70,0x03,0xC0,0x0F,0x08,0x7F,0xE1,0xFE,0x00,0x00,
// 'f'
0x66,0x02,0x09,0x13,0x00,0x08,
0x07,0x0F,0x87,0xC7,0x83,0x81,0xE3,0xFD,0xFE,0x3C,0x1C,0x0E,0x07,0x03,0x81,0xC0,0xE0,0x70,0x38,0x1C,0x00,0x00,
// 'g'
0x67,0x07,0x0C,0x12,0x00,0x0C,
0x00,0x01,0xFF,0x3F,0xF7,0x1C,0x70,0xC7,0x0C,0x7B,0xC3,0xF8,0x7F,0x07,0x00,0x7F,0x83,0xFE,0x7F,0xE7,0x0F,0xF0,0xF7,0xFE,0x7F,0xC0,0xE0,
// 'h'
0x68,0x02,0x0C,0x13,0x01,0x0E,
0x60,0x07,0x00,0x70,0x07,0x00,0x70,0x07,0x00,0x77,0xC7,0xFE,0x7D,0xE7,0x0E,0x70,0xE7,0x0E,0x70,0xE7,0x0E,0x70,0xE7,0x0E,0x70,0xE7,0x0E,0x00,0x00,
// 'i'
0x69,0x02,0x05,0x13,0x01,0x06,
0x03,0xBC,0xE0,0x01,0xCE,0x73,0x9C,0xE7,0x39,0xCE,0x73,0x80,
// 'j'
0x6A,0x02,0x07,0x17,0xFF,0x07,
0x00,0x38,0x70,0xE0,0x00,0x07,0x0E,0x1C,0x38,0x70,0xE1,0xC3,0x87,0x0E,0x1C,0x38,0x70,0xE7,0xCF,0x18,0x00,
// 'k'
0x6B,0x02,0x0C,0x13,0x01,0x0C,
0x60,0x07,0x00,0x70,0x07,0x00,0x70,0x07,0x00,0x71,0xE7,0x3C,0x73,0x87,0x70,0x7E,0x07,0xE0,0x7F,0x07,0x78,0x73,0x87,0x3C,0x71,0xE7,0x0E,0x00,0x00,
// 'l'
0x6C,0x02,0x04,0x13,0x01,0x06,
0x67,0x77,0x77,0x77,0x77,0x77,0x77,0x77,0x77,0x00,
// 'm'
0x6D,0x07,0x13,0x0E,0x01,0x15,
0x00,0x00,0x0C,0xF9,0xF1,0xFF,0xFF,0x3E,0xFD,0xE7,0x0F,0x1E,0xE1,0xC3,0xDC,0x38,0x7B,0x87,0x0F,0x70,0xE1,0xEE,0x1C,0x3D,0xC3,0x87,0xB8,0x70,0xF7,0x0E,0x1E,0x00,0x00,0x00,
// 'n'
0x6E,0x07,0x0C,0x0E,0x01,0x0E,
0x00,0x06,0x7C,0x7F,0xE7,0xDE,0x70,0xE7,0x0E,0x70,0xE7,0x0E,0x70,0xE7,0x0E,0x70,0xE7,0x0E,0x70,0xE0,0x00,
// 'o'
0x6F,0x07,0x0E,0x0E,0x00,0x0E,
0x00,0x00,0x7F,0x83,0xFF,0x0F,0x1C,0x78,0x79,0xC0,0xE7,0x03,0x9C,0x0E,0x70,0x39,0xE1,0xE3,0xCF,0x0F,0xF8,0x1F,0xC0,0x00,0x00,
// 'p'
0x70,0x07,0x0C,0x12,0x01,0x0E,
0x00,0x06,0x7C,0x7F,0xE7,0xDE,0x70,0xF7,0x07,0x70,0x77,0x07,0x70,0xF7,0x0F,0x7D,0xE7,0xFE,0x77,0xC7,0x00,0x70,0x07,0x00,0x70,0x06,0x00,
// 'q'
0x71,0x07,0x0D,0x12,0x00,0x0E,
0x00,0x00,0xF9,0x8F,0xFC,0x7B,0xE7,0x87,0x38,0x39,0xC1,0xCE,0x0E,0x70,0x73,0xC3,0x8F,0x7C,0x7F,0xE1,0xF7,0x00,0x38,0x01,0xC0,0x0E,0x00,0x70,0x01,0x80,
// 'r'
0x72,0x07,0x08,0x0E,0x01,0x09,
0x00,0x6F,0x7F,0x7F,0x78,0x70,0x70,0x70,0x70,0x70,0x70,0x70,0x70,0x00,
// 's'
0x73,0x07,0x0A,0x0E,0x00,0x0A,
0x00,0x0F,0xE7,0xF9,0xC0,0x70,0x1F,0x03,0xF0,0x7E,0x03,0x80,0xF6,0x39,0xFE,0x7F,0x00,0x00,
// 't'
0x74,0x04,0x09,0x11,0x00,0x09,
0x00,0x1C,0x0E,0x07,0x0F,0xF7,0xF8,0xF0,0x70,0x38,0x1C,0x0E,0x07,0x03,0x81,0xE8,0x7C,0x3E,0x00,0x00,
// 'u'
0x75,0x07,0x0C,0x0E,0x01,0x0E,
0x00,0x07,0x0E,0x70,0xE7,0x0E,0x70,0xE7,0x0E,0x70,0xE7,0x0E,0x70,0xE7,0x1E,0x7F,0xE7,0xFE,0x3E,0x60,0x00,
// 'v'
0x76,0x07,0x0D,0x0E,0x00,0x0C,
0x00,0x07,0x83,0x9C,0x3C,0xE1,0xC7,0x8E,0x1C,0x70,0xE7,0x07,0xB8,0x1D,0xC0,0xFC,0x07,0xE0,0x1F,0x00,0xF0,0x00,0x00,
// 'w'
0x77,0x07,0x13,0x0E,0x00,0x13,
0x00,0x00,0x1E,0x1C,0x1D,0xC3,0xC7,0xB8,0x78,0xE7,0x9F,0x1C,0x73,0xE3,0x8E,0x6E,0xE1,0xCD,0xDC,0x3F,0xBB,0x83,0xF3,0xF0,0x7C,0x7C,0x0F,0x8F,0x80,0xF0,0xF0,0x00,0x00,0x00,
// 'x'
0x78,0x07,0x0C,0x0E,0x00,0x0C,
0x00,0x07,0x0E,0x79,0xE3,0x9C,0x3F,0xC1,0xF8,0x1F,0x01,0xF8,0x1F,0x83,0xBC,0x39,0xC7,0x1E,0x70,0xE0,0x00,
// 'y'
0x79,0x07,0x0D,0x12,0x00,0x0C,
0x00,0x07,0x83,0x9C,0x3C,0xE1,0xC7,0x8E,0x1C,0x70,0xE7,0x07,0xB8,0x1F,0xC0,0xFC,0x03,0xE0,0x1F,0x00,0xF0,0x03,0x80,0x3C,0x01,0xC0,0x1E,0x00,0x60,0x00,
// 'z'
0x7A,0x07,0x0A,0x0D,0x00,0x0A,
0x00,0x1F,0xE7,0xF8,0x1E,0x07,0x03,0xC0,0xE0,0x70,0x3C,0x0E,0x07,0xF9,0xFF,0x7F,0x80,
// '{'
0x7B,0x01,0x09,0x18,0x00,0x09,
0x00,0x07,0x87,0xC3,0x81,0xC0,0xE0,0x70,0x38,0x1C,0x0E,0x1E,0x0E,0x07,0x81,0xC0,0x70,0x38,0x1C,0x0E,0x07,0x03,0x81,0xC0,0xF8,0x3C,0x00,
// '|'
0x7C,0x01,0x04,0x18,0x04,0x0C,
0x07,0x77,0x77,0x77,0x77,0x77,0x77,0x77,0x77,0x77,0x77,0x76,
// '}'
0x7D,0x01,0x08,0x18,0x01,0x09,
0x00,0xF0,0xF8,0x38,0x38,0x38,0x38,0x38,0x38,0x1C,0x1E,0x0F,0x0F,0x1C,0x1C,0x18,0x38,0x38,0x38,0x38,0x38,0xF8,0xF0,0x00,
// '~'
0x7E,0x05,0x0D,0x06,0x00,0x0D,
0x1C,0x03,0xF1,0x9F,0xDC,0xCF,0xE6,0x3E,0x00,0x00,

// Terminator
0xFF
};