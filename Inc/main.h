/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
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
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f1xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */
extern uint16_t softwd;
extern uint8_t DriverLock;
extern uint8_t ResetPassword;

extern uint8_t showMenu_Flag;
extern uint8_t showSystemVersion_Flag;
extern uint8_t showSystemSerialNumber_Flag;
extern uint8_t GetDataForPC_Flag;
extern uint8_t GetDataForPC_DataIndex;

extern uint8_t cdc_buff_index;
extern uint8_t cdc_buff[64];

extern uint8_t ContentIndex_m;
/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */
void Handle_Buttons(void);
void PasswordUI(void);
void VerifyPassword(void);
void StatusSwitch(void);
/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define W25Q256_CS_Pin GPIO_PIN_4
#define W25Q256_CS_GPIO_Port GPIOA
#define OLED_RST_Pin GPIO_PIN_2
#define OLED_RST_GPIO_Port GPIOB
#define OLED_SCK_Pin GPIO_PIN_10
#define OLED_SCK_GPIO_Port GPIOB
#define OLED_SDA_Pin GPIO_PIN_11
#define OLED_SDA_GPIO_Port GPIOB
#define KEY_5_Pin GPIO_PIN_12
#define KEY_5_GPIO_Port GPIOB
#define KEY_4_Pin GPIO_PIN_13
#define KEY_4_GPIO_Port GPIOB
#define KEY_3_Pin GPIO_PIN_14
#define KEY_3_GPIO_Port GPIOB
#define KEY_2_Pin GPIO_PIN_15
#define KEY_2_GPIO_Port GPIOB
#define KEY_1_Pin GPIO_PIN_8
#define KEY_1_GPIO_Port GPIOA

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
