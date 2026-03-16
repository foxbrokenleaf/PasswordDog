/***************************************************************************************
  * 程序名称：				0.69寸LCD显示屏驱动程序（4针脚I2C接口）
  * 屏幕分辨率：				64x32
  * 驱动芯片：				ST7585
  * 引脚数量：               14Pin
  * 程序创建时间：			2025.03.16
  ***************************************************************************************
  */

#include "stm32f10x.h"
#include "st7585.h"
#include "st7585_font.h"
#include <string.h>
#include <stdio.h>
#include <stdarg.h>

/**
  * 数据存储格式：
  * 纵向8点，高位在下，先从左到右，再从上到下
  * 每一个Bit对应一个像素点
  * 
  * 坐标轴定义：
  * 左上角为(0, 0)点
  * 横向向右为X轴，取值范围：0~63
  * 纵向向下为Y轴，取值范围：0~31
  * 
  *       0             X轴           63 
  *      .--------------------------->
  *    0 |
  *      |
  *  Y轴 |
  *      |
  *   31 |
  *      v
  * 
  */

/* 全局变量 */
/**
  * LCD显存数组
  * 所有的显示函数，都只是对此显存数组进行读写
  * 随后调用ST7585_Update函数或ST7585_UpdateArea函数
  * 才会将显存数组的数据发送到LCD硬件，进行显示
  * 
  * 64x32分辨率：
  * Y方向32像素需要4页（每页8像素）
  * X方向64列
  */
uint8_t ST7585_DisplayBuf[4][64];  // [页][列]，4页 * 64列

/* GPIO引脚定义 - 根据您的实际硬件连接修改 */
#define ST7585_SCL_PORT GPIOB
#define ST7585_SCL_PIN GPIO_Pin_6
#define ST7585_SDA_PORT GPIOB
#define ST7585_SDA_PIN GPIO_Pin_7
#define ST7585_RES_PORT GPIOB
#define ST7585_RES_PIN GPIO_Pin_8   // 复位引脚

/* 引脚配置 */

/**
  * 函    数：ST7585写SCL高低电平
  * 参    数：BitValue 要写入SCL的电平值，范围：0/1
  * 返 回 值：无
  */
void ST7585_W_SCL(uint8_t BitValue)
{
    GPIO_WriteBit(ST7585_SCL_PORT, ST7585_SCL_PIN, (BitAction)BitValue);
}

/**
  * 函    数：ST7585写SDA高低电平
  * 参    数：BitValue 要写入SDA的电平值，范围：0/1
  * 返 回 值：无
  */
void ST7585_W_SDA(uint8_t BitValue)
{
    GPIO_WriteBit(ST7585_SDA_PORT, ST7585_SDA_PIN, (BitAction)BitValue);
}

/**
  * 函    数：ST7585写RES高低电平（复位引脚）
  * 参    数：BitValue 要写入RES的电平值，范围：0/1
  * 返 回 值：无
  */
void ST7585_W_RES(uint8_t BitValue)
{
    GPIO_WriteBit(ST7585_RES_PORT, ST7585_RES_PIN, (BitAction)BitValue);
}

/**
  * 函    数：ST7585引脚初始化
  * 参    数：无
  * 返 回 值：无
  */
 void ST7585_GPIO_Init(void){
    
    uint32_t i, j;
    GPIO_InitTypeDef tmp;
    
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    
    tmp.GPIO_Mode = GPIO_Mode_Out_PP;
    tmp.GPIO_Pin = ST7585_SCL_PIN;
    tmp.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_Init(ST7585_SCL_PORT, &tmp);
    
    tmp.GPIO_Mode = GPIO_Mode_Out_PP;
    tmp.GPIO_Pin = ST7585_SDA_PIN;
    tmp.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_Init(ST7585_SDA_PORT, &tmp);
    
    tmp.GPIO_Mode = GPIO_Mode_Out_PP;
    tmp.GPIO_Pin = ST7585_RES_PIN;
    tmp.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_Init(ST7585_RES_PORT, &tmp);
    
    /* 在初始化前，加入适量延时，待LCD供电稳定 */
    for (i = 0; i < 1000; i ++)
    {
        for (j = 0; j < 1000; j ++);
    }
    
    /* 释放SCL、SDA和RES */
    ST7585_W_SCL(1);
    ST7585_W_SDA(1);
    ST7585_W_RES(1);
}

