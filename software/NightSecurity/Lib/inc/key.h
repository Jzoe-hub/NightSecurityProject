/**********************************************************************
 * 文件名称： key.h
 * 功能描述： 按键驱动头文件（3 个独立按键, 上拉输入, 按下=低电平）
 * 硬件连接： KEY1→PB13, KEY2→PB14, KEY3→PB15 (GPIO 上拉输入)
 ***********************************************************************/
#ifndef __KEY_H
#define __KEY_H

#include "stm32f1xx_hal.h"

#define KEY1_PORT           GPIOB
#define KEY1_PIN            GPIO_PIN_13
#define KEY2_PORT           GPIOB
#define KEY2_PIN            GPIO_PIN_14
#define KEY3_PORT           GPIOB
#define KEY3_PIN            GPIO_PIN_15

uint8_t Key1_IsPressed(void);               /* 1=按下, 0=松开          */
uint8_t Key2_IsPressed(void);
uint8_t Key3_IsPressed(void);

#endif /* __KEY_H */
