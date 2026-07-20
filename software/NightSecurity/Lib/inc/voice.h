/**********************************************************************
 * 文件名称： voice.h
 * 功能描述： JQ8900 语音模块驱动头文件
 * 硬件连接： IO1 → PB6 (低脉冲触发), IO2 → PB7, IO3 → PB3, IO4 → PB4
 *           BUSY → PB12 (GPIO 输入, 高=播放中, 低=空闲)
 * 来    源： 小暗XiaoAn (CSDN), SPL→HAL 移植
 * 说    明： 支持两种控制模式:
 *           1. IO 脉冲触发 (当前硬件配置) — 拉低 200ms 触发预设曲目
 *           2. 单线串口 (需额外配 DATA 引脚) — 可指定曲目号播放
 ***********************************************************************/
#ifndef __VOICE_H
#define __VOICE_H

#include "stm32f1xx_hal.h"

/* ---- 触发时长 ---- */
#define VOICE_TRIGGER_MS        200     /* 低脉冲持续时间 ms      */

/* ---- IO 触发引脚 ---- */
#define VOICE_IO1_PORT          GPIOB
#define VOICE_IO1_PIN           GPIO_PIN_6
#define VOICE_IO2_PORT          GPIOB
#define VOICE_IO2_PIN           GPIO_PIN_7
#define VOICE_IO3_PORT          GPIOB
#define VOICE_IO3_PIN           GPIO_PIN_3
#define VOICE_IO4_PORT          GPIOB
#define VOICE_IO4_PIN           GPIO_PIN_4

/* ---- BUSY 引脚 ---- */
#define VOICE_BUSY_PORT         GPIOB
#define VOICE_BUSY_PIN          GPIO_PIN_12

/* ==================== IO 脉冲触发（当前硬件） ==================== */

void Voice_IO1_Trigger(void);       /* 触发曲目 1 (火灾报警)   */
void Voice_IO2_Trigger(void);       /* 触发曲目 2 (入侵报警)   */
void Voice_IO3_Trigger(void);       /* 触发曲目 3              */
void Voice_IO4_Trigger(void);       /* 触发曲目 4              */
uint8_t Voice_IsBusy(void);         /* 1=播放中, 0=空闲        */

#endif /* __VOICE_H */
