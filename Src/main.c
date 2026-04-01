/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
__ALIGN_BEGIN static uint8_t HID_MOUSE_ReportDesc[HID_MOUSE_REPORT_DESC_SIZE]  __ALIGN_END =
{
0x05, 0x01,        // Usage Page (Generic Desktop Ctrls)
0x09, 0x06,        // Usage (Keyboard)
0xA1, 0x01,        // Collection (Application)                                                   
0x05, 0x07,        //   Usage Page (Kbrd/Keypad)
0x19, 0xE0,        //   Usage Minimum (0xE0)
0x29, 0xE7,        //   Usage Maximum (0xE7)
0x15, 0x00,        //   Logical Minimum (0)
0x25, 0x01,        //   Logical Maximum (1)
0x75, 0x01,        //   Report Size (1)
0x95, 0x08,        //   Report Count (8)
0x81, 0x02,        //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
0x95, 0x01,        //   Report Count (1)
0x75, 0x08,        //   Report Size (8)
0x81, 0x01,        //   Input (Const,Array,Abs,No Wrap,Linear,Preferred State,No Null Position)
0x95, 0x05,        //   Report Count (5)
0x75, 0x01,        //   Report Size (1)
0x05, 0x08,        //   Usage Page (LEDs)
0x19, 0x01,        //   Usage Minimum (Num Lock)
0x29, 0x03,        //   Usage Maximum (Scroll Lock)
0x91, 0x02,        //   Output (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
0x95, 0x01,        //   Report Count (1)
0x75, 0x03,        //   Report Size (3)
0x91, 0x01,        //   Output (Const,Array,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
0x95, 0x06,        //   Report Count (6)
0x75, 0x08,        //   Report Size (8)
0x15, 0x00,        //   Logical Minimum (0)
0x25, 0x65,        //   Logical Maximum (101)
0x05, 0x07,        //   Usage Page (Kbrd/Keypad)
0x19, 0x00,        //   Usage Minimum (0x00)
0x29, 0x65,        //   Usage Maximum (0x65)
0x81, 0x00,        //   Input (Data,Array,Abs,No Wrap,Linear,Preferred State,No Null Position)
0xC0,              // End Collection
// 63 bytes
};  

#define HID_EPIN_SIZE                 0x08U	//Keyboard
#define HID_MOUSE_REPORT_DESC_SIZE    63U	//Keyboard

The data lenght is 256Byte
Sector  -> 16 Data
Block   -> 256 Data
Chip    -> 131072 Data

No        : 4Byte
Platform  : 32Byte
Account   : 32Byte
Password  : 128Byte

Platfrom(32Byte):Github
Account(32Byte):User@exmple.com
Password(AES - 128Byte):6175dbaffbfa020634fef45d79b3d392986ab04f577bd23551e231fe13b7bc2f986ab04f577bd23551e231fe13b7bc2f986ab04f577bd23551e231fe13b7bc2f


-----------------------------
No        : 4Byte
Platform  : 64Byte
Account   : 64Byte
Password  : 32Byte
Data Lenght -> 196Byte

	*/
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "spi.h"
#include "tim.h"
#include "usart.h"
#include "usb_device.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include "OLED.h"
#include "Key.h"
#include "oled_menu.h"
#include "w25q256.h"
#include "data_storage.h"
#include "sha_256.h"
#include "InternalFlash.h"
#include "usbd_cdc_if.h"
#include "aes_ecb.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
// ¶¨Òå´æ´¢µØÖ·
#define UNIQUE_ID_BASE_ADDRESS 0x1FFFF7E8
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

uint32_t lastSelectTime = 0;
uint32_t tick = 0;
uint16_t softwd = 0;

uint8_t DriverLock = 1;
uint8_t UnlockPassword[8] = {0, 0, 0, 0, 0, 0, 0, 0};
uint8_t UnlockPasswordIndex = 0;
uint8_t ResetPassword = 0;
uint8_t read_data[32];

uint8_t showMenu_Flag = 1;
uint8_t showSystemVersion_Flag = 0;
uint8_t showSystemSerialNumber_Flag = 0;

uint8_t GetDataForPC_Flag = 0;
uint8_t GetDataForPC_DataIndex = 0;

uint8_t cdc_buff_index = 0;
uint8_t cdc_buff[64];

