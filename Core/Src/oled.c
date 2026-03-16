#include "oled.h"
#include "oled_font.h"
#include "i2c.h"
#include "stm32f1xx_hal.h"

// SSD1306 I2C地址定义
// 7位地址0x3C，左移1位变为8位地址供HAL使用
#define OLED_I2C_ADDRESS    (0x3C << 1)   // OLED设备I2C地址
#define OLED_CONTROL_CMD    0x00          // 后续为命令字节
#define OLED_CONTROL_DATA   0x40          // 后续为数据字节

// 内部函数声明
static void OLED_WriteCommand(uint8_t command);
static void OLED_WriteData(uint8_t data);
static void OLED_SetCursor(uint8_t row, uint8_t col);
static uint32_t OLED_Pow(uint32_t x, uint32_t y);

/**
 * @brief  OLED显示屏初始化
 * @note   需要先调用MX_I2C1_Init()初始化I2C外设
 * @retval 无
 */
void OLED_Init(void)
{
    // 等待显示屏上电稳定
    HAL_Delay(50);

    // SSD1306标准初始化序列
    OLED_WriteCommand(0xAE); // 关闭显示屏

    OLED_WriteCommand(0xD5); // 设置显示时钟分频比/振荡器频率
    OLED_WriteCommand(0x80); // 默认值

    OLED_WriteCommand(0xA8); // 设置多路复用比
    OLED_WriteCommand(0x3F); // 1/64 duty

    OLED_WriteCommand(0xD3); // 设置显示偏移
    OLED_WriteCommand(0x00); // 无偏移

    OLED_WriteCommand(0x40); // 设置显示起始行地址为0

    OLED_WriteCommand(0xA1); // 设置段重映射，0xA1为正常顺序（列地址0映射到SEG0）
    OLED_WriteCommand(0xC8); // 设置COM输出扫描方向，0xC8为从上到下扫描

    OLED_WriteCommand(0xDA); // 设置COM引脚硬件配置
    OLED_WriteCommand(0x12);

    OLED_WriteCommand(0x81); // 设置对比度控制
    OLED_WriteCommand(0xCF); // 对比度值

    OLED_WriteCommand(0xD9); // 设置预充电周期
    OLED_WriteCommand(0xF1);

    OLED_WriteCommand(0xDB); // 设置VCOMH消电电平
    OLED_WriteCommand(0x30);

    OLED_WriteCommand(0xA4); // 开启RAM内容显示（A5=忽略RAM内容全亮）
    OLED_WriteCommand(0xA6); // 设置正常显示（A7为反色显示）

    OLED_WriteCommand(0x8D); // 开启电荷泵
    OLED_WriteCommand(0x14); // 启用内部电荷泵

    OLED_WriteCommand(0xAF); // 开启显示屏

    OLED_Clear(); // 清屏
}

/**
 * @brief  清除整个OLED显示屏
 * @retval 无
 */
void OLED_Clear(void)
{
    for (uint8_t row = 0; row < 8; row++)
    {
        OLED_SetCursor(row, 0);
        for (uint8_t col = 0; col < 128; col++)
        {
            OLED_WriteData(0x00);
        }
    }
}

#define OLED_FONT_CHAR_COUNT 95u  /* ASCII 0x20..0x7E 共95个可显示字符 */

/**
 * @brief  在显示屏指定位置显示一个ASCII字符
 * @param  line   行号 1~4（每行高16像素）
 * @param  column 列号 1~16（每列宽8像素）
 * @param  ch     要显示的ASCII字符
 * @retval 无
 */
void OLED_ShowChar(uint8_t line, uint8_t column, char ch)
{
    // 参数边界检查
    if (line < 1 || line > 4 || column < 1 || column > 16)
        return;

    uint8_t index = (uint8_t)ch;
    // 只支持空格到波浪号之间的可显示字符
    if (index < ' ' || index > '~')
        index = ' ';
    index -= ' ';

    if (index >= OLED_FONT_CHAR_COUNT)
        index = 0; // 超出范围显示空格

    // 每个字符8x16像素，占用2个PAGE行
    OLED_SetCursor((line - 1) * 2, (column - 1) * 8);
    for (uint8_t i = 0; i < 8; i++)
    {
        OLED_WriteData(OLED_F8x16[index][i]);
    }

    OLED_SetCursor((line - 1) * 2 + 1, (column - 1) * 8);
    for (uint8_t i = 0; i < 8; i++)
    {
        OLED_WriteData(OLED_F8x16[index][i + 8]);
    }
}

/**
 * @brief  在显示屏指定位置显示一个以NULL结尾的字符串
 * @param  line   行号 1~4
 * @param  column 起始列号 1~16
 * @param  str    要显示的字符串指针（必须以NULL结尾）
 * @retval 无
 */
void OLED_ShowString(uint8_t line, uint8_t column, const char *str)
{
    // 参数合法性检查
    if (line < 1 || line > 4 || column < 1 || column > 16 || str == NULL)
        return;

    uint8_t col = column;
    while (*str && col <= 16)
    {
        OLED_ShowChar(line, col++, *str++);
    }
}

/**
 * @brief  在指定位置显示无符号十进制数字
 * @param  line   行号 1~4
 * @param  column 起始列号 1~16
 * @param  number 要显示的数字
 * @param  length 显示位数（不足补前导零）
 * @retval 无
 */
void OLED_ShowNum(uint8_t line, uint8_t column, uint32_t number, uint8_t length)
{
    for (uint8_t i = 0; i < length; i++)
    {
        uint8_t digit = (number / OLED_Pow(10, length - i - 1)) % 10;
        OLED_ShowChar(line, column + i, '0' + digit);
    }
}

