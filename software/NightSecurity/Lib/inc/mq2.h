/**********************************************************************
 * 文件名称： mq2.h
 * 功能描述： MQ-2 烟雾/可燃气体传感器驱动头文件 — 纯换算层
 * 硬件连接： AO → PA0 (ADC1_IN0, 规则组 Rank 1)
 * 采样方式： TIM3 每 100ms TRGO 触发 ADC1 扫描 3 通道, DMA 循环搬运,
 *           本驱动不碰任何硬件, 只做数学换算
 * 使用示例： float smoke_ppm = MQ2_RawToPPM(g_adc_raw[MQ2_ADC_INDEX]);
 ***********************************************************************/
#ifndef __MQ2_H
#define __MQ2_H

#include <stdint.h>

/* MQ-2 在 ADC DMA 缓冲区中的下标, 全部下标约定：
 *   [0] = MQ-2  (Rank1 / CH0 / PA0)   ← 本驱动
 *   [1] = MQ-7  (Rank2 / CH1 / PA1)
 *   [2] = 火焰  (Rank3 / CH4 / PA4)                                   */
#define MQ2_ADC_INDEX   0

float MQ2_RawToPPM(uint16_t raw);   // ADC 12位原始值(0~4095) → 浓度(ppm), 公式常数待更新

#endif /* __MQ2_H */