/* 通信协议 */

/**
  * 函    数：I2C起始
  * 参    数：无
  * 返 回 值：无
  */
void ST7585_I2C_Start(void)
{
    ST7585_W_SDA(1);
    ST7585_W_SCL(1);
    ST7585_W_SDA(0);
    ST7585_W_SCL(0);
}

/**
  * 函    数：I2C终止
  * 参    数：无
  * 返 回 值：无
  */
void ST7585_I2C_Stop(void)
{
    ST7585_W_SDA(0);
    ST7585_W_SCL(1);
    ST7585_W_SDA(1);
}

/**
  * 函    数：I2C发送一个字节
  * 参    数：Byte 要发送的一个字节数据，范围：0x00~0xFF
  * 返 回 值：无
  */
void ST7585_I2C_SendByte(uint8_t Byte)
{
    uint8_t i;
    
    for (i = 0; i < 8; i++)
    {
        ST7585_W_SDA(!!(Byte & (0x80 >> i)));
        ST7585_W_SCL(1);
        ST7585_W_SCL(0);
    }
    
    /* 等待应答 */
    ST7585_W_SCL(1);
    ST7585_W_SCL(0);
}

/**
  * 函    数：ST7585写命令
  * 参    数：Command 要写入的命令值，范围：0x00~0xFF
  * 返 回 值：无
  */
void ST7585_WriteCommand(uint8_t Command)
{
    ST7585_I2C_Start();
    ST7585_I2C_SendByte(ST7585_ADDR);
    ST7585_I2C_SendByte(0x00);      // 控制字节：后续为命令
    ST7585_I2C_SendByte(Command);
    ST7585_I2C_Stop();
}

/**
  * 函    数：ST7585写数据
  * 参    数：Data 要写入数据的起始地址
  * 参    数：Count 要写入数据的数量
  * 返 回 值：无
  */
void ST7585_WriteData(uint8_t *Data, uint8_t Count)
{
    uint8_t i;
    
    ST7585_I2C_Start();
    ST7585_I2C_SendByte(ST7585_ADDR);
    ST7585_I2C_SendByte(0x40);      // 控制字节：后续为数据
    for (i = 0; i < Count; i ++)
    {
        ST7585_I2C_SendByte(Data[i]);
    }
    ST7585_I2C_Stop();
}

/**
  * 函    数：硬件复位
  * 参    数：无
  * 返 回 值：无
  */
void ST7585_HardwareReset(void)
{
    ST7585_W_RES(0);
    for (uint32_t i = 0; i < 10000; i++);  // 延时约10ms
    ST7585_W_RES(1);
    for (uint32_t i = 0; i < 10000; i++);  // 延时约10ms
}

/* 硬件配置 */

/**
  * 函    数：ST7585初始化
  * 参    数：无
  * 返 回 值：无
  * 说    明：使用前，需要调用此初始化函数
  */
