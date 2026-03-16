#ifndef __OLED_H
#define __OLED_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize the SSD1306-based OLED display (I2C 0x3C).
 *
 * Requires the I2C peripheral to be initialized (MX_I2C1_Init).
 */
void OLED_Init(void);

/**
 * @brief Clear the entire OLED screen.
 */
void OLED_Clear(void);

/**
 * @brief Show an ASCII character on the OLED.
 *
 * @param line   1..4 (each 16 pixels high)
 * @param column 1..16 (each 8 pixels wide)
 * @param ch     ASCII character to display
 */
void OLED_ShowChar(uint8_t line, uint8_t column, char ch);

/**
 * @brief Show a null-terminated string on the OLED.
 *
 * @param line   1..4
 * @param column 1..16
 * @param str    Null-terminated string (ASCII)
 */
void OLED_ShowString(uint8_t line, uint8_t column, const char *str);

/**
 * @brief Show an unsigned decimal number with fixed width.
 */
void OLED_ShowNum(uint8_t line, uint8_t column, uint32_t number, uint8_t length);

/**
 * @brief Show a signed decimal number with fixed width (includes + or -).
 */
void OLED_ShowSignedNum(uint8_t line, uint8_t column, int32_t number, uint8_t length);

/**
 * @brief Show an unsigned hexadecimal number with fixed width.
 */
void OLED_ShowHexNum(uint8_t line, uint8_t column, uint32_t number, uint8_t length);

/**
 * @brief Show an unsigned binary number with fixed width.
 */
void OLED_ShowBinNum(uint8_t line, uint8_t column, uint32_t number, uint8_t length);

/**
 * @brief Show a 16x16 Chinese character on the OLED.
 *
 * @param line   1..4 (each 16 pixels high)
 * @param column 1..8 (each 16 pixels wide)
 * @param index  index of Chinese character in the font table
 */
void OLED_ShowChineseChar(uint8_t line, uint8_t column, uint8_t index);

/**
 * @brief Show a string of Chinese characters (16x16 each).
 *
 * @param line   1..4
 * @param column starting column 1..8
 * @param str    null-terminated string using Chinese index mapping
 */
void OLED_ShowChinese(uint8_t line, uint8_t column, const char *str);

#ifdef __cplusplus
}
#endif

#endif // __OLED_H
