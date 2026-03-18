#include "Key.h"
#include "stm32f10x.h"
#include "Delay.h"

#define KEY_NUM 5

GPIO_TypeDef *GPIO_Port_x[5] = {
    GPIOA,
    GPIOA,
    GPIOA,
    GPIOA,
    GPIOA
};
uint16_t GPIO_Pin_x[5] = {
    GPIO_Pin_8,
    GPIO_Pin_9,
    GPIO_Pin_10,
    GPIO_Pin_11,
    GPIO_Pin_12
};

void Key_Init(){
    uint8_t index = 0;
    GPIO_InitTypeDef tmp;
    
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    
    for(index = 0;index < KEY_NUM;index++){
        tmp.GPIO_Mode = GPIO_Mode_IPU;
        tmp.GPIO_Pin = GPIO_Pin_x[index];
        tmp.GPIO_Speed = GPIO_Speed_2MHz;
        GPIO_Init(GPIO_Port_x[index], &tmp);
    }

}

uint8_t Key_Reads(uint8_t index){
    uint8_t res = 0;

    if(GPIO_ReadInputDataBit(GPIO_Port_x[index], GPIO_Pin_x[index]) == 0){
        Delay_ms(20);
        while(GPIO_ReadInputDataBit(GPIO_Port_x[index], GPIO_Pin_x[index]) == 0) res = 1;
        Delay_ms(20);
    }

    return res;
}