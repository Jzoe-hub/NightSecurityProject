/**********************************************************************
 * 文件名称： esp8266.h
 * 功能描述： ESP8266 帧协议驱动头文件（STM32 ↔ ESP8266 USART2 通信）
 * 硬件连接： USART2: PA2(TX)→ESP_RX, PA3(RX)→ESP_TX, 115200bps 8N1
 *            DMA1_Channel6 循环接收, USART2 IDLE 中断检测帧尾
 * 说    明： 1. CubeMX 已配置 USART2 + DMA1_CH6, 本驱动只做帧协议
 *           2. 本驱动不依赖 FreeRTOS, 只提供纯收发接口
 *           3. HAL_UARTEx_RxEventCallback 在 radar.c 已有实现,
 *              本驱动扩展它处理 USART2 的数据
 ***********************************************************************/
#ifndef __ESP8266_H
#define __ESP8266_H

#include "stm32f1xx_hal.h"
#include <stdbool.h>
#include <stdint.h>

/* ==================== 帧协议常量 ==================== */
#define ESP_FRAME_STX       0xAA   /* 帧头                   */
#define ESP_FRAME_ETX       0x55   /* 帧尾                   */
#define ESP_RX_BUF_SIZE     128    /* DMA 循环接收缓冲区（极限省RAM） */
#define ESP_FRAME_BUF_SIZE  256    /* 帧解析缓冲区                    */
#define ESP_PAYLOAD_MAX     128    /* 单帧最大负载（命令JSON最大80字节） */
#define ESP_FRAME_QUEUE_LEN 2      /* 接收帧环形缓冲区（省RAM）       */

/* ==================== TYPE 码 ==================== */
#define ESP_TYPE_STATE      0x01   /* 上行: 传感器状态上报     */
#define ESP_TYPE_HEARTBEAT  0x02   /* 上行: 心跳               */
#define ESP_TYPE_CMD        0x03   /* 下行: 控制命令           */
#define ESP_TYPE_CONFIG     0x04   /* 下行: 阈值配置           */

/* ==================== 接收帧结构体 ==================== */
typedef struct {
    uint8_t  type;                     /* 帧类型                 */
    uint8_t  payload[ESP_PAYLOAD_MAX]; /* 负载数据               */
    uint16_t len;                      /* 负载实际长度           */
    bool     valid;                    /* true=有效帧             */
} EspFrame;

/* ==================== 公开接口 ==================== */

void     Esp8266_Init(void);
void     Esp8266_SendFrame(uint8_t type, uint8_t *payload, uint16_t len);
bool     Esp8266_GetFrame(EspFrame *frame);
void     Esp8266_ProcessRxData(uint16_t size);  /* 由 radar.c 回调调用 */

#endif /* __ESP8266_H */
