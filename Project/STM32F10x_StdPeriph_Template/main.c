/**
  ******************************************************************************
  * @file    Project/STM32F10x_StdPeriph_Template/main.c 
  * @author  MCD Application Team
  * @version V3.6.0
  * @date    20-September-2021
  * @brief   Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2011 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "stm32f10x.h"
#include "stm32_eval.h"
#include <stdio.h>
#include "oled.h"
#include "Delay.h"
#include "string.h"
#include "Key.h"
// #include "ist3931.h"
// #include "ist3931_font.h"
// #include "st7585.h"

#ifdef USE_STM32100B_EVAL
 #include "stm32100b_eval_lcd.h"
#elif defined USE_STM3210B_EVAL
 #include "stm3210b_eval_lcd.h"
#elif defined USE_STM3210E_EVAL
//  #include "stm3210e_eval_lcd.h" 
#elif defined USE_STM3210C_EVAL
 #include "stm3210c_eval_lcd.h"
#elif defined USE_STM32100E_EVAL
 #include "stm32100e_eval_lcd.h"
#endif

/** @addtogroup STM32F10x_StdPeriph_Template
  * @{
  */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#ifdef USE_STM32100B_EVAL
  #define MESSAGE1   "STM32 MD Value Line " 
  #define MESSAGE2   " Device running on  " 
  #define MESSAGE3   "  STM32100B-EVAL    " 
#elif defined (USE_STM3210B_EVAL)
  #define MESSAGE1   "STM32 Medium Density" 
  #define MESSAGE2   " Device running on  " 
  #define MESSAGE3   "   STM3210B-EVAL    " 
#elif defined (STM32F10X_XL) && defined (USE_STM3210E_EVAL)
  #define MESSAGE1   "  STM32 XL Density  " 
  #define MESSAGE2   " Device running on  " 
  #define MESSAGE3   "   STM3210E-EVAL    "
#elif defined (USE_STM3210E_EVAL)
  #define MESSAGE1   " STM32 High Density " 
  #define MESSAGE2   " Device running on  " 
  #define MESSAGE3   "   STM3210E-EVAL    " 
#elif defined (USE_STM3210C_EVAL)
  #define MESSAGE1   " STM32 Connectivity " 
  #define MESSAGE2   " Line Device running" 
  #define MESSAGE3   " on STM3210C-EVAL   "
#elif defined (USE_STM32100E_EVAL)
  #define MESSAGE1   "STM32 HD Value Line " 
  #define MESSAGE2   " Device running on  " 
  #define MESSAGE3   "  STM32100E-EVAL    "   
#endif

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
 USART_InitTypeDef USART_InitStructure;
/* Private function prototypes -----------------------------------------------*/
#ifdef __GNUC__
/* With GCC/RAISONANCE, small printf (option LD Linker->Libraries->Small printf
   set to 'Yes') calls __io_putchar() */
#define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
#else
#define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
#endif /* __GNUC__ */

/* Private functions ---------------------------------------------------------*/
/*
! 数据
@ data_1
@ ...
@ data_2
! 设备安全
@ 设置密码
! 版本信息
@ 系统版本
@ 系统序列号
! 系统功能
@ 重启
@ 从PC传输数据
*/

void OLED_Display(uint8_t index);
void OLED_SetContent(uint8_t index, const char *str);
void MenuTask(void);
void LimitTask(void);
char *TitleName[] = {
  "!:",
  "@:",
  "#:",
  "M:",
  "P:"
};
//此层级菜单标示"!"
char *FirstLevelMenu[] = {
  "Data",
  "Safe",
  "Infomation",
  "System",
};
char *SecondLevelMenu_Data = "Samsung S20 Ultra display password";
char *SecondLevelMenu_Safe[] = {
  "Driver Password",
};
char *SecondLevelMenu_VersionInformation[] = {
  "USB VID",
  "System Version",
  "Serial Number",
};
char *SecondLevelMenu_System[] = {
  "Restart",
  "Get data from PC",
};
char *ThridLevelMenu = "zsdfll";
uint8_t MenuIndex[] = {
  0, 0, 0
};
char *Message[] = {
  "USB HID Send OK!",

};
char *Content = "Samsung S20 Ultra";
uint8_t DisplayIndex = 0;
uint8_t LevelMenuIndex = 0;
uint8_t DisplayIndexOld = 0;
int16_t DisplayLineX = -24;
uint8_t KeyIndex = 0;
uint8_t MenuFlag = 0;
uint8_t DateLen = 0;

/*
char tmp_str[256];
sprintf(tmp_str, "MenuIndex[%d] = %d", LevelMenuIndex, MenuIndex[LevelMenuIndex]);
Content = tmp_str;   
*/
/**
  * @brief  Main program.
  * @param  None
  * @retval None
  */