void ST7585_Init(void)
{
    ST7585_GPIO_Init();
    
    /* 硬件复位 */
    ST7585_HardwareReset();
    
    /* ST7585初始化序列（适用于64x32）基于数据手册 */
    ST7585_WriteCommand(ST7585_CMD_DISPLAY_OFF);        // 关闭显示 (0xAE)
    
    /* 设置偏压 - 1/9偏压适合高电压操作 [citation:1]*/
    ST7585_WriteCommand(ST7585_CMD_SET_BIAS_1_9);       // 0xA2
    
    /* 设置扫描方向 */
    ST7585_WriteCommand(ST7585_CMD_SET_COM_SCAN_DIRECTION); // 0xC0 (正常)
    ST7585_WriteCommand(ST7585_CMD_SET_SEG_SCAN_DIRECTION); // 0xA0 (正常)
    
    /* 设置升压电路 [citation:1]*/
    ST7585_WriteCommand(ST7585_CMD_SET_BOOSTER_RATIO);  // 0xF8
    ST7585_WriteCommand(0x00);                           // 升压比设置
    
    /* 设置电源控制 - 打开升压器、稳压器和跟随器 [citation:1]*/
    ST7585_WriteCommand(ST7585_CMD_SET_POWER_CTRL | 0x07); // 0x2F
    
    /* 等待电源稳定 */
    for (uint32_t i = 0; i < 50000; i++);
    
    /* 设置稳压器比例 */
    ST7585_WriteCommand(ST7585_CMD_SET_REGULATOR_RATIO); // 0x20
    ST7585_WriteCommand(0x3B);                           // 建议值
    
    /* 设置电子对比度 (32级可调) [citation:1]*/
    ST7585_WriteCommand(ST7585_CMD_SET_ELECTRONIC_VOLUME); // 0x81
    ST7585_WriteCommand(0x1A);                            // 对比度值 (0-31)
    
    /* 设置显示起始行 */
    ST7585_WriteCommand(ST7585_CMD_SET_DISPLAY_START_LINE); // 0x40
    
    /* 设置显示模式 - 正常显示 */
    ST7585_WriteCommand(ST7585_CMD_DISPLAY_NORMAL);      // 0xA6
    
    /* 设置列地址范围 - 适配64列 */
    ST7585_WriteCommand(ST7585_CMD_SET_COLUMN_ADDR);     // 0x15
    ST7585_WriteCommand(0x00);                            // 列起始 (0)
    ST7585_WriteCommand(0x3F);                            // 列结束 (63)
    
    /* 设置行地址范围 - 适配32行，4页 */
    ST7585_WriteCommand(ST7585_CMD_SET_ROW_ADDR);        // 0x75
    ST7585_WriteCommand(0x00);                            // 行起始 (0)
    ST7585_WriteCommand(0x1F);                            // 行结束 (31)
    
    /* 开启显示 */
    ST7585_WriteCommand(ST7585_CMD_DISPLAY_ON);           // 0xAF
    
    ST7585_Clear();               // 清空显存数组
    ST7585_Update();              // 更新显示
}

/**
  * 函    数：ST7585设置显示光标位置
  * 参    数：Page 指定光标所在的页，范围：0~3 (因为32像素 = 4页)
  * 参    数：X 指定光标所在的X轴坐标，范围：0~63
  * 返 回 值：无
  */
void ST7585_SetCursor(uint8_t Page, uint8_t X)
{
    /* 64x32分辨率：页0-3，X坐标0-63 */
    if (Page > 3) Page = 3;
    if (X > 63) X = 63;
    
    /* ST7585使用页地址和列地址设置 */
    ST7585_WriteCommand(ST7585_CMD_SET_PAGE_ADDR | Page);  // 设置页地址 (0xB0-0xB3)
    ST7585_WriteCommand(ST7585_CMD_SET_COLUMN_ADDR_MSB | ((X >> 4) & 0x0F)); // 列高4位
    ST7585_WriteCommand(ST7585_CMD_SET_COLUMN_ADDR_LSB | (X & 0x0F));        // 列低4位
}

/* 工具函数 */

/**
  * 函    数：次方函数
  * 参    数：X 底数
  * 参    数：Y 指数
  * 返 回 值：等于X的Y次方
  */
uint32_t ST7585_Pow(uint32_t X, uint32_t Y)
{
    uint32_t Result = 1;
    while (Y --)
    {
        Result *= X;
    }
    return Result;
}

/* 功能函数 */

/**
  * 函    数：将ST7585显存数组更新到LCD屏幕
  * 参    数：无
  * 返 回 值：无
  */
void ST7585_Update(void)
{
    uint8_t j;
    /* 遍历每一页 (4页) */
    for (j = 0; j < 4; j ++)
    {
        /* 设置光标位置为每一页的第一列 */
        ST7585_SetCursor(j, 0);
        /* 写入写RAM命令 */
        ST7585_WriteCommand(ST7585_CMD_WRITE_RAM);
        /* 连续写入64个数据，将显存数组的数据写入到LCD硬件 */
        ST7585_WriteData(ST7585_DisplayBuf[j], 64);
    }
}

