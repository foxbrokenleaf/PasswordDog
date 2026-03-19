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
#define UID_BASE 0x1FFFF7E8
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
void POSTTask(void);
void OLED_Display(uint8_t index);
void OLED_SetContent(uint8_t index, const char *str);
void MenuTask(void);
void LimitTask(void);
void ExecTask(void);

void TIM2_Init(void);
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
  "Test",
};
char *ThridLevelMenu = "zsdfll";
uint8_t MenuIndex[] = {
  0, 0, 0
};
char *Message[] = {
  "USB HID Send OK!",

};
//大更新  YYYYMM Ver.
//小更新  MM.DD.YYYY Update
char *SystemVersion = "202603 Ver.";
char *SystemSerialNumber = "XXXXXXXXXXXX";
char *t_SystemSafeSetPassword = "";
char *Content = "Samsung S20 Ultra";
uint8_t DisplayIndex = 0;
uint8_t LevelMenuIndex = 0;
uint8_t DisplayIndexOld = 0;
int16_t DisplayLineX = -24;
uint8_t KeyIndex = 0;
uint8_t MenuFlag = 0;
uint8_t DateLen = 0;
uint8_t TimerCounter = 0;
uint8_t Click_5sec_flag = 0;
uint8_t WorkLed = 0;
uint8_t StatePassword = 1;
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

    {
      GPIO_InitTypeDef tmp;
      RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);      
      tmp.GPIO_Mode = GPIO_Mode_Out_PP;
      tmp.GPIO_Pin = GPIO_Pin_13;
      tmp.GPIO_Speed = GPIO_Speed_2MHz;
      GPIO_Init(GPIOC, &tmp);      
    }

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

    TIM2_Init();

    Key_Init();
    OLED_Init();

    do{
      OLED_Init();
      POSTTask();
    }while(0);

    OLED_SetContent(MenuIndex[LevelMenuIndex], FirstLevelMenu[MenuIndex[0]]);

    while (1)
    {
      if(Key_Reads(KeyIndex) == 1){
        // 设置密码
        // if(LevelMenuIndex == 2 && MenuIndex[0] == 1){
        if(0){
          char tmp_str[256];
          strcpy(tmp_str, t_SystemSafeSetPassword);
          if(KeyIndex == 0){
            strcat(tmp_str, "D");
            MenuFlag = 1;
          }
          if(KeyIndex == 1){
            strcat(tmp_str, "R");
            MenuFlag = 1;   
          }
          if(KeyIndex == 2){
            strcat(tmp_str, "L");              
            MenuFlag = 1;
          }
          if(KeyIndex == 3){
            
            MenuFlag = 1;
          }
          if(KeyIndex == 4){
            strcat(tmp_str, "U"); 
            MenuFlag = 1;
          }   
          t_SystemSafeSetPassword = tmp_str;         
        }else{  //正常模式
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
            ExecTask();
          }
          if(KeyIndex == 4){
            MenuIndex[LevelMenuIndex]--;
            MenuFlag = 1;
          }  
        }              
      }
      if(MenuFlag) MenuTask();
      if(Click_5sec_flag) ExecTask();
      LimitTask();
      OLED_Display(LevelMenuIndex);
      KeyIndex++;
      KeyIndex %= 5;
      KeepClick = 0;
      if(WorkLed){
        GPIO_WriteBit(GPIOC, GPIO_Pin_13, Bit_SET);
        WorkLed = 0;
      }else{
        GPIO_WriteBit(GPIOC, GPIO_Pin_13, Bit_RESET);
        WorkLed = 1;
      }
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
    MenuIndex[LevelMenuIndex] %= 3;
    OLED_SetContent(MenuIndex[LevelMenuIndex], SecondLevelMenu_System[MenuIndex[LevelMenuIndex]]);
  }
  if(LevelMenuIndex == 2 && MenuIndex[0] == 1){
    OLED_SetContent(MenuIndex[LevelMenuIndex], t_SystemSafeSetPassword);
  }  
  if(LevelMenuIndex == 2 && MenuIndex[0] == 0){
    OLED_SetContent(MenuIndex[LevelMenuIndex], ThridLevelMenu);
    LevelMenuIndex = 4;
  }
  if(LevelMenuIndex == 2 && MenuIndex[0] == 2 && MenuIndex[1] == 1){
    OLED_SetContent(MenuIndex[LevelMenuIndex], SystemVersion);
  } 
  if(LevelMenuIndex == 2 && MenuIndex[0] == 2 && MenuIndex[1] == 2){
    char tmp_str[128];
    sprintf(tmp_str, "%04X%04X%04X", (uint32_t)(*((uint32_t *)UID_BASE)),
                                      (uint32_t)(*((uint32_t *)(UID_BASE + 0x04))),
                                      (uint32_t)(*((uint32_t *)(UID_BASE + 0x14))));
    SystemSerialNumber = tmp_str;
    OLED_SetContent(MenuIndex[LevelMenuIndex], SystemSerialNumber);
  }     
  if(LevelMenuIndex == 4 &&  MenuIndex[0] == 0){
    OLED_SetContent(MenuIndex[LevelMenuIndex], ThridLevelMenu);
  }
  if(LevelMenuIndex == 2 && MenuIndex[0] == 3){
    char tmp_str[256];
    sprintf(tmp_str, "TimerCounter = %d", TimerCounter);
    Content = tmp_str;
    OLED_SetContent(LevelMenuIndex, Content);
  }  
}

