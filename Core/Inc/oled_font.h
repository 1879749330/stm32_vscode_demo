#ifndef __OLED_FONT_H
#define __OLED_FONT_H

#include <stdint.h>

/*
 * 8x16 font for SSD1306-style OLED.
 * Indexed by ASCII code minus 0x20 (space).
 * Only characters 0x20..0x7E are included.
 */
extern const uint8_t OLED_F8x16[][16];

/*
 * 16x16 Chinese font for SSD1306-style OLED.
 * Each character is 32 bytes (16x16 bits).
 */
extern const uint8_t OLED_C16x16[][32];

#endif // __OLED_FONT_H