/**
  * 函    数：将ST7585显存数组部分更新到LCD屏幕
  * 参    数：X 指定区域左上角的横坐标，范围：-32768~32767，屏幕区域：0~63
  * 参    数：Y 指定区域左上角的纵坐标，范围：-32768~32767，屏幕区域：0~31
  * 参    数：Width 指定区域的宽度，范围：0~64
  * 参    数：Height 指定区域的高度，范围：0~32
  * 返 回 值：无
  */
void ST7585_UpdateArea(int16_t X, int16_t Y, uint8_t Width, uint8_t Height)
{
    int16_t j;
    int16_t Page, Page1;
    
    Page = Y / 8;
    Page1 = (Y + Height - 1) / 8 + 1;
    if (Y < 0)
    {
        Page -= 1;
        Page1 -= 1;
    }
    
    /* 遍历指定区域涉及的相关页 */
    for (j = Page; j < Page1; j ++)
    {
        if (X >= 0 && X <= 63 && j >= 0 && j <= 3)
        {
            ST7585_SetCursor(j, X);
            ST7585_WriteCommand(ST7585_CMD_WRITE_RAM);
            ST7585_WriteData(&ST7585_DisplayBuf[j][X], Width);
        }
    }
}

/**
  * 函    数：将ST7585显存数组全部清零
  * 参    数：无
  * 返 回 值：无
  */
void ST7585_Clear(void)
{
    uint8_t i, j;
    for (j = 0; j < 4; j ++)            // 遍历4页
    {
        for (i = 0; i < 64; i ++)       // 遍历64列
        {
            ST7585_DisplayBuf[j][i] = 0x00;
        }
    }
}

/**
  * 函    数：将ST7585显存数组部分清零
  * 参    数：X 指定区域左上角的横坐标，范围：-32768~32767，屏幕区域：0~63
  * 参    数：Y 指定区域左上角的纵坐标，范围：-32768~32767，屏幕区域：0~31
  * 参    数：Width 指定区域的宽度，范围：0~64
  * 参    数：Height 指定区域的高度，范围：0~32
  * 返 回 值：无
  */
void ST7585_ClearArea(int16_t X, int16_t Y, uint8_t Width, uint8_t Height)
{
    int16_t i, j;
    
    for (j = Y; j < Y + Height; j ++)
    {
        for (i = X; i < X + Width; i ++)
        {
            if (i >= 0 && i <= 63 && j >= 0 && j <= 31)
            {
                ST7585_DisplayBuf[j / 8][i] &= ~(0x01 << (j % 8));
            }
        }
    }
}

/**
  * 函    数：将ST7585显存数组全部取反
  * 参    数：无
  * 返 回 值：无
  */
void ST7585_Reverse(void)
{
    uint8_t i, j;
    for (j = 0; j < 4; j ++)
    {
        for (i = 0; i < 64; i ++)
        {
            ST7585_DisplayBuf[j][i] ^= 0xFF;
        }
    }
}

/**
  * 函    数：将ST7585显存数组部分取反
  * 参    数：X 指定区域左上角的横坐标，范围：-32768~32767，屏幕区域：0~63
  * 参    数：Y 指定区域左上角的纵坐标，范围：-32768~32767，屏幕区域：0~31
  * 参    数：Width 指定区域的宽度，范围：0~64
  * 参    数：Height 指定区域的高度，范围：0~32
  * 返 回 值：无
  */
void ST7585_ReverseArea(int16_t X, int16_t Y, uint8_t Width, uint8_t Height)
{
    int16_t i, j;
    
    for (j = Y; j < Y + Height; j ++)
    {
        for (i = X; i < X + Width; i ++)
        {
            if (i >= 0 && i <= 63 && j >= 0 && j <= 31)
            {
                ST7585_DisplayBuf[j / 8][i] ^= 0x01 << (j % 8);
            }
        }
    }
}

