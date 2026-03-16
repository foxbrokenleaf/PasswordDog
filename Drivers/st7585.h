#ifndef __ST7585_H
#define __ST7585_H

#include <stdint.h>

/* 屏幕分辨率定义 */
#define ST7585_WIDTH            64
#define ST7585_HEIGHT           32
#define ST7585_PAGES            (ST7585_HEIGHT / 8)  // 32/8 = 4页

/* 字体大小定义 */
#define ST7585_8X16             8
#define ST7585_6X8              6

/* 填充模式定义 */
#define ST7585_UNFILLED         0
#define ST7585_FILLED           1

/* I2C地址定义 (ST7585的I2C地址通常为0x7A或0x7C，根据SA0引脚决定) */
#define ST7585_ADDR             0x7A        // SA0接地时
// #define ST7585_ADDR           0x7C        // SA0接VCC时

/* ST7585命令集 - 基于数据手册 */
#define ST7585_CMD_SET_PAGE_ADDR           0xB0  // 设置页地址 (0xB0-0xB7)
#define ST7585_CMD_SET_COLUMN_ADDR_MSB     0x10  // 设置列地址高4位
#define ST7585_CMD_SET_COLUMN_ADDR_LSB     0x00  // 设置列地址低4位

#define ST7585_CMD_DISPLAY_ON               0xAF
#define ST7585_CMD_DISPLAY_OFF              0xAE
#define ST7585_CMD_DISPLAY_INVERSE          0xA7
#define ST7585_CMD_DISPLAY_NORMAL           0xA6
#define ST7585_CMD_DISPLAY_ALL_POINTS_ON    0xA5
#define ST7585_CMD_DISPLAY_ALL_POINTS_OFF   0xA4

#define ST7585_CMD_SET_BIAS_1_9              0xA2  // 1/9偏压
#define ST7585_CMD_SET_BIAS_1_7              0xA3  // 1/7偏压

#define ST7585_CMD_SET_POWER_CTRL             0x28  // 电源控制 (0x28-0x2F)
#define ST7585_CMD_SET_BOOSTER_RATIO          0xF8  // 升压比设置
#define ST7585_CMD_SET_REGULATOR_RATIO        0x20  // 稳压器设置

#define ST7585_CMD_SET_VOLTAGE_DOUBLER_ENABLE 0x2C
#define ST7585_CMD_SET_VOLTAGE_DOUBLER_DISABLE 0x24

#define ST7585_CMD_SET_ELECTRONIC_VOLUME       0x81  // 电子对比度控制 (32级)
#define ST7585_CMD_SET_PUMP_CONTROL            0x8D

#define ST7585_CMD_SET_DCDC_CONTROL             0x30
#define ST7585_CMD_SET_DISPLAY_START_LINE       0x40
#define ST7585_CMD_SET_COM_SCAN_DIRECTION       0xC0  // COM扫描方向 (C0正常, C8反向)
#define ST7585_CMD_SET_SEG_SCAN_DIRECTION       0xA0  // SEG扫描方向 (A0正常, A1反向)

#define ST7585_CMD_SET_RESISTOR_RATIO           0x23  // 电阻率设置
#define ST7585_CMD_SET_POWER_SAVE               0xAC  // 省电模式

#define ST7585_CMD_NOP                         0xE3  // 空操作

#define ST7585_CMD_WRITE_RAM                    0x5C  // 写RAM数据
#define ST7585_CMD_READ_RAM                     0x5D  // 读RAM数据

#define ST7585_CMD_ENTER_SLEEP_MODE             0xAE
#define ST7585_CMD_EXIT_SLEEP_MODE              0xAF

#define ST7585_CMD_SET_COLUMN_ADDR               0x15  // 设置列地址范围
#define ST7585_CMD_SET_ROW_ADDR                   0x75  // 设置行地址范围

/* 函数声明 */
void ST7585_Init(void);
void ST7585_Update(void);
void ST7585_UpdateArea(int16_t X, int16_t Y, uint8_t Width, uint8_t Height);

void ST7585_Clear(void);
void ST7585_ClearArea(int16_t X, int16_t Y, uint8_t Width, uint8_t Height);
void ST7585_Reverse(void);
void ST7585_ReverseArea(int16_t X, int16_t Y, uint8_t Width, uint8_t Height);

void ST7585_ShowChar(int16_t X, int16_t Y, char Char, uint8_t FontSize);
void ST7585_ShowString(int16_t X, int16_t Y, char *String, uint8_t FontSize);
void ST7585_ShowNum(int16_t X, int16_t Y, uint32_t Number, uint8_t Length, uint8_t FontSize);
void ST7585_ShowSignedNum(int16_t X, int16_t Y, int32_t Number, uint8_t Length, uint8_t FontSize);
void ST7585_ShowHexNum(int16_t X, int16_t Y, uint32_t Number, uint8_t Length, uint8_t FontSize);
void ST7585_ShowBinNum(int16_t X, int16_t Y, uint32_t Number, uint8_t Length, uint8_t FontSize);
void ST7585_ShowImage(int16_t X, int16_t Y, uint8_t Width, uint8_t Height, const uint8_t *Image);
void ST7585_Printf(int16_t X, int16_t Y, uint8_t FontSize, char *format, ...);

void ST7585_DrawPoint(int16_t X, int16_t Y);
uint8_t ST7585_GetPoint(int16_t X, int16_t Y);
void ST7585_DrawLine(int16_t X0, int16_t Y0, int16_t X1, int16_t Y1);
void ST7585_DrawRectangle(int16_t X, int16_t Y, uint8_t Width, uint8_t Height, uint8_t IsFilled);
void ST7585_DrawCircle(int16_t X, int16_t Y, uint8_t Radius, uint8_t IsFilled);

void ST7585_SetContrast(uint8_t Contrast);  // 32级对比度 (0-31)
void ST7585_DisplayOn(void);
void ST7585_DisplayOff(void);
void ST7585_SetSleepMode(uint8_t Enable);
void ST7585_SetBias(uint8_t Bias_1_9);  // 1: 1/9偏压, 0: 1/7偏压
void ST7585_SetDirection(uint8_t ComDir, uint8_t SegDir);  // 设置扫描方向

#endif