uint32_t lastcurri_m = 0;
uint8_t ContentIndex_m = 0;
uint8_t ContentLenght_m = 0;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_USART1_UART_Init();
  MX_USB_DEVICE_Init();
  MX_TIM2_Init();
  MX_SPI1_Init();
  /* USER CODE BEGIN 2 */

	OLED_Init();
	Key_Init();
  W25qxx_Init();
  

  HAL_Delay(3000);

	printf("Initialize done!\r\n"); 
	printf("HCLK Freq = %d\r\n", HAL_RCC_GetHCLKFreq());
	printf("PCLK1 Freq = %d\r\n", HAL_RCC_GetPCLK1Freq());
	printf("PCLK2 Freq = %d\r\n", HAL_RCC_GetPCLK2Freq());
	printf("SYSCLK Freq = %d\r\n", HAL_RCC_GetSysClockFreq());

	usb_printf("Initialize done!\r\n"); 
	usb_printf("HCLK Freq = %d\r\n", HAL_RCC_GetHCLKFreq());
	usb_printf("PCLK1 Freq = %d\r\n", HAL_RCC_GetPCLK1Freq());
	usb_printf("PCLK2 Freq = %d\r\n", HAL_RCC_GetPCLK2Freq());
	usb_printf("SYSCLK Freq = %d\r\n", HAL_RCC_GetSysClockFreq());  

	HAL_TIM_Base_Start_IT(&htim2); 	

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  OLED_Clear();
  OLED_ShowString(0, 0, "System Start", OLED_8X16);
  OLED_Update();
  HAL_Delay(500);

  INTFLASH_ReadBuffer(INTFLASH_USER_START_ADDR, read_data, sizeof(hash));

  //Lock driver
  while(DriverLock){
    Handle_Buttons();

    if(DriverLock){
      PasswordUI();
    }

    softwd = 0;
  }

  HAL_Delay(500);
  MenuInit(MainMenu);
  OLED_Clear();

  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  Handle_Buttons();

  if(showSystemVersion_Flag){
    OLED_ShowString(0, 0, "Ver1.0.0.00", OLED_8X16);
    OLED_Update();
  }
  if(showSystemSerialNumber_Flag){
    OLED_ShowHexNum(0, 0, *(volatile uint32_t *)(UNIQUE_ID_BASE_ADDRESS), 8, OLED_6X8);
    OLED_ShowHexNum(48, 0, *(volatile uint32_t *)(UNIQUE_ID_BASE_ADDRESS + 4), 8, OLED_6X8);
    OLED_ShowHexNum(24, 8, *(volatile uint32_t *)(UNIQUE_ID_BASE_ADDRESS + 8), 8, OLED_6X8);    
    OLED_Update();
  }
  if(GetDataForPC_Flag){
    uint32_t nowcurri = HAL_GetTick();
    if(nowcurri > lastcurri_m + 1000){
        lastcurri_m = nowcurri;
        ContentIndex_m--;
    }     
    switch (GetDataForPC_DataIndex)
    {
    case 0:
      if(ContentIndex_m == 0 || ContentLenght_m == 0) ContentLenght_m = ContentIndex_m = strlen((cdc_buff));
      OLED_ShowString(0, 0, (cdc_buff + ContentLenght_m - ContentIndex_m), OLED_6X8);
      OLED_ShowString(0, 8, "Platfrom", OLED_6X8);
      OLED_Update();
      OLED_Clear();
      break;
    case 1:
      if(ContentIndex_m == 0 || ContentLenght_m == 0) ContentLenght_m = ContentIndex_m = strlen((cdc_buff));
      OLED_ShowString(0, 0, (cdc_buff + ContentLenght_m - ContentIndex_m), OLED_6X8);
      OLED_ShowString(0, 8, "Account", OLED_6X8);
      OLED_Update();
      OLED_Clear();
      break;
    case 2:
      if(ContentIndex_m == 0 || ContentLenght_m == 0) ContentLenght_m = ContentIndex_m = strlen((cdc_buff));
      OLED_ShowString(0, 0, (cdc_buff + ContentLenght_m - ContentIndex_m), OLED_6X8);
      OLED_ShowString(0, 8, "Password", OLED_6X8);
      OLED_Update();
      OLED_Clear();
      break;            
    
    default:
      break;
    }
  }
  if(showMenu_Flag) MenuDisplay();
  if(ResetPassword){
    if(DriverLock == 1){
      PasswordUI();
    }    
  }
  
    // USBD_HID_SendReport(&hUsbDeviceFS, KeyboardBuff, sizeof(KeyboardBuff) / sizeof(KeyboardBuff[0]));
  HAL_Delay(10);
  softwd = 0;
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USB;
  PeriphClkInit.UsbClockSelection = RCC_USBCLKSOURCE_PLL_DIV1_5;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

