/**********************************************************************
 * 文件名称： fire.h
 * 功能描述： 火焰传感器驱动头文件（同时支持 AO 模拟口 + DO 数字口）
 * 硬件连接： AO → PA4 (ADC1_IN4, 规则组 Rank 3)
 *           DO → PA11 (GPIO 上拉输入, 0=有火 1=无火)
 * 采样方式： TIM3 每 100ms TRGO 触发 ADC1 扫描 3 通道, DMA 循环搬运,
 *           本驱动不碰硬件, 只做数据处理
 * 使用示例： uint16_t intensity = Fire_GetIntensity(g_adc_raw[FIRE_ADC_INDEX]);
 *           uint8_t  detected  = Fire_IsDetected();
 ***********************************************************************/
#ifndef __FIRE_H
#define __FIRE_H

#include <stdint.h>

/* AO 火焰传感器在 ADC DMA 缓冲区中的下标, 全部下标约定：
 *   [0] = MQ-2  (Rank1 / CH0 / PA0)
 *   [1] = MQ-7  (Rank2 / CH1 / PA1)
 *   [2] = 火焰  (Rank3 / CH4 / PA4)   ← 本驱动                         */
#define FIRE_ADC_INDEX      2

/* DO 数字口引脚 */
#define FIRE_DO_PORT        GPIOA
#define FIRE_DO_PIN         GPIO_PIN_11

uint16_t Fire_GetIntensity(uint16_t raw);   // AO口：ADC 原始值 → 火焰强度 (0~4095, 越大越强)
uint8_t  Fire_IsDetected(void);             // DO 数字口: 0=有火, 1=无火

#endif /* __FIRE_H */