/**
  * 函    数：ST7585显示一个字符
  * 参    数：X 指定字符左上角的横坐标，范围：-32768~32767，屏幕区域：0~63
  * 参    数：Y 指定字符左上角的纵坐标，范围：-32768~32767，屏幕区域：0~31
  * 参    数：Char 指定要显示的字符，范围：ASCII码可见字符
  * 参    数：FontSize 指定字体大小
  *           范围：ST7585_8X16		宽8像素，高16像素
  *                 ST7585_6X8		宽6像素，高8像素
  * 返 回 值：无
  */
void ST7585_ShowChar(int16_t X, int16_t Y, char Char, uint8_t FontSize)
{
    if (FontSize == ST7585_8X16)
    {
        /* 检查是否超出屏幕范围 */
        if (X + 8 > 64 || Y + 16 > 32) return;
        ST7585_ShowImage(X, Y, 8, 16, ST7585_F8x16[Char - ' ']);
    }
    else if(FontSize == ST7585_6X8)
    {
        if (X + 6 > 64 || Y + 8 > 32) return;
        ST7585_ShowImage(X, Y, 6, 8, ST7585_F6x8[Char - ' ']);
    }
}

/**
  * 函    数：ST7585显示字符串
  * 参    数：X 指定字符串左上角的横坐标，范围：-32768~32767，屏幕区域：0~63
  * 参    数：Y 指定字符串左上角的纵坐标，范围：-32768~32767，屏幕区域：0~31
  * 参    数：String 指定要显示的字符串
  * 参    数：FontSize 指定字体大小
  *           范围：ST7585_8X16		宽8像素，高16像素
  *                 ST7585_6X8		宽6像素，高8像素
  * 返 回 值：无
  */
void ST7585_ShowString(int16_t X, int16_t Y, char *String, uint8_t FontSize)
{
    uint16_t i = 0;
    uint16_t XOffset = 0;
    uint8_t charWidth = (FontSize == ST7585_8X16) ? 8 : 6;
    
    while (String[i] != '\0')
    {
        if (X + XOffset + charWidth <= 64)
        {
            ST7585_ShowChar(X + XOffset, Y, String[i], FontSize);
            XOffset += charWidth;
        }
        else
        {
            break;  // 超出屏幕宽度，停止显示
        }
        i++;
    }
}

/**
  * 函    数：ST7585显示数字（十进制，正整数）
  * 参    数：X 指定数字左上角的横坐标，范围：-32768~32767，屏幕区域：0~63
  * 参    数：Y 指定数字左上角的纵坐标，范围：-32768~32767，屏幕区域：0~31
  * 参    数：Number 指定要显示的数字，范围：0~4294967295
  * 参    数：Length 指定数字的长度，范围：0~10
  * 参    数：FontSize 指定字体大小
  */
void ST7585_ShowNum(int16_t X, int16_t Y, uint32_t Number, uint8_t Length, uint8_t FontSize)
{
    uint8_t i;
    uint8_t charWidth = (FontSize == ST7585_8X16) ? 8 : 6;
    
    for (i = 0; i < Length; i++)
    {
        if (X + i * charWidth + charWidth <= 64)
        {
            ST7585_ShowChar(X + i * charWidth, Y, 
                Number / ST7585_Pow(10, Length - i - 1) % 10 + '0', FontSize);
        }
    }
}

/**
  * 函    数：ST7585显示有符号数字
  */
void ST7585_ShowSignedNum(int16_t X, int16_t Y, int32_t Number, uint8_t Length, uint8_t FontSize)
{
    uint8_t i;
    uint32_t Number1;
    uint8_t charWidth = (FontSize == ST7585_8X16) ? 8 : 6;
    
    if (Number >= 0)
    {
        if (X + charWidth <= 64)
        {
            ST7585_ShowChar(X, Y, '+', FontSize);
            Number1 = Number;
        }
    }
    else
    {
        if (X + charWidth <= 64)
        {
            ST7585_ShowChar(X, Y, '-', FontSize);
            Number1 = -Number;
        }
    }
    
    for (i = 0; i < Length; i++)
    {
        if (X + (i + 1) * charWidth + charWidth <= 64)
        {
            ST7585_ShowChar(X + (i + 1) * charWidth, Y, 
                Number1 / ST7585_Pow(10, Length - i - 1) % 10 + '0', FontSize);
        }
    }
}