void Handle_Buttons(void) {
    static uint32_t lastKeyTime = 0;
    uint32_t currentTime = HAL_GetTick();
    if (currentTime - lastKeyTime < 200) return;    
    if(DriverLock){
      //Left
      if (KeyInputBuff[0].KeyState != KEY_UP) {
          if(UnlockPasswordIndex > 0) UnlockPasswordIndex--;
          lastKeyTime = currentTime;
      }
      // Up
      if (KeyInputBuff[1].KeyState != KEY_UP) {
          if(UnlockPassword[UnlockPasswordIndex] < 9) UnlockPassword[UnlockPasswordIndex]++;
          lastSelectTime = currentTime;
          lastKeyTime = currentTime;
      }
      // OK
      if (KeyInputBuff[2].KeyState != KEY_UP) {
          uint32_t tmp_pwd = UnlockPassword[0] * 10 ^ 7;
          tmp_pwd += UnlockPassword[1] * 10 ^ 6;
          tmp_pwd += UnlockPassword[2] * 10 ^ 5;
          tmp_pwd += UnlockPassword[3] * 10 ^ 4;
          tmp_pwd += UnlockPassword[4] * 10 ^ 3;
          tmp_pwd += UnlockPassword[5] * 10 ^ 2;
          tmp_pwd += UnlockPassword[6] * 10 ^ 1;
          tmp_pwd += UnlockPassword[7];
          SHA256_Init(&ctx);
          SHA256_Update(&ctx, &tmp_pwd, 8);
          SHA256_Final(&ctx, hash);
          VerifyPassword();
          lastKeyTime = currentTime;
      }
      // Right
      if (KeyInputBuff[3].KeyState != KEY_UP) {
          if(UnlockPasswordIndex < 7) UnlockPasswordIndex++;
          lastKeyTime = currentTime;
      }
      // Down
      if (KeyInputBuff[4].KeyState != KEY_UP) {
          if(UnlockPassword[UnlockPasswordIndex] > 0) UnlockPassword[UnlockPasswordIndex]--;
          lastSelectTime = currentTime;
          lastKeyTime = currentTime;
      }
    }
    else{
      //Left
      if (KeyInputBuff[0].KeyState != KEY_UP) {
        usb_printf("Left Click!\r\n");
        Menu_Back();
        lastKeyTime = currentTime;
      }
      //Up
      if (KeyInputBuff[1].KeyState != KEY_UP) {
        Menu_Up();
        usb_printf("Up Click!\r\n");        
        lastSelectTime = currentTime;
        lastKeyTime = currentTime;
      }
      // OK
      if (KeyInputBuff[2].KeyState != KEY_UP) {
        usb_printf("OK Click!\r\n");  
          if(showMenu_Flag) Menu_Enter();
          lastKeyTime = currentTime;
      }
      //Right
      if (KeyInputBuff[3].KeyState != KEY_UP) {
        usb_printf("Right Click!\r\n");  
          StatusSwitch();
          lastKeyTime = currentTime;
      }
      //Down
      if (KeyInputBuff[4].KeyState != KEY_UP) {
        usb_printf("Down Click!\r\n");  
          Menu_Down();
          lastSelectTime = currentTime;
          lastKeyTime = currentTime;
      }
    }
}

void PasswordUI(void){
  OLED_ShowChar(UnlockPasswordIndex * 12, 0, '+', OLED_6X8);
  //0 12 24 36 48 60 72 84
  OLED_Printf(0, 8, OLED_6X8, "%01d %01d %01d %01d %01d %01d %01d %01d", UnlockPassword[0], 
    UnlockPassword[1], UnlockPassword[2], UnlockPassword[3], UnlockPassword[4], UnlockPassword[5], 
    UnlockPassword[6], UnlockPassword[7]);

    OLED_Update();
    OLED_Clear();    
}

void VerifyPassword(void){
  /* ï¿½ï¿½Ê¼ï¿½ï¿½ */
  INTFLASH_Init();
  if(ResetPassword && DriverLock == 1) INTFLASH_EraseUserArea();
  uint8_t result = 0;

  uint32_t hasPWD = 0x00;
  if (result == INTFLASH_OK) {
      /* Ð´ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ */
      
      for(uint8_t i = 0;i < 32;i++) if(read_data[i] == 0xFF) hasPWD = hasPWD | (0x80000000 >> i);
      // for(uint8_t i = 0;i < 32;i++) usb_printf("%02X", read_data[i]);
      usb_printf("\r\n");
      if(hasPWD == 0xFFFFFFFF) result = INTFLASH_WriteBuffer(INTFLASH_USER_START_ADDR, hash, sizeof(hash));
      if (result == INTFLASH_OK) {
          /* ï¿½ï¿½È¡ï¿½ï¿½ï¿½ï¿½ */
          INTFLASH_ReadBuffer(INTFLASH_USER_START_ADDR, read_data, sizeof(hash));
          /* ï¿½ï¿½Ö¤ */
          if (INTFLASH_VerifyData(INTFLASH_USER_START_ADDR, hash, sizeof(hash))) {
              /* ï¿½É¹ï¿½ */
              if(!ResetPassword){
                for(uint8_t i = 0;i < 8;i++) UnlockPassword[i] = 0;
                OLED_ShowString(0,0,"  WELCOME  ", OLED_8X16);
                OLED_Update();
                HAL_Delay(500);
              }
              else{
                OLED_ShowString(0,0,"SET PWD OK!", OLED_8X16);
                OLED_Update();
                HAL_Delay(500);                
                Function_Reboot();
              }
              DriverLock = 0;
          }
      }
  }
  
  /* ï¿½ï¿½ï¿½ï¿½Flash */
  INTFLASH_DeInit();    
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	if(htim == &htim2)
	{
		KeyScan(KeyInputBuff);
		KeyScan(KeyInputBuff + 1);
		KeyScan(KeyInputBuff + 2);
		KeyScan(KeyInputBuff + 3);
		KeyScan(KeyInputBuff + 4);

		softwd++;
		if(softwd > 10000){
      Function_Reboot();
		}
	}
}

