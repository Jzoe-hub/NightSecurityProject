/**********************************************************************
 * 文件名称： esp8266.c
 * 功能描述： ESP8266 帧协议驱动 — CRC16 + DMA 接收 + 帧解析 + 环形缓冲
 * 硬件连接： USART2: PA2(TX)→ESP_RX, PA3(RX)→ESP_TX, 115200bps 8N1
 *           DMA1_Channel6 循环接收, USART2 IDLE 中断检测帧尾
 * 说    明： 1. USART2/DMA 初始化 → CubeMX MX_USART2_UART_Init() 已做
 *           2. HAL_UARTEx_RxEventCallback 在 radar.c 中统一分发,
 *              检测到 USART2 时调用本文件的 Esp8266_ProcessRxData()
 *           3. 本驱动不依赖 FreeRTOS, 只提供纯收发接口
 *           4. 接收帧存入环形缓冲区, Esp8266_GetFrame() 非阻塞取帧
 ***********************************************************************/
#include "esp8266.h"
#include "usart.h"             /* CubeMX 生成, 声明 huart2 */
#include <string.h>

/* ==================== 内部缓冲区 ==================== */

/* DMA 循环接收缓冲区 */
static uint8_t  rx_dma_buf[ESP_RX_BUF_SIZE];

/* 帧解析中间状态 */
static uint8_t  rx_parse_buf[ESP_FRAME_BUF_SIZE];
static uint16_t rx_parse_len  = 0;		//接收的帧长度

/* 接收帧环形缓冲区 */
static EspFrame rx_frame_ring[ESP_FRAME_QUEUE_LEN];
static uint8_t  rx_frame_head  = 0;   /* 写入位置 */
static uint8_t  rx_frame_tail  = 0;   /* 读取位置 */
static uint8_t  rx_frame_count = 0;   /* 当前帧数 */

/* ==================== CRC16-MODBUS ==================== */

/**********************************************************************
 * 函数名称： crc16_modbus
 * 功能描述： 计算 MODBUS CRC-16 校验值（与 ESP8266 端一致）
 * 输入参数： data — 数据指针
 *            len  — 数据长度
 * 返 回 值： 16 位 CRC 值
 ***********************************************************************/
static uint16_t crc16_modbus(uint8_t *data, uint16_t len)
{
	uint16_t crc = 0xFFFF;              // 1. 从一个固定初值开始
	for (uint16_t i = 0; i < len; i++) {
	    crc ^= data[i];                 // 2. 把当前字节"揉进"crc（异或）
	    for (uint16_t j = 0; j < 8; j++) {       // 3. 揉完再搅 8 次
	        if (crc & 0x0001)           //    最低位是 1 就...
	            crc = (crc >> 1) ^ 0xA001;  // 右移再异或一个固定值
	        else
	            crc >>= 1;              //    否则只右移
	    }
	}
	return crc;                         // 4. 得到 2 字节"指纹"
}

/* ==================== 帧解析 ==================== */

/**********************************************************************
 * 函数名称： esp8266_parse_frame
 * 功能描述： 在 DMA 缓冲区中搜索帧头 0xAA, 找到后验证帧结构,
 *           校验 CRC, 提取 type+payload 存入环形缓冲区。
 *           解析完一帧后继续搜索后续帧（支持粘包）。
 * 输入参数： data — DMA 收到的数据
 *            size — 数据长度
 ***********************************************************************/
static void esp8266_parse_frame(uint8_t *data, uint16_t size)
{
    for (uint16_t i = 0; i < size; i++)
    {
        uint8_t ch = data[i];

        /* 搜索帧头 0xAA */
        if (rx_parse_len == 0) {
            if (ch != ESP_FRAME_STX) continue;
        }

        /* 存入解析缓冲区 */
        if (rx_parse_len < ESP_FRAME_BUF_SIZE) {
            rx_parse_buf[rx_parse_len++] = ch;
        }

        /* 帧太短, 继续收 */
        if (rx_parse_len < 7) continue;   /* STX+TYPE+LEN(2)+PAYLOAD(0)+CRC(2)+ETX */

        uint8_t  type = rx_parse_buf[1];
        uint16_t plen = (rx_parse_buf[2] << 8) | rx_parse_buf[3];

        if (plen > ESP_PAYLOAD_MAX) {
            rx_parse_len = 0;              /* 长度非法, 丢弃 */
            continue;
        }

        uint16_t total = 1 + 1 + 2 + plen + 2 + 1;  /* STX+TYPE+LEN+PAYLOAD+CRC+ETX */
        if (total > ESP_FRAME_BUF_SIZE) {
            rx_parse_len = 0;
            continue;
        }

        /* 还没收完一帧 */
        if (rx_parse_len < total) continue;

        /* 收到完整帧: 校验帧尾 */
        if (rx_parse_buf[total - 1] != ESP_FRAME_ETX) {
            rx_parse_len = 0;              /* 帧尾不匹配, 丢弃整帧 */
            continue;
        }

        /* 校验 CRC: 范围 TYPE+LEN+PAYLOAD */
        uint16_t calc_crc = crc16_modbus(&rx_parse_buf[1], 1 + 2 + plen);
        uint16_t recv_crc = (rx_parse_buf[1 + 1 + 2 + plen] << 8)
                          |  rx_parse_buf[1 + 1 + 2 + plen + 1];

        if (calc_crc == recv_crc && rx_frame_count < ESP_FRAME_QUEUE_LEN)
        {
            /* 帧有效: 存入环形缓冲区 */
            EspFrame *frame = &rx_frame_ring[rx_frame_head];
            frame->type  = type;
            frame->len   = plen;
            frame->valid = true;
            if (plen > 0) {
                memcpy(frame->payload,
                       &rx_parse_buf[1 + 1 + 2], plen);
            }
            rx_frame_head = (rx_frame_head + 1) % ESP_FRAME_QUEUE_LEN;
            rx_frame_count++;
        }
        /* CRC 错误 → 静默丢弃 */

        rx_parse_len = 0;                  /* 准备解析下一帧 */
    }
}

