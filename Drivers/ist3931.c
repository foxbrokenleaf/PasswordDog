/***************************************************************************************
  * 程序名称：				IST3931 LCD显示屏驱动程序
  * 屏幕分辨率：				144x65
  * 驱动芯片：				IST3931
  * 通信协议：				I2C
  * 引脚数量：               14Pin
  * 程序创建时间：			2025.03.16
  * 说明：修复显示内容自动消失的问题
  ***************************************************************************************
  */

#include "stm32f10x.h"
#include "ist3931.h"
#include <string.h>
#include <stdio.h>

/* 全局变量 - 显存数组 */
uint8_t IST3931_DisplayBuf[9][144];  // [页][列]，9页 * 144列

/* 全局标志 - 记录显示状态 */
static uint8_t display_on = 1;

/* GPIO引脚定义 */
#define IST3931_SCL_PORT        GPIOB
#define IST3931_SCL_PIN         GPIO_Pin_6
#define IST3931_SDA_PORT        GPIOB
#define IST3931_SDA_PIN         GPIO_Pin_7
#define IST3931_RST_PORT        GPIOB
#define IST3931_RST_PIN         GPIO_Pin_8

/* I2C延时参数 */
#define I2C_DELAY_US            2

/* 应答标志 - 使用volatile避免优化警告 */
static volatile uint8_t ack = 1;

/**
  * 隔行扫描映射表 - 物理COM到逻辑Y坐标的映射
  * 根据您提供的顺序：COM33(32),COM16(15),COM32(31),COM15(14),COM31(30),COM14(13),...
  * 注意：物理COM编号从0开始（对应COM1）
  */
static const uint8_t phys_to_log[65] = {
    /* 需要根据您的完整COM顺序填写，这里是示例 */
    32, 15, 31, 14, 30, 13, 29, 12, 28, 11, 27, 10, 26, 9, 25, 8,
    24, 7, 23, 6, 22, 5, 21, 4, 20, 3, 19, 2, 18, 1, 17, 0,
    /* 剩余33个映射需要根据实际规律填写 */
    33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48,
    49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64,
    65
};

/**
  * 逻辑Y坐标到物理COM的映射
  */
static const uint8_t log_to_phys[65] = {
    /* 根据phys_to_log反推，需要正确填写 */
    31, 29, 27, 25, 23, 21, 19, 17, 15, 13, 11, 9, 7, 5, 3, 1,
    0, 2, 4, 6, 8, 10, 12, 14, 16, 18, 20, 22, 24, 26, 28, 30,
    /* 剩余33个映射需要根据实际规律填写 */
    32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47,
    48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63,
    64
};

/* 延时函数 */
void IST3931_DelayUs(uint32_t us)
{
    uint32_t i;
    for (i = 0; i < us * 8; i++)
    {
        __NOP();
    }
}

void IST3931_DelayMs(uint32_t ms)
{
    uint32_t i;
    for (i = 0; i < ms; i++)
    {
        IST3931_DelayUs(1000);
    }
}

/* GPIO控制函数 */
void IST3931_W_SCL(uint8_t BitValue)
{
    GPIO_WriteBit(IST3931_SCL_PORT, IST3931_SCL_PIN, (BitAction)BitValue);
}

void IST3931_W_SDA(uint8_t BitValue)
{
    GPIO_WriteBit(IST3931_SDA_PORT, IST3931_SDA_PIN, (BitAction)BitValue);
}

void IST3931_W_RST(uint8_t BitValue)
{
    GPIO_WriteBit(IST3931_RST_PORT, IST3931_RST_PIN, (BitAction)BitValue);
}

uint8_t IST3931_R_SDA(void)
{
    return GPIO_ReadInputDataBit(IST3931_SDA_PORT, IST3931_SDA_PIN);
}

/* GPIO初始化 */
void IST3931_GPIO_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Pin = IST3931_SCL_PIN;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(IST3931_SCL_PORT, &GPIO_InitStructure);
    
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Pin = IST3931_SDA_PIN;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(IST3931_SDA_PORT, &GPIO_InitStructure);
    
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Pin = IST3931_RST_PIN;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(IST3931_RST_PORT, &GPIO_InitStructure);
    
    IST3931_W_SCL(1);
    IST3931_W_SDA(1);
    IST3931_W_RST(1);
    
    IST3931_DelayMs(50);
}