void StatusSwitch(void){
  if(showSystemVersion_Flag){
    showSystemVersion_Flag = 0;
    showMenu_Flag = 1;
  }
  if(showSystemSerialNumber_Flag){
    showSystemSerialNumber_Flag = 0;
    showMenu_Flag = 1;
  } 
  if(GetDataForPC_Flag){
    GetDataForPC_DataIndex++;
    memset(cdc_buff, '\0', 64);
    if(GetDataForPC_DataIndex == 3){
      GetDataForPC_DataIndex = 0;
      showMenu_Flag = 1;
      GetDataForPC_Flag = 0;
    }
  }

    // ²âÊÔ²»Í¬µÄÊý¾Ý³¤¶È
    const char *test_cases[] = {
        "00112233445566778899AABBCCDDEEFF",           // 16×Ö½Ú£¬ÕýºÃÒ»¸ö¿é
        "00112233445566778899AABBCCDDEEFF0011",       // 17×Ö½Ú£¬ÐèÒªÌî³ä
        "00112233445566778899AABBCCDDEEFF00112233",   // 18×Ö½Ú£¬ÐèÒªÌî³ä
        "0011223344556677",                           // 8×Ö½Ú£¬ÐèÒªÌî³ä
        "48656C6C6F20576F726C64"                      // "Hello World" 11×Ö½Ú
    };
    
    const char *key = "15e2b0d3c33891ebb0f1ef609ec419420c20e320ce94c65fbc8c3312448eb225";
    
    for (int i = 0; i < 5; i++) {
        char ciphertext[256] = {0};
        char decrypted[256] = {0};
        
        usb_printf("\n========== Test Case %d ==========\n", i + 1);
        usb_printf("Original Data: %s\n", test_cases[i]);
        usb_printf("Original Length: %d bytes\n", (int)strlen(test_cases[i]) / 2);
        usb_printf("Key: %s\n", key);
        
        // ¼ÓÃÜ
        int ret = AES_ECB_Encrypt(test_cases[i], key, ciphertext, sizeof(ciphertext));
        if (ret == AES_SUCCESS) {
            usb_printf("Encrypted: %s\n", ciphertext);
            usb_printf("Encrypted Length: %d bytes\n", (int)strlen(ciphertext) / 2);
            
            // ½âÃÜ
            ret = AES_ECB_Decrypt(ciphertext, key, decrypted, sizeof(decrypted));
            if (ret == AES_SUCCESS) {
                usb_printf("Decrypted: %s\n", decrypted);
                usb_printf("Decrypted Length: %d bytes\n", (int)strlen(decrypted) / 2);
                
                // ÑéÖ¤½á¹û
                if (strcmp(decrypted, test_cases[i]) == 0) {
                    usb_printf("Result: SUCCESS\n");
                } else {
                    usb_printf("Result: FAILED - Data mismatch\n");
                    usb_printf("Expected: %s\n", test_cases[i]);
                    usb_printf("Got: %s\n", decrypted);
                }
            } else {
                usb_printf("Decryption failed with code: %d\n", ret);
            }
        } else {
            usb_printf("Encryption failed with code: %d\n", ret);
        }
    }
    
    // ²âÊÔ»º³åÇø´óÐ¡¼ì²é
    usb_printf("\n========== Buffer Size Test ==========\n");
    char small_buffer[10] = {0};
    int ret = AES_ECB_Encrypt("00112233", key, small_buffer, sizeof(small_buffer));
    if (ret == AES_ERR_BUFFER_TOO_SMALL) {
        usb_printf("Buffer size check works correctly\n");
    }  

}

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