int main(void)
{
    /* 串口初始化 */
    USART_InitTypeDef USART_InitStructure;
    
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
    
    USART_InitStructure.USART_BaudRate = 9600;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    
    USART_Init(USART1, &USART_InitStructure);
    USART_Cmd(USART1, ENABLE);

    Key_Init();
    OLED_Init();

    OLED_SetContent(MenuIndex[LevelMenuIndex], FirstLevelMenu[MenuIndex[0]]);

    while (1)
    {
      if(Key_Reads(KeyIndex) == 1){
        if(KeyIndex == 0){
          MenuIndex[LevelMenuIndex]++;
          MenuFlag = 1;
        }
        if(KeyIndex == 1){
          if(LevelMenuIndex < 3) LevelMenuIndex++;
          MenuFlag = 1;   
        }
        if(KeyIndex == 2){
          if(LevelMenuIndex == 1) MenuIndex[2] = 0;
          if(LevelMenuIndex == 0) MenuIndex[1] = 0;
          if(LevelMenuIndex == 4) LevelMenuIndex = 2;          
          if(LevelMenuIndex > 0 && LevelMenuIndex < 3) LevelMenuIndex--;
          MenuFlag = 1;
        }
        if(KeyIndex == 3){
          if(LevelMenuIndex != DisplayIndexOld) DisplayIndexOld = LevelMenuIndex;
          LevelMenuIndex = 3;
          OLED_SetContent(LevelMenuIndex, Message[0]);
        }
        if(KeyIndex == 4){
          MenuIndex[LevelMenuIndex]--;
          MenuFlag = 1;
        }                
      }
      if(MenuFlag) MenuTask();
      LimitTask();
      OLED_Display(LevelMenuIndex);
      KeyIndex++;
      KeyIndex %= 5;
    }
}
/**
  * @brief  Retargets the C library printf function to the USART.
  * @param  None
  * @retval None
  */
PUTCHAR_PROTOTYPE
{
  /* Place your implementation of fputc here */
  /* e.g. write a character to the USART */
  USART_SendData(EVAL_COM1, (uint8_t) ch);

  /* Loop until the end of transmission */
  while (USART_GetFlagStatus(EVAL_COM1, USART_FLAG_TC) == RESET)
  {}

  return ch;
}
#ifdef  USE_FULL_ASSERT

/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t* file, uint32_t line)
{ 
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

  /* Infinite loop */
  while (1)
  {
  }
}
#endif

void OLED_SetContent(uint8_t index, const char *str){
  // if(DisplayIndex != index) DisplayIndexOld = DisplayIndex;
  DisplayLineX = -24;
  // DisplayIndex = index;
  Content = (char*)str;
}

void OLED_Display(uint8_t index){
  OLED_ShowString(-(DisplayLineX++), 0, Content, OLED_8X16);
  OLED_ShowString(0, 0, TitleName[index], OLED_8X16);
  OLED_Update();
  OLED_Clear();
  if(DisplayLineX == strlen(Content) * 8){
    DisplayLineX = -96;
    if(LevelMenuIndex == 3){
      LevelMenuIndex = DisplayIndexOld;
      MenuFlag = 1;
    }
  }
}

void MenuTask(void){
  MenuFlag = 0;
  if(LevelMenuIndex == 0){
    MenuIndex[LevelMenuIndex] %= 4;
    OLED_SetContent(MenuIndex[LevelMenuIndex], FirstLevelMenu[MenuIndex[LevelMenuIndex]]);
  }
  if(LevelMenuIndex == 1 && MenuIndex[0] == 0){
    OLED_SetContent(MenuIndex[LevelMenuIndex], SecondLevelMenu_Data);
  }
  if(LevelMenuIndex == 1 && MenuIndex[0] == 1){
    MenuIndex[LevelMenuIndex] %= 1;
    OLED_SetContent(MenuIndex[LevelMenuIndex], SecondLevelMenu_Safe[MenuIndex[LevelMenuIndex]]);
  }
  if(LevelMenuIndex == 1 && MenuIndex[0] == 2){
    MenuIndex[LevelMenuIndex] %= 3;
    OLED_SetContent(MenuIndex[LevelMenuIndex], SecondLevelMenu_VersionInformation[MenuIndex[LevelMenuIndex]]);
  }
  if(LevelMenuIndex == 1 && MenuIndex[0] == 3){
    MenuIndex[LevelMenuIndex] %= 2;
    OLED_SetContent(MenuIndex[LevelMenuIndex], SecondLevelMenu_System[MenuIndex[LevelMenuIndex]]);
  }
  if(LevelMenuIndex == 2 && MenuIndex[0] == 0){
    OLED_SetContent(MenuIndex[LevelMenuIndex], ThridLevelMenu);
    LevelMenuIndex = 4;
  }
  if(LevelMenuIndex == 4 &&  MenuIndex[0] == 0){
    OLED_SetContent(MenuIndex[LevelMenuIndex], ThridLevelMenu);
  }
}

void LimitTask(void){
  if(MenuIndex[0] == 1 && LevelMenuIndex == 2) LevelMenuIndex = 1;
  if(MenuIndex[0] == 2 && LevelMenuIndex == 2) LevelMenuIndex = 1;
  if(MenuIndex[0] == 3 && LevelMenuIndex == 2) LevelMenuIndex = 1;
}