/* I2C协议函数 */
void iic_start(void)
{
    IST3931_W_SDA(1);
    IST3931_DelayUs(I2C_DELAY_US);
    IST3931_W_SCL(1);
    IST3931_DelayUs(I2C_DELAY_US);
    IST3931_W_SDA(0);
    IST3931_DelayUs(I2C_DELAY_US);
    IST3931_W_SCL(0);
    IST3931_DelayUs(I2C_DELAY_US);
}

void iic_stop(void)
{
    IST3931_W_SDA(0);
    IST3931_DelayUs(I2C_DELAY_US);
    IST3931_W_SCL(1);
    IST3931_DelayUs(I2C_DELAY_US);
    IST3931_W_SDA(1);
    IST3931_DelayUs(I2C_DELAY_US);
}

void IICSendByte(uint8_t Byte)
{
    uint8_t i;
    
    for (i = 0; i < 8; i++)
    {
        if (Byte & 0x80)
            IST3931_W_SDA(1);
        else
            IST3931_W_SDA(0);
        
        Byte <<= 1;
        
        IST3931_DelayUs(I2C_DELAY_US);
        IST3931_W_SCL(1);
        IST3931_DelayUs(I2C_DELAY_US);
        IST3931_W_SCL(0);
        IST3931_DelayUs(I2C_DELAY_US);
    }
}

void check_ACK(void)
{
    IST3931_W_SDA(1);
    IST3931_DelayUs(I2C_DELAY_US);
    IST3931_W_SCL(1);
    IST3931_DelayUs(I2C_DELAY_US);
    
    if (IST3931_R_SDA() == 1)
        ack = 0;
    else
        ack = 1;
    
    IST3931_W_SCL(0);
    IST3931_DelayUs(I2C_DELAY_US);
}

/* 硬件复位 */
void IST3931_HardwareReset(void)
{
    IST3931_W_RST(0);
    IST3931_DelayMs(10);
    IST3931_W_RST(1);
    IST3931_DelayMs(10);
}

/* 写命令 */
void IST3931_WriteCommand(uint8_t Command)
{
    iic_start();
    IICSendByte(IST3931_SLAVE_ADDR);
    check_ACK();
    IICSendByte(0x80);
    check_ACK();
    IICSendByte(Command);
    check_ACK();
    iic_stop();
}

/* 写数据 */
void IST3931_WriteData(uint8_t Data)
{
    iic_start();
    IICSendByte(IST3931_SLAVE_ADDR);
    check_ACK();
    IICSendByte(0xc0);
    check_ACK();
    IICSendByte(Data);
    check_ACK();
    iic_stop();
}

/* 写多个数据 */
void IST3931_WriteDataMulti(uint8_t *Data, uint16_t Count)
{
    uint16_t i;
    
    iic_start();
    IICSendByte(IST3931_SLAVE_ADDR);
    check_ACK();
    IICSendByte(0xc0);
    check_ACK();
    
    for (i = 0; i < Count; i++)
    {
        IICSendByte(Data[i]);
        check_ACK();
    }
    
    iic_stop();
}

/* 物理行地址到逻辑行地址的映射 */
uint8_t IST3931_MapCOM(uint8_t physical_y)
{
    if (physical_y < 65)
        return phys_to_log[physical_y];
    return physical_y;
}

/* 逻辑行地址到物理行地址的映射 */
uint8_t IST3931_MapPhysicalY(uint8_t logical_y)
{
    if (logical_y < 65)
        return log_to_phys[logical_y];
    return logical_y;
}

/* 设置光标位置 - Y是物理行地址 */
void IST3931_SetCursor(uint8_t Y, uint8_t X)
{
    /* Y: 物理行地址 (0-64)
       X: 列地址，8个列为一个地址 (0-17) */
    
    IST3931_WriteCommand(0x10 + (Y >> 4));      // 设置行地址高3位
    IST3931_WriteCommand(0x00 + (Y & 0x0F));    // 设置行地址低4位
    IST3931_WriteCommand(0xc0 + X);              // 设置列地址
}

/* 初始化序列 */
void Write_ist3931_init1(void)
{
    IST3931_WriteCommand(0x3a);  // 开启时钟振荡
    IST3931_WriteCommand(0x61);  // 行排列方式【正常】
    IST3931_WriteCommand(0x2f);  // 升压器开
    IST3931_WriteCommand(0xb1);  // 设置升压电压
    IST3931_WriteCommand(200);    // 设置升压电压值(对比度)
    IST3931_WriteCommand(0x34);  // LCD Blass（亮度）
    IST3931_WriteCommand(0x62);  // 行、列方向，全亮
    IST3931_WriteCommand(0x91);  // 占空比低4位
    IST3931_WriteCommand(0xa2);  // 占空比高3位
    IST3931_WriteCommand(0x40);  // 起始行低4位
    IST3931_WriteCommand(0x50);  // 起始行高3位
    IST3931_WriteCommand(0x3d);  // 开显示
}