/**
 * @brief  在指定位置显示有符号十进制数字
 * @param  line   行号 1~4
 * @param  column 起始列号 1~16
 * @param  number 要显示的数字（可正可负）
 * @param  length 数字部分的位数
 * @retval 无
 */
void OLED_ShowSignedNum(uint8_t line, uint8_t column, int32_t number, uint8_t length)
{
    if (number >= 0)
    {
        OLED_ShowChar(line, column, '+');
        OLED_ShowNum(line, column + 1, (uint32_t)number, length);
    }
    else
    {
        OLED_ShowChar(line, column, '-');
        OLED_ShowNum(line, column + 1, (uint32_t)(-number), length);
    }
}

/**
 * @brief  在指定位置显示无符号十六进制数字
 * @param  line   行号 1~4
 * @param  column 起始列号 1~16
 * @param  number 要显示的数字
 * @param  length 显示位数
 * @retval 无
 */
void OLED_ShowHexNum(uint8_t line, uint8_t column, uint32_t number, uint8_t length)
{
    for (uint8_t i = 0; i < length; i++)
    {
        uint8_t digit = (number / OLED_Pow(16, length - i - 1)) % 16;
        if (digit < 10)
            OLED_ShowChar(line, column + i, '0' + digit);
        else
            OLED_ShowChar(line, column + i, 'A' + (digit - 10));
    }
}

/**
 * @brief  在指定位置显示无符号二进制数字
 * @param  line   行号 1~4
 * @param  column 起始列号 1~16
 * @param  number 要显示的数字
 * @param  length 显示位数
 * @retval 无
 */
void OLED_ShowBinNum(uint8_t line, uint8_t column, uint32_t number, uint8_t length)
{
    for (uint8_t i = 0; i < length; i++)
    {
        uint8_t bit = (number >> (length - i - 1)) & 1U;
        OLED_ShowChar(line, column + i, '0' + bit);
    }
}

/**
 * @brief  通过I2C向OLED发送一个命令字节
 * @param  command 要发送的命令
 * @retval 无
 */
static void OLED_WriteCommand(uint8_t command)
{
    uint8_t buf[2] = {OLED_CONTROL_CMD, command};
    HAL_I2C_Master_Transmit(&hi2c1, OLED_I2C_ADDRESS, buf, sizeof(buf), 50);
}

/**
 * @brief  通过I2C向OLED发送一个数据字节
 * @param  data 要发送的数据
 * @retval 无
 */
static void OLED_WriteData(uint8_t data)
{
    uint8_t buf[2] = {OLED_CONTROL_DATA, data};
    HAL_I2C_Master_Transmit(&hi2c1, OLED_I2C_ADDRESS, buf, sizeof(buf), 50);
}

/**
 * @brief  设置显示光标位置
 * @param  row 页号（0~7），每页8行像素
 * @param  col 列号（0~127），每列1像素
 * @retval 无
 */
static void OLED_SetCursor(uint8_t row, uint8_t col)
{
    OLED_WriteCommand(0xB0 | (row & 0x07));          // 设置页地址
    OLED_WriteCommand(0x10 | ((col >> 4) & 0x0F));  // 设置列地址高4位
    OLED_WriteCommand(0x00 | (col & 0x0F));          // 设置列地址低4位
}

/**
 * @brief  计算x的y次幂（整数幂）
 * @param  x 底数
 * @param  y 指数
 * @retval x^y
 */
static uint32_t OLED_Pow(uint32_t x, uint32_t y)
{
    uint32_t result = 1;
    while (y--)
        result *= x;
    return result;
}

/**
 * @brief  在指定位置显示一个16x16的中文汉字
 * @param  line   行号 1~4（每行高16像素，刚好容纳一个中文）
 * @param  column 列号 1~8（每个中文宽16像素）
 * @param  index  汉字在字库中的索引
 * @retval 无
 */
void OLED_ShowChineseChar(uint8_t line, uint8_t column, uint8_t index)
{
    if (line < 1 || line > 4 || column < 1 || column > 8)
        return;

    // 汉字16x16，占用2个page行（8+8=16像素高），每像素占1位，共16x16/8=32字节
    for (uint8_t i = 0; i < 2; i++)  // 上半部分和下半部分
    {
        OLED_SetCursor((line - 1) * 2 + i, (column - 1) * 16);
        for (uint8_t j = 0; j < 16; j++)  // 16个字节（每字节一列）
        {
            OLED_WriteData(OLED_C16x16[index][i * 16 + j]);
        }
    }
}

/**
 * @brief  显示一串中文汉字字符串（每个汉字16x16）
 * @param  line   行号 1~4
 * @param  column 起始列号 1~8
 * @param  str    中文字符串，每个汉字对应字库中预先定义的索引顺序
 * @note   字库中已包含的汉字（按索引顺序）：
 *         0:心  1:率  2:血  3:氧  4:温  5:度  6:水  7:位  8:监  9:测
 *         10:系 11:统 12:正 13:常 14:校 15:准 16:多 17:参 18:数 19:信 20:息
 * @retval 无
 */
void OLED_ShowChinese(uint8_t line, uint8_t column, const char *str)
{
    if (line < 1 || line > 4 || column < 1 || column > 8 || str == NULL)
        return;

    uint8_t col = column;
    uint8_t idx = 0;
    // UTF-8中文占3字节，我们每跳过3字节取一个汉字，索引顺序递增
    // 调用者必须保证字符串中的汉字顺序与字库索引顺序一致
    while (*str && col <= 8)
    {
        OLED_ShowChineseChar(line, col, idx);
        idx++;
        str += 3;  // 跳过UTF-8编码的3字节
        col++;
    }
}
