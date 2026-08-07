/**********************************************************************
 * 文件名称： radar.c
 * 功能描述： HLK-LD2410C 毫米波雷达传感器驱动
 * 硬件连接： USART1 (PA9/TX, PA10/RX, 25600bps 8N1, DMA1_CH5 循环接收)
 * 来    源： 基于 GitHub: CLi321/sensor-driver-code 移植
 *           UART 层: SPL(UART4+PC10/11+RXNE中断) → HAL(USART1+DMA+IDLE)
 *           帧解析: 保留原作者 DataGet_LD2410C() 逻辑不变
 * 说    明： 1. USART1/DMA 初始化 → CubeMX MX_USART1_UART_Init() 已做
 *           2. Radar_Init() 启动 DMA 循环接收, 随后每收到一帧自动解析
 *           3. 解析结果存入全局 Detection_Target_LD2410C
 ***********************************************************************/
#include "radar.h"
#include "usart.h"         /* CubeMX 生成, 声明 huart1 */
#include "esp8266.h"       /* Esp8266_ProcessRxData()   */
#include <string.h>

/* ---- 全局实例定义 ---- */
_RX_Data_LD2410C           RX_Data_LD2410C;
_Receive_Data_LD2410C      Receive_Data_LD2410C;
_Detection_Target_LD2410C  Detection_Target_LD2410C;

/* ---- 内部标志 ---- */
static bool flag_start_LD2410C = false;

/* ==================== 帧解析（原版逻辑保留） ==================== */

/**********************************************************************
 * 函数名称： DataGet_LD2410C
 * 功能描述： 从 Receive_Data_LD2410C.RECEIVE_BUF 中解析雷达目标数据
 *           帧头 F4 F3 F2 F1, 校验 AA/55, 提取运动/静止目标信息
 *           解析失败则将所有目标数据清零
 * 来    源： CLi321/sensor-driver-code (原版保留)
 ***********************************************************************/
void DataGet_LD2410C(void)
{
	if (Receive_Data_LD2410C.RECEIVE_BUF[0] == 0xf4
			&& Receive_Data_LD2410C.RECEIVE_BUF[1] == 0xf3
			&& Receive_Data_LD2410C.RECEIVE_BUF[2] == 0xf2
			&& Receive_Data_LD2410C.RECEIVE_BUF[3] == 0xf1)
	{
		if (Receive_Data_LD2410C.RECEIVE_BUF[7]  == 0xaa
				&& Receive_Data_LD2410C.RECEIVE_BUF[17] == 0x55)
		{
			Detection_Target_LD2410C.STATE_target
					= Receive_Data_LD2410C.RECEIVE_BUF[8];
			Detection_Target_LD2410C.MOTION_target_distance
					= (Receive_Data_LD2410C.RECEIVE_BUF[10] << 8)
					+ Receive_Data_LD2410C.RECEIVE_BUF[9];
			Detection_Target_LD2410C.MOTION_target_energy
					= Receive_Data_LD2410C.RECEIVE_BUF[11];
			Detection_Target_LD2410C.STATIC_target_distance
					= (Receive_Data_LD2410C.RECEIVE_BUF[13] << 8)
					+ Receive_Data_LD2410C.RECEIVE_BUF[12];
			Detection_Target_LD2410C.STATIC_target_energy
					= Receive_Data_LD2410C.RECEIVE_BUF[14];
			Detection_Target_LD2410C.Detection_target_distance
					= (Receive_Data_LD2410C.RECEIVE_BUF[16] << 8)
					+ Receive_Data_LD2410C.RECEIVE_BUF[15];
		}
		else
		{
			Detection_Target_LD2410C.STATE_target            = 0;
			Detection_Target_LD2410C.MOTION_target_distance  = 0;
			Detection_Target_LD2410C.MOTION_target_energy    = 0;
			Detection_Target_LD2410C.STATIC_target_distance  = 0;
			Detection_Target_LD2410C.STATIC_target_energy    = 0;
			Detection_Target_LD2410C.Detection_target_distance = 0;
		}
	}
	else
	{
		Detection_Target_LD2410C.STATE_target            = 0;
		Detection_Target_LD2410C.MOTION_target_distance  = 0;
		Detection_Target_LD2410C.MOTION_target_energy    = 0;
		Detection_Target_LD2410C.STATIC_target_distance  = 0;
		Detection_Target_LD2410C.STATIC_target_energy    = 0;
		Detection_Target_LD2410C.Detection_target_distance = 0;
	}
}

/* ==================== DMA 接收管理 ==================== */

/**********************************************************************
 * 函数名称： Radar_Init
 * 功能描述： 启动 USART1 DMA 循环接收, 等待雷达数据帧
 * 说    明： 调用前需确保 MX_USART1_UART_Init() 已完成
 ***********************************************************************/
void Radar_Init(void)
{
	RX_Data_LD2410C.rx_ok  = false;
	RX_Data_LD2410C.rx_len = 0;
	flag_start_LD2410C = false;
	HAL_UARTEx_ReceiveToIdle_DMA(&huart1,
			RX_Data_LD2410C.rx_buff, BUFF_MAX_LEN_LD2410C);
}

/**********************************************************************
 * 函数名称： HAL_UARTEx_RxEventCallback
 * 功能描述： HAL 回调 — UART IDLE 或接收半满时自动调用
 *           每收到完整一帧（IDLE 检测到帧间隔），拷贝到处理缓冲区并解析
 * 说    明： 本函数覆盖 HAL 弱定义, 会被所有 UART 的 IDLE 事件触发,
 *           内部通过 huart->Instance 区分 USART1
 ***********************************************************************/
void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size)
{
	if (huart->Instance == USART2) {             /* ESP8266 的数据       */
		Esp8266_ProcessRxData(Size);
		return;
	}
	if (huart->Instance != USART1) return;       /* 只处理雷达的 UART    */

	/* 拷贝接收到的数据到处理缓冲区 */
	Receive_Data_LD2410C.Receive_len = Size;
	memset(Receive_Data_LD2410C.RECEIVE_BUF, 0, RECEIVE_MAX_LEN_LD2410C);
	memcpy(Receive_Data_LD2410C.RECEIVE_BUF,
			RX_Data_LD2410C.rx_buff, Size);

	RX_Data_LD2410C.rx_ok  = true;
	RX_Data_LD2410C.rx_len = 0;

	DataGet_LD2410C();                               /* 解析帧数据        */

	/* 重新启动 DMA 接收下一帧 */
	HAL_UARTEx_ReceiveToIdle_DMA(&huart1,
			RX_Data_LD2410C.rx_buff, BUFF_MAX_LEN_LD2410C);
}
