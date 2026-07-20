/**********************************************************************
 * 文件名称： radar.h
 * 功能描述： HLK-LD2410C 毫米波雷达传感器驱动头文件
 * 硬件连接： TX → PA10 (USART1_RX), RX → PA9 (USART1_TX)
 *            OT1 → PB5 (GPIO 浮空输入, 有人=高 无人=低)
 * 协议格式： 帧头 F4 F3 F2 F1 ... 帧尾 F5
 * 波特率：   25600bps, 8N1
 * 来    源： 基于 GitHub CLi321/sensor-driver-code, 保留原作者解析逻辑,
 *            UART 层由 SPL(UART4+PC10/11) 移植为 HAL(USART1+DMA+PA9/10)
 ***********************************************************************/
#ifndef __RADAR_H
#define __RADAR_H

#include "stm32f1xx_hal.h"
#include <stdbool.h>

/* ---- 缓冲区大小 ---- */
#define BUFF_MAX_LEN_LD2410C       300     /* UART DMA 接收缓冲区 */
#define RECEIVE_MAX_LEN_LD2410C   1024     /* 帧处理缓冲区         */

/* ---- UART DMA 接收缓冲（对接 huart1） ---- */
typedef struct
{
	uint8_t  rx_buff[BUFF_MAX_LEN_LD2410C];  /* DMA 循环接收缓冲区      */
	bool     rx_ok;                           /* 帧接收完成标志          */
	uint16_t rx_len;                          /* 本次接收长度            */
} _RX_Data_LD2410C;

/* ---- 帧处理缓冲（从 rx_buff 拷贝完整帧） ---- */
typedef struct
{
	uint8_t  RECEIVE_BUF[RECEIVE_MAX_LEN_LD2410C];
	uint16_t Receive_len;
} _Receive_Data_LD2410C;

/* ---- 解析结果（检测目标数据） ---- */
typedef struct
{
	uint8_t  STATE_target;                    /* 目标状态                  */
	uint8_t  MOTION_target_distance;          /* 运动目标距离 (cm)         */
	uint8_t  MOTION_target_energy;            /* 运动目标能量值            */
	uint8_t  STATIC_target_distance;          /* 静止目标距离 (cm)         */
	uint8_t  STATIC_target_energy;            /* 静止目标能量值            */
	uint8_t  Detection_target_distance;       /* 探测目标距离 (cm)         */
	bool     rx_ok;                           /* 接收完成标志              */
	uint16_t len;                             /* 帧长度                    */
} _Detection_Target_LD2410C;

/* ---- 全局实例 ---- */
extern _RX_Data_LD2410C           RX_Data_LD2410C;
extern _Receive_Data_LD2410C      Receive_Data_LD2410C;
extern _Detection_Target_LD2410C  Detection_Target_LD2410C;

/* ---- 接口函数 ---- */

void Radar_Init(void);                   /* 启动 UART DMA 接收           */
void DataGet_LD2410C(void);              /* 帧解析, 提取目标数据         */

#endif /* __RADAR_H */