/* 设置正常显示模式 */
void IST3931_SetNormalMode(void)
{
    IST3931_WriteCommand(0x60);  // 正常显示
}

/* 清屏函数 - 使用物理地址 */
void ist3931_disp_clear(void)
{
    uint16_t i;
    uint8_t y;
    
    /* 需要遍历所有65行 */
    for (y = 0; y < 65; y++)
    {
        IST3931_SetCursor(y, 0);
        
        /* 每行有18个列地址（144/8=18）*/
        for (i = 0; i < 18; i++)
        {
            IST3931_WriteData(0x00);
        }
    }
}

/* 主初始化函数 */
void IST3931_Init(void)
{
    IST3931_GPIO_Init();
    IST3931_HardwareReset();
    IST3931_DelayMs(50);
    
    Write_ist3931_init1();
    IST3931_DelayMs(20);
    
    IST3931_SetNormalMode();
    IST3931_DelayMs(10);
    
    ist3931_disp_clear();
    IST3931_Clear();
    
    display_on = 1;
}

/* 清空显存 - 显存中存储的是逻辑坐标的数据 */
void IST3931_Clear(void)
{
    uint8_t i, j;
    for (j = 0; j < 9; j++)
    {
        for (i = 0; i < 144; i++)
        {
            IST3931_DisplayBuf[j][i] = 0x00;
        }
    }
}

/**
  * 函    数：更新全屏 - 考虑隔行扫描映射
  */
void IST3931_Update(void)
{
    uint8_t col;
    uint16_t physical_y;
    uint8_t logical_y;
    uint8_t page_data[18];  // 每行18个字节数据
    
    /* 遍历所有65行（物理行）*/
    for (physical_y = 0; physical_y < 65; physical_y++)
    {
        /* 将物理行映射回逻辑行，找到显存中对应的数据 */
        logical_y = IST3931_MapCOM(physical_y);
        
        /* 设置光标到物理行地址 */
        IST3931_SetCursor(physical_y, 0);
        
        /* 准备这一行对应的数据（从显存中提取）*/
        for (col = 0; col < 18; col++)
        {
            uint8_t page_idx = logical_y / 8;
            uint8_t bit_pos = logical_y % 8;
            uint8_t data_byte = 0;
            uint8_t i;
            
            /* 收集这一行在当前列的8个像素 */
            for (i = 0; i < 8; i++)
            {
                if (col * 8 + i < 144)  // 防止越界
                {
                    if (IST3931_DisplayBuf[page_idx][col * 8 + i] & (0x01 << bit_pos))
                    {
                        data_byte |= (0x80 >> i);
                    }
                }
            }
            
            page_data[col] = data_byte;
        }
        
        /* 写入这一行的所有列数据 */
        IST3931_WriteDataMulti(page_data, 18);
    }
}

/**
  * 函    数：更新指定区域
  */
void IST3931_UpdateArea(uint16_t X, uint16_t Y, uint8_t Width, uint8_t Height)
{
    uint16_t physical_y;
    uint16_t logical_y_start, logical_y_end;
    uint16_t x_start_col, x_end_col;
    uint8_t col;
    
    logical_y_start = Y;
    logical_y_end = Y + Height - 1;
    x_start_col = X / 8;
    x_end_col = (X + Width - 1) / 8;
    
    if (x_end_col > 17) x_end_col = 17;
    
    /* 遍历所有65行物理行，找出受影响的 */
    for (physical_y = 0; physical_y < 65; physical_y++)
    {
        uint8_t logical_y = IST3931_MapCOM(physical_y);
        
        if (logical_y >= logical_y_start && logical_y <= logical_y_end)
        {
            /* 这一行需要更新 */
            IST3931_SetCursor(physical_y, x_start_col);
            
            /* 准备这一行受影响列的数据 */
            for (col = x_start_col; col <= x_end_col; col++)
            {
                uint8_t page_idx = logical_y / 8;
                uint8_t bit_pos = logical_y % 8;
                uint8_t data_byte = 0;
                uint8_t i;
                
                for (i = 0; i < 8; i++)
                {
                    uint16_t pixel_x = col * 8 + i;
                    if (pixel_x >= X && pixel_x < X + Width && pixel_x < 144)
                    {
                        if (IST3931_DisplayBuf[page_idx][pixel_x] & (0x01 << bit_pos))
                        {
                            data_byte |= (0x80 >> i);
                        }
                    }
                }
                
                IST3931_WriteData(data_byte);
            }
        }
    }
}

