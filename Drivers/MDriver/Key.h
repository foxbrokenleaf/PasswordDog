#ifndef __KEY_H__
#define __KEY_H__

#include "stm32f1xx_hal.h"

typedef enum{
	KEY_DOWN = 0U,
	KEY_HOLD,
	KEY_UP
}KeyActionType;
typedef struct{
	GPIO_TypeDef *GPIO_Port;
	uint16_t GPIO_Pin;
	KeyActionType KeyState;
	uint16_t HoldCnt;
}ButtonType;

extern unsigned char KeepClick;
extern unsigned char KeyState[5];
extern ButtonType KeyInputBuff[];

void Key_Init();
void KeyScan(ButtonType *bt);

#endif