/* ==================== DMA 接收回调 (由 radar.c 的 HAL_UARTEx_RxEventCallback 调用) ==================== */

/**********************************************************************
 * 函数名称： Esp8266_ProcessRxData
 * 功能描述： 被 HAL_UARTEx_RxEventCallback (in radar.c) 调用,
 *           处理 USART2 DMA+IDLE 接收到的一帧数据
 * 输入参数： size — 本次接收到的字节数
 ***********************************************************************/
void Esp8266_ProcessRxData(uint16_t size)
{
    if (size > 0 && size <= ESP_RX_BUF_SIZE) {
        esp8266_parse_frame(rx_dma_buf, size);
    }
    HAL_UARTEx_ReceiveToIdle_DMA(&huart2, rx_dma_buf, ESP_RX_BUF_SIZE);  /* ★ 重启接收，让 IDLE 中断重新上岗 */
}

/* ==================== 公开接口 ==================== */

/**********************************************************************
 * 函数名称： Esp8266_Init
 * 功能描述： 启动 USART2 DMA+IDLE 循环接收
 *           调用前需确保 MX_USART2_UART_Init() 已完成
 ***********************************************************************/
void Esp8266_Init(void)
{
    memset(rx_dma_buf, 0, sizeof(rx_dma_buf));
    rx_parse_len    = 0;
    rx_frame_head   = 0;
    rx_frame_tail   = 0;
    rx_frame_count  = 0;
    HAL_UARTEx_ReceiveToIdle_DMA(&huart2, rx_dma_buf, ESP_RX_BUF_SIZE);
}

/**********************************************************************
 * 函数名称： Esp8266_SendFrame
 * 功能描述： 构建帧 (STX+TYPE+LEN+PAYLOAD+CRC16+ETX),
 *           通过 USART2 阻塞发送给 ESP8266
 * 输入参数： type    — 帧类型 (ESP_TYPE_STATE 等)
 *            payload — 负载数据指针
 *            len     — 负载长度 (0~255)
 ***********************************************************************/
void Esp8266_SendFrame(uint8_t type, uint8_t *payload, uint16_t len)
{
    uint8_t frame[ESP_FRAME_BUF_SIZE];
    uint16_t pos = 0;

    if (len > ESP_PAYLOAD_MAX) return;

    frame[pos++] = ESP_FRAME_STX;                     /* 帧头 0xAA          */
    frame[pos++] = type;                              /* TYPE               */
    frame[pos++] = (len >> 8) & 0xFF;                 /* LEN 高字节         */
    frame[pos++] =  len       & 0xFF;                 /* LEN 低字节         */

    if (len > 0 && payload != NULL) {
        memcpy(&frame[pos], payload, len);
    }
    pos += len;

    /* CRC16 计算范围: TYPE + LEN(2B) + PAYLOAD */
    uint16_t crc = crc16_modbus(&frame[1], 1 + 2 + len);
    frame[pos++] = (crc >> 8) & 0xFF;
    frame[pos++] =  crc       & 0xFF;

    frame[pos++] = ESP_FRAME_ETX;                     /* 帧尾 0x55          */

    HAL_UART_Transmit(&huart2, frame, pos, 100);      /* 阻塞发送, 超时100ms */
}

/**********************************************************************
 * 函数名称： Esp8266_GetFrame
 * 功能描述： 从环形缓冲区取出一帧, 非阻塞
 * 输入参数： frame — 输出, 存放取出的帧
 * 返 回 值： true=取到有效帧, false=缓冲区空
 ***********************************************************************/
bool Esp8266_GetFrame(EspFrame *frame)
{
    if (rx_frame_count == 0) return false;

    /* 禁用中断防止 DMA 回调同时写入 */
    __disable_irq();
    *frame = rx_frame_ring[rx_frame_tail];
    rx_frame_ring[rx_frame_tail].valid = false;
    rx_frame_tail = (rx_frame_tail + 1) % ESP_FRAME_QUEUE_LEN;
    rx_frame_count--;
    __enable_irq();

    return frame->valid;
}