/* 画点 - 在显存中操作，使用逻辑坐标 */
void IST3931_DrawPoint(uint16_t X, uint16_t Y)
{
    if (X < IST3931_WIDTH && Y < IST3931_HEIGHT)
    {
        IST3931_DisplayBuf[Y / 8][X] |= 0x01 << (Y % 8);
    }
}

/**
  * 函    数：显示16x8字体 - 完全匹配成功代码
  */
void IST3931_ShowFont16x8(uint8_t x, uint8_t y, uint8_t num, const uint8_t font[][16])
{
    uint8_t i, yy;
    uint8_t physical_y;
    
    for(i = 0; i < 16; i++)
    {
        if(i % 2 == 0)  // 偶数行
        {
            yy = 24 - i/2 + y;  // 逻辑行地址
            if (yy < 65)
            {
                physical_y = IST3931_MapPhysicalY(yy);  // 转换为物理行地址
                
                IST3931_WriteCommand(0x10 + (physical_y >> 4));      // 行地址高3位
                IST3931_WriteCommand(0x00 + (physical_y & 0x0F));    // 行地址低4位
                IST3931_WriteCommand(0xc0 + x);                      // 列地址
                IST3931_WriteData(font[num][i]);
            }
        }
        else  // 奇数行
        {
            yy = 7 - i/2 + y;  // 逻辑行地址
            if (yy < 65)
            {
                physical_y = IST3931_MapPhysicalY(yy);  // 转换为物理行地址
                
                IST3931_WriteCommand(0x10 + (physical_y >> 4));      // 行地址高3位
                IST3931_WriteCommand(0x00 + (physical_y & 0x0F));    // 行地址低4位
                IST3931_WriteCommand(0xc0 + x);                      // 列地址
                IST3931_WriteData(font[num][i]);
            }
        }
    }
}

/**
  * 函    数：显示8x16字符 - 考虑隔行扫描
  */
void IST3931_ShowChar(uint8_t X, uint8_t Y, char Char, const uint8_t font8x16[][16])
{
    uint8_t i;
    uint8_t yy;
    uint8_t physical_y;
    uint8_t char_index = Char - ' ';
    
    if (char_index > 95) char_index = 0;  // 防止越界
    
    for(i = 0; i < 16; i++)
    {
        if(i % 2 == 0)
        {
            yy = 24 - i/2 + Y;
            if (yy < 65)
            {
                physical_y = IST3931_MapPhysicalY(yy);
                
                IST3931_WriteCommand(0x10 + (physical_y >> 4));
                IST3931_WriteCommand(0x00 + (physical_y & 0x0F));
                IST3931_WriteCommand(0xc0 + X);
                IST3931_WriteData(font8x16[char_index][i]);
            }
        }
        else
        {
            yy = 7 - i/2 + Y;
            if (yy < 65)
            {
                physical_y = IST3931_MapPhysicalY(yy);
                
                IST3931_WriteCommand(0x10 + (physical_y >> 4));
                IST3931_WriteCommand(0x00 + (physical_y & 0x0F));
                IST3931_WriteCommand(0xc0 + X);
                IST3931_WriteData(font8x16[char_index][i]);
            }
        }
    }
}

/**
  * 函    数：通过显存显示字符（双缓冲）
  */
void IST3931_ShowCharBuf(uint16_t X, uint16_t Y, char Char, const uint8_t font8x16[][16])
{
    uint8_t i, j;
    uint8_t char_index = Char - ' ';
    uint16_t buf_x, buf_y;
    
    if (char_index > 95) char_index = 0;
    
    for (j = 0; j < 16; j++)  // 高度16像素
    {
        for (i = 0; i < 8; i++)  // 宽度8像素
        {
            buf_x = X + i;
            buf_y = Y + j;
            
            if (buf_x < IST3931_WIDTH && buf_y < IST3931_HEIGHT)
            {
                if (font8x16[char_index][j] & (0x80 >> i))
                {
                    IST3931_DisplayBuf[buf_y / 8][buf_x] |= 0x01 << (buf_y % 8);
                }
                else
                {
                    IST3931_DisplayBuf[buf_y / 8][buf_x] &= ~(0x01 << (buf_y % 8));
                }
            }
        }
    }
}