/**
  * 函    数：ST7585显示十六进制数字
  */
void ST7585_ShowHexNum(int16_t X, int16_t Y, uint32_t Number, uint8_t Length, uint8_t FontSize)
{
    uint8_t i, SingleNumber;
    uint8_t charWidth = (FontSize == ST7585_8X16) ? 8 : 6;
    
    for (i = 0; i < Length; i++)
    {
        if (X + i * charWidth + charWidth <= 64)
        {
            SingleNumber = Number / ST7585_Pow(16, Length - i - 1) % 16;
            
            if (SingleNumber < 10)
            {
                ST7585_ShowChar(X + i * charWidth, Y, SingleNumber + '0', FontSize);
            }
            else
            {
                ST7585_ShowChar(X + i * charWidth, Y, SingleNumber - 10 + 'A', FontSize);
            }
        }
    }
}

/**
  * 函    数：ST7585显示二进制数字
  */
void ST7585_ShowBinNum(int16_t X, int16_t Y, uint32_t Number, uint8_t Length, uint8_t FontSize)
{
    uint8_t i;
    uint8_t charWidth = (FontSize == ST7585_8X16) ? 8 : 6;
    
    for (i = 0; i < Length; i++)
    {
        if (X + i * charWidth + charWidth <= 64)
        {
            ST7585_ShowChar(X + i * charWidth, Y, 
                Number / ST7585_Pow(2, Length - i - 1) % 2 + '0', FontSize);
        }
    }
}

/**
  * 函    数：ST7585显示图像
  * 参    数：X 指定图像左上角的横坐标，范围：-32768~32767，屏幕区域：0~63
  * 参    数：Y 指定图像左上角的纵坐标，范围：-32768~32767，屏幕区域：0~31
  * 参    数：Width 指定图像的宽度，范围：0~64
  * 参    数：Height 指定图像的高度，范围：0~32
  * 参    数：Image 指定要显示的图像
  */
void ST7585_ShowImage(int16_t X, int16_t Y, uint8_t Width, uint8_t Height, const uint8_t *Image)
{
    uint8_t i = 0, j = 0;
    int16_t Page, Shift;
    
    /* 检查图像尺寸是否超出屏幕范围 */
    if (X + Width > 64) Width = 64 - X;
    if (Y + Height > 32) Height = 32 - Y;
    
    /* 将图像所在区域清空 */
    ST7585_ClearArea(X, Y, Width, Height);
    
    /* 遍历指定图像涉及的相关页 */
    for (j = 0; j < (Height - 1) / 8 + 1; j ++)
    {
        /* 遍历指定图像涉及的相关列 */
        for (i = 0; i < Width; i ++)
        {
            if (X + i >= 0 && X + i <= 63)
            {
                Page = Y / 8;
                Shift = Y % 8;
                if (Y < 0)
                {
                    Page -= 1;
                    Shift += 8;
                }
                
                if (Page + j >= 0 && Page + j <= 3)
                {
                    ST7585_DisplayBuf[Page + j][X + i] |= Image[j * Width + i] << (Shift);
                }
                
                if (Page + j + 1 >= 0 && Page + j + 1 <= 3)
                {                   
                    ST7585_DisplayBuf[Page + j + 1][X + i] |= Image[j * Width + i] >> (8 - Shift);
                }
            }
        }
    }
}

/**
  * 函    数：ST7585使用printf函数打印格式化字符串
  */
void ST7585_Printf(int16_t X, int16_t Y, uint8_t FontSize, char *format, ...)
{
    char String[128];
    va_list arg;
    va_start(arg, format);
    vsprintf(String, format, arg);
    va_end(arg);
    ST7585_ShowString(X, Y, String, FontSize);
}

/**
  * 函    数：ST7585在指定位置画一个点
  */
void ST7585_DrawPoint(int16_t X, int16_t Y)
{
    if (X >= 0 && X <= 63 && Y >= 0 && Y <= 31)
    {
        ST7585_DisplayBuf[Y / 8][X] |= 0x01 << (Y % 8);
    }
}