void LimitTask(void){
  // if(MenuIndex[0] == 1 && LevelMenuIndex == 2) LevelMenuIndex = 1;
  // if(MenuIndex[0] == 2 && LevelMenuIndex == 2) LevelMenuIndex = 1;
  // if(MenuIndex[0] == 3 && LevelMenuIndex == 2) LevelMenuIndex = 1;
}

void POSTTask(void){
  //OLED Display POST
  for(uint8_t i = 0;i < 96;i++){
    for(uint8_t j = 0;j < 16;j++) OLED_DrawPoint(i, j);
    OLED_Update();
  }
}

void ExecTask(void){
  if(LevelMenuIndex == 1 && MenuIndex[0] == 3 && MenuIndex[1] == 0){
    __set_FAULTMASK(1);
    NVIC_SystemReset();
  }
  if(LevelMenuIndex == 4 && MenuIndex[0] == 0){
    LevelMenuIndex = 3;
    OLED_SetContent(LevelMenuIndex, Message[0]);
  }
  if(LevelMenuIndex == 2 && MenuIndex[0] == 1){
    LevelMenuIndex = 1;
  }
}

// 定时器2初始化函数 - 1秒中断
void TIM2_Init(void)
{
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    NVIC_InitTypeDef NVIC_InitStructure;
    
    // 1. 使能TIM2时钟
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
    
    // 2. 计算定时器参数
    // 系统时钟72MHz，APB1预分频器为2，TIM2时钟为72MHz
    // 定时时间 = (prescaler + 1) * (period + 1) / TIMxCLK
    // 1秒 = (7200 - 1 + 1) * (10000 - 1 + 1) / 72MHz = 7200 * 10000 / 72000000 = 1秒
    
    // 预分频值：7200-1，得到10kHz计数频率 (72MHz / 7200 = 10kHz)
    TIM_TimeBaseStructure.TIM_Prescaler = 7200 - 1;
    
    // 自动重装载值：10000-1，每10000次计数产生一次中断 (10kHz / 10000 = 1Hz)
    TIM_TimeBaseStructure.TIM_Period = 1000 - 1;
    
    // 设置时钟分频
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    
    // 设置计数模式为向上计数
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
    
    // 初始化TIM2
    TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);
    
    // 3. 清除更新中断标志
    TIM_ClearFlag(TIM2, TIM_FLAG_Update);
    
    // 4. 使能更新中断
    TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);
    
    // 5. 配置NVIC中断优先级
    NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;  // 抢占优先级
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;         // 子优先级
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
    
    // 6. 使能TIM2
    TIM_Cmd(TIM2, ENABLE);
}

// 定时器2中断服务函数
void TIM2_IRQHandler(void)
{
    // 检查是否为更新中断
    if (TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET)
    {
        // 清除中断标志位
        TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
        
        // 在这里添加每1秒要执行的代码
        // 例如：翻转LED或执行其他任务.
        if(KeepClick){
          if(TimerCounter >= 5){
            __set_FAULTMASK(1);
            NVIC_SystemReset();            
          }
          TimerCounter++;
          KeepClick = 0;
        }
        else TimerCounter = 0;
        
    }
}