/**
  * 函    数：显示字符串（直接写屏）
  */
void IST3931_ShowString(uint8_t X, uint8_t Y, char *String, const uint8_t font8x16[][16])
{
    uint8_t i = 0;
    
    while (String[i] != '\0' && X + i <= IST3931_COL_ADDR_MAX)
    {
        IST3931_ShowChar(X + i, Y, String[i], font8x16);
        i++;
    }
}

/**
  * 函    数：显示字符串（双缓冲）
  */
void IST3931_ShowStringBuf(uint16_t X, uint16_t Y, char *String, const uint8_t font8x16[][16])
{
    uint8_t i = 0;
    
    while (String[i] != '\0')
    {
        if (X + i * 8 + 8 <= IST3931_WIDTH)
        {
            IST3931_ShowCharBuf(X + i * 8, Y, String[i], font8x16);
        }
        else
        {
            break;
        }
        i++;
    }
}

/**
  * 函    数：显示图像
  */
void IST3931_ShowImage(uint16_t X, uint16_t Y, uint8_t Width, uint8_t Height, const uint8_t *Image)
{
    uint8_t i, j;
    uint16_t buf_x, buf_y;
    
    for (j = 0; j < Height && Y + j < IST3931_HEIGHT; j++)
    {
        for (i = 0; i < Width && X + i < IST3931_WIDTH; i++)
        {
            buf_x = X + i;
            buf_y = Y + j;
            
            if (Image[j * Width + i] & (0x01 << (j % 8)))
            {
                IST3931_DisplayBuf[buf_y / 8][buf_x] |= 0x01 << (buf_y % 8);
            }
            else
            {
                IST3931_DisplayBuf[buf_y / 8][buf_x] &= ~(0x01 << (buf_y % 8));
            }
        }
    }
}

/* 设置对比度 */
void IST3931_SetContrast(uint8_t Contrast)
{
    IST3931_WriteCommand(0xb1);
    IST3931_WriteCommand(Contrast);
}

/* 设置偏压（亮度） */
void IST3931_SetBias(uint8_t Bias)
{
    if (Bias < 0x30) Bias = 0x30;
    if (Bias > 0x37) Bias = 0x37;
    IST3931_WriteCommand(Bias);
}

/* 设置显示模式 */
void IST3931_SetDisplayMode(uint8_t Mode)
{
    IST3931_WriteCommand(Mode);
}

/* 开启显示 - 增强版，确保显示保持 */
void IST3931_DisplayOn(void)
{
    IST3931_WriteCommand(0x3d);  // 开显示
    display_on = 1;
    
    /* 开显示后，立即刷新一次屏幕，确保内容显示 */
    IST3931_Update();
}

/* 关闭显示 */
void IST3931_DisplayOff(void)
{
    IST3931_WriteCommand(0xae);  // 关显示
    display_on = 0;
}

/* 画线函数 - 简单实现 */
void IST3931_DrawLine(uint16_t X0, uint16_t Y0, uint16_t X1, uint16_t Y1)
{
    int16_t dx = X1 > X0 ? X1 - X0 : X0 - X1;
    int16_t dy = Y1 > Y0 ? Y1 - Y0 : Y0 - Y1;
    int16_t steps = dx > dy ? dx : dy;
    
    if (steps == 0)
    {
        IST3931_DrawPoint(X0, Y0);
        return;
    }
    
    for (int16_t i = 0; i <= steps; i++)
    {
        uint16_t x = X0 + (int32_t)(X1 - X0) * i / steps;
        uint16_t y = Y0 + (int32_t)(Y1 - Y0) * i / steps;
        IST3931_DrawPoint(x, y);
    }
}

/**
  * 函    数：刷新显示 - 保持屏幕内容
  * 说    明：在需要保持显示时调用此函数
  */
void IST3931_Refresh(void)
{
    if (display_on)
    {
        IST3931_Update();
    }
}

/**
  * 函    数：设置显示保持模式
  * 说    明：防止屏幕内容自动消失
  */
void IST3931_SetDisplayHold(void)
{
    /* 确保显示开启 */
    IST3931_DisplayOn();
    
    /* 可以设置一些保持显示的寄存器 */
    IST3931_WriteCommand(0xa5);  // 设置显示为所有点亮（测试用）
    IST3931_WriteCommand(0xa4);  // 恢复正常显示模式
}