/**
  * 函    数：ST7585获取指定位置点的值
  */
uint8_t ST7585_GetPoint(int16_t X, int16_t Y)
{
    if (X >= 0 && X <= 63 && Y >= 0 && Y <= 31)
    {
        if (ST7585_DisplayBuf[Y / 8][X] & 0x01 << (Y % 8))
        {
            return 1;
        }
    }
    return 0;
}

/**
  * 函    数：ST7585画线（Bresenham算法）
  */
void ST7585_DrawLine(int16_t X0, int16_t Y0, int16_t X1, int16_t Y1)
{
    int16_t x, y, dx, dy, d, incrE, incrNE, temp;
    int16_t x0 = X0, y0 = Y0, x1 = X1, y1 = Y1;
    uint8_t yflag = 0, xyflag = 0;
    
    if (y0 == y1)
    {
        if (x0 > x1) {temp = x0; x0 = x1; x1 = temp;}
        for (x = x0; x <= x1; x ++)
        {
            ST7585_DrawPoint(x, y0);
        }
    }
    else if (x0 == x1)
    {
        if (y0 > y1) {temp = y0; y0 = y1; y1 = temp;}
        for (y = y0; y <= y1; y ++)
        {
            ST7585_DrawPoint(x0, y);
        }
    }
    else
    {
        if (x0 > x1)
        {
            temp = x0; x0 = x1; x1 = temp;
            temp = y0; y0 = y1; y1 = temp;
        }
        
        if (y0 > y1)
        {
            y0 = -y0;
            y1 = -y1;
            yflag = 1;
        }
        
        if (y1 - y0 > x1 - x0)
        {
            temp = x0; x0 = y0; y0 = temp;
            temp = x1; x1 = y1; y1 = temp;
            xyflag = 1;
        }
        
        dx = x1 - x0;
        dy = y1 - y0;
        incrE = 2 * dy;
        incrNE = 2 * (dy - dx);
        d = 2 * dy - dx;
        x = x0;
        y = y0;
        
        if (yflag && xyflag){ST7585_DrawPoint(y, -x);}
        else if (yflag)     {ST7585_DrawPoint(x, -y);}
        else if (xyflag)    {ST7585_DrawPoint(y, x);}
        else                {ST7585_DrawPoint(x, y);}
        
        while (x < x1)
        {
            x ++;
            if (d < 0)
            {
                d += incrE;
            }
            else
            {
                y ++;
                d += incrNE;
            }
            
            if (yflag && xyflag){ST7585_DrawPoint(y, -x);}
            else if (yflag)     {ST7585_DrawPoint(x, -y);}
            else if (xyflag)    {ST7585_DrawPoint(y, x);}
            else                {ST7585_DrawPoint(x, y);}
        }   
    }
}

/**
  * 函    数：ST7585矩形
  */
void ST7585_DrawRectangle(int16_t X, int16_t Y, uint8_t Width, uint8_t Height, uint8_t IsFilled)
{
    int16_t i, j;
    
    /* 检查边界 */
    if (X + Width > 64) Width = 64 - X;
    if (Y + Height > 32) Height = 32 - Y;
    
    if (!IsFilled)
    {
        for (i = X; i < X + Width; i ++)
        {
            ST7585_DrawPoint(i, Y);
            ST7585_DrawPoint(i, Y + Height - 1);
        }
        for (i = Y; i < Y + Height; i ++)
        {
            ST7585_DrawPoint(X, i);
            ST7585_DrawPoint(X + Width - 1, i);
        }
    }
    else
    {
        for (i = X; i < X + Width; i ++)
        {
            for (j = Y; j < Y + Height; j ++)
            {
                ST7585_DrawPoint(i, j);
            }
        }
    }
}

/**
  * 函    数：ST7585画圆
  */
