/**********************************************************************
 * 文件名称： rgb.h
 * 功能描述： RGB LED 驱动头文件（共阳极, 低电平亮）
 * 硬件连接： R→PB0, G→PB1, B→PA7 (推挽输出, 高=灭 低=亮)
 ***********************************************************************/
#ifndef __RGB_H
#define __RGB_H

#include "stm32f1xx_hal.h"

#define RGB_R_PORT          GPIOB
#define RGB_R_PIN           GPIO_PIN_0
#define RGB_G_PORT          GPIOB
#define RGB_G_PIN           GPIO_PIN_1
#define RGB_B_PORT          GPIOA
#define RGB_B_PIN           GPIO_PIN_7

void RGB_Set(uint8_t r, uint8_t g, uint8_t b);  /* 1=亮, 0=灭 (非 PWM) */
void RGB_Off(void);                              /* 全灭                  */

#endif /* __RGB_H */
