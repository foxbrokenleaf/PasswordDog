#ifndef __IST3931_H
#define __IST3931_H

#include <stdint.h>

/* 屏幕分辨率定义 */
#define IST3931_WIDTH           144
#define IST3931_HEIGHT          65
#define IST3931_PAGES           ((IST3931_HEIGHT + 7) / 8)  // 65/8 ≈ 9页
#define IST3931_COL_ADDR_MAX    17  // 144/8 - 1 = 17

/* 字体大小定义 */
#define IST3931_FONT_8X16       0   // 8x16字体
#define IST3931_FONT_16X8       1   // 16x8字体

/* I2C地址定义 */
#define IST3931_SLAVE_ADDR       0x7E        // 从机地址
#define IST3931_CMD_BYTE         0x80        // 命令字节标识
#define IST3931_DATA_BYTE        0xC0        // 数据字节标识

/* 函数声明 */
void IST3931_Init(void);
void IST3931_Update(void);
void IST3931_UpdateArea(uint16_t X, uint16_t Y, uint8_t Width, uint8_t Height);
void IST3931_Clear(void);
void IST3931_ClearArea(uint16_t X, uint16_t Y, uint8_t Width, uint8_t Height);
void ist3931_disp_clear(void);
void IST3931_DisplayOn(void);
void IST3931_DisplayOff(void);
void IST3931_HardwareReset(void);

/* 底层I2C函数 */
void IST3931_WriteCommand(uint8_t Command);
void IST3931_WriteData(uint8_t Data);
void IST3931_WriteDataMulti(uint8_t *Data, uint16_t Count);
void IST3931_SetCursor(uint8_t Y, uint8_t X);

/* 配置函数 */
void IST3931_SetContrast(uint8_t Contrast);
void IST3931_SetBias(uint8_t Bias);
void IST3931_SetNormalMode(void);
void IST3931_SetDisplayMode(uint8_t Mode);

/* 显示函数 */
void IST3931_ShowFont16x8(uint8_t x, uint8_t y, uint8_t num, const uint8_t font[][16]);
void IST3931_ShowChar(uint8_t X, uint8_t Y, char Char, const uint8_t font8x16[][16]);
void IST3931_ShowString(uint8_t X, uint8_t Y, char *String, const uint8_t font8x16[][16]);
void IST3931_ShowImage(uint16_t X, uint16_t Y, uint8_t Width, uint8_t Height, const uint8_t *Image);

/* 绘图函数 */
void IST3931_DrawPoint(uint16_t X, uint16_t Y);
void IST3931_DrawLine(uint16_t X0, uint16_t Y0, uint16_t X1, uint16_t Y1);

/* 隔行扫描映射函数 - 新增 */
uint8_t IST3931_MapCOM(uint8_t physical_y);
uint8_t IST3931_MapPhysicalY(uint8_t logical_y);

/* 延时函数 */
void IST3931_DelayUs(uint32_t us);
void IST3931_DelayMs(uint32_t ms);

#endif