void ST7585_DrawCircle(int16_t X, int16_t Y, uint8_t Radius, uint8_t IsFilled)
{
    int16_t x, y, d;
    
    /* 限制半径，避免超出屏幕 */
    if (Radius > 16) Radius = 16;
    
    if (!IsFilled)
    {
        d = 1 - Radius;
        x = 0;
        y = Radius;
        
        while (x <= y)
        {
            ST7585_DrawPoint(X + x, Y + y);
            ST7585_DrawPoint(X - x, Y - y);
            ST7585_DrawPoint(X + y, Y + x);
            ST7585_DrawPoint(X - y, Y - x);
            ST7585_DrawPoint(X + x, Y - y);
            ST7585_DrawPoint(X - x, Y + y);
            ST7585_DrawPoint(X + y, Y - x);
            ST7585_DrawPoint(X - y, Y + x);
            
            if (d < 0)
            {
                d += 2 * x + 1;
            }
            else
            {
                d += 2 * (x - y) + 1;
                y--;
            }
            x++;
        }
    }
    else
    {
        /* 简化的填充圆：画一系列同心圆 */
        for (uint8_t r = 0; r <= Radius; r++)
        {
            d = 1 - r;
            x = 0;
            y = r;
            
            while (x <= y)
            {
                for (int16_t i = X - x; i <= X + x; i++)
                {
                    ST7585_DrawPoint(i, Y + y);
                    ST7585_DrawPoint(i, Y - y);
                }
                for (int16_t i = X - y; i <= X + y; i++)
                {
                    ST7585_DrawPoint(i, Y + x);
                    ST7585_DrawPoint(i, Y - x);
                }
                
                if (d < 0)
                {
                    d += 2 * x + 1;
                }
                else
                {
                    d += 2 * (x - y) + 1;
                    y--;
                }
                x++;
            }
        }
    }
}

/**
  * 函    数：设置对比度 (32级可调)
  * 参    数：Contrast 对比度值，范围0-31
  */
void ST7585_SetContrast(uint8_t Contrast)
{
    if (Contrast > 31) Contrast = 31;
    ST7585_WriteCommand(ST7585_CMD_SET_ELECTRONIC_VOLUME);
    ST7585_WriteCommand(Contrast);
}

/**
  * 函    数：开启显示
  */
void ST7585_DisplayOn(void)
{
    ST7585_WriteCommand(ST7585_CMD_DISPLAY_ON);
}

/**
  * 函    数：关闭显示
  */
void ST7585_DisplayOff(void)
{
    ST7585_WriteCommand(ST7585_CMD_DISPLAY_OFF);
}

/**
  * 函    数：设置睡眠模式
  * 参    数：Enable 1-进入睡眠模式，0-退出睡眠模式
  */
void ST7585_SetSleepMode(uint8_t Enable)
{
    if (Enable)
    {
        ST7585_WriteCommand(ST7585_CMD_ENTER_SLEEP_MODE);
    }
    else
    {
        ST7585_WriteCommand(ST7585_CMD_EXIT_SLEEP_MODE);
    }
}

/**
  * 函    数：设置偏压
  * 参    数：Bias_1_9 1: 1/9偏压 (高电压), 0: 1/7偏压 (低电压)
  */
void ST7585_SetBias(uint8_t Bias_1_9)
{
    if (Bias_1_9)
    {
        ST7585_WriteCommand(ST7585_CMD_SET_BIAS_1_9);
    }
    else
    {
        ST7585_WriteCommand(ST7585_CMD_SET_BIAS_1_7);
    }
}

/**
  * 函    数：设置扫描方向
  * 参    数：ComDir COM扫描方向 (1: 反向, 0: 正常)
  * 参    数：SegDir SEG扫描方向 (1: 反向, 0: 正常)
  */
void ST7585_SetDirection(uint8_t ComDir, uint8_t SegDir)
{
    if (ComDir)
    {
        ST7585_WriteCommand(0xC8);  // COM反向扫描
    }
    else
    {
        ST7585_WriteCommand(0xC0);  // COM正常扫描
    }
    
    if (SegDir)
    {
        ST7585_WriteCommand(0xA1);  // SEG反向扫描
    }
    else
    {
        ST7585_WriteCommand(0xA0);  // SEG正常扫描
    }
}