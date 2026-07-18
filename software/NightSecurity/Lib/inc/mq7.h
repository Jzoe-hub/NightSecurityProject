/**********************************************************************
 * 文件名称： mq7.h
 * 功能描述： MQ-7 一氧化碳(CO)传感器驱动头文件 — 纯换算层
 * 硬件连接： AO → PA1 (ADC1_IN1, 规则组 Rank 2)
 * 采样方式： TIM3 每 100ms TRGO 触发 ADC1 扫描 3 通道, DMA 循环搬运,
 *           本驱动不碰任何硬件, 只做数学换算
 * 使用示例： float co_ppm = MQ7_RawToPPM(g_adc_raw[MQ7_ADC_INDEX]);
 ***********************************************************************/
#ifndef __MQ7_H
#define __MQ7_H

#include <stdint.h>

/* MQ-7 在 ADC DMA 缓冲区中的下标, 全部下标约定：
 *   [0] = MQ-2  (Rank1 / CH0 / PA0)
 *   [1] = MQ-7  (Rank2 / CH1 / PA1)   ← 本驱动
 *   [2] = 火焰  (Rank3 / CH4 / PA4)                                   */
#define MQ7_ADC_INDEX   1

float MQ7_RawToPPM(uint16_t raw);   // ADC 12位原始值(0~4095) → CO浓度(ppm)

#endif /* __MQ7_H */
