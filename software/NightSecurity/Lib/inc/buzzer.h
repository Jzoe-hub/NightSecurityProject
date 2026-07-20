/**********************************************************************
 * 文件名称： buzzer.h
 * 功能描述： 蜂鸣器驱动头文件（有源蜂鸣器, GPIO 直驱）
 * 硬件连接： I/O → PA5 (推挽输出, 高电平响, 低电平停)
 ***********************************************************************/
#ifndef __BUZZER_H
#define __BUZZER_H

#include "stm32f1xx_hal.h"

#define BUZZER_PORT         GPIOA
#define BUZZER_PIN          GPIO_PIN_5

void    Buzzer_On(void);                    /* 蜂鸣器响                */
void    Buzzer_Off(void);                   /* 蜂鸣器停                */
void    Buzzer_Beep(uint16_t ms);           /* 短鸣 N 毫秒 (阻塞)      */

#endif /* __BUZZER_H */
