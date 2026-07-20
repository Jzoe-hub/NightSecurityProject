/**********************************************************************
 * 文件名称： finger.c
 * 功能描述： ZW101 半导体指纹模块驱动（UART 指令-应答协议）
 * 硬件连接： USART3 (PB10/TX, PB11/RX, 57600bps 8N1)
 *           TOUCH → PA6 (手指检测, 1=有手指)
 * 来    源： 基于 ZW101 Arduino 示例 (ZW101.ino) 协议逆向, HAL 重写
 * 说    明： 1. USART3 初始化 → CubeMX MX_USART3_UART_Init() 已做
 *           2. 指令-应答模式, 每次发指令后等待应答, 超时 1s
 *           3. 注册流程：获取图像×5→合并特征→存储模板
 ***********************************************************************/
#include "finger.h"
#include "usart.h"             /* CubeMX 生成, 声明 huart3 */
#include <string.h>

/* ---- 内部常量 ---- */
#define FP_RESPONSE_TIMEOUT     1000   /* ms, 应答超时  */
#define FP_RESP_BUF_SIZE        50     /* 应答缓冲区    */

/* ---- 内部函数声明 ---- */
static void  fp_send_cmd(uint8_t cmd);
static void  fp_send_cmd2(uint8_t cmd, uint8_t param1);
static void  fp_send_cmd3(uint8_t cmd, uint8_t param1,
                          uint16_t param2, uint16_t param3);
static bool  fp_recv_response(void);

static uint8_t g_fp_response[FP_RESP_BUF_SIZE];
static uint8_t g_fp_resp_len;

/* ==================== TOUCH 引脚 ==================== */

/**********************************************************************
 * 函数名称： Finger_IsTouched
 * 功能描述： 读取 TOUCH 引脚电平
 * 返 回 值： 1=手指按下, 0=无手指
 ***********************************************************************/
uint8_t Finger_IsTouched(void)
{
	return (HAL_GPIO_ReadPin(FP_TOUCH_PORT, FP_TOUCH_PIN) == GPIO_PIN_SET) ? 1 : 0;
}

/* ==================== 协议层：发送 ==================== */

/**********************************************************************
 * 函数名称： fp_send_packet
 * 功能描述： 构建并发送 ZW101 指令包
 * 输入参数： cmd  — 指令码
 *            params — 参数字节数组
 *            param_len — 参数字节数
 ***********************************************************************/
static void fp_send_packet(uint8_t cmd, uint8_t *params, uint8_t param_len)
{
	uint8_t  packet[32];
	uint16_t length  = 1 + param_len + 2;   /* cmd+params+校验和(2) */
	uint16_t checksum = FP_PACKET_ID_CMD + (length >> 8) + (length & 0xFF);

	packet[0] = FP_HEADER_HIGH;             /* 帧头 EF          */
	packet[1] = FP_HEADER_LOW;              /* 帧头 01          */
	packet[2] = (FP_ADDR_DEFAULT >> 24) & 0xFF;   /* 地址 4Byte  */
	packet[3] = (FP_ADDR_DEFAULT >> 16) & 0xFF;
	packet[4] = (FP_ADDR_DEFAULT >> 8)  & 0xFF;
	packet[5] =  FP_ADDR_DEFAULT        & 0xFF;
	packet[6] = FP_PACKET_ID_CMD;            /* 包标识           */
	packet[7] = (length >> 8) & 0xFF;       /* 长度高字节       */
	packet[8] =  length       & 0xFF;       /* 长度低字节       */
	packet[9] = cmd;                         /* 指令码           */
	checksum += cmd;

	for (uint8_t i = 0; i < param_len; i++)
	{
		packet[10 + i] = params[i];
		checksum += params[i];
	}

	packet[10 + param_len]     = (checksum >> 8) & 0xFF;  /* 校验和高字节 */
	packet[11 + param_len]     =  checksum       & 0xFF;  /* 校验和低字节 */

	uint8_t total = 10 + param_len + 2;         /* 帧头2+地址4+标识1+长度2+数据N+校验2 */
	HAL_UART_Transmit(&huart3, packet, total, FP_RESPONSE_TIMEOUT);
}

/* ---- 便捷发送函数 ---- */

static void fp_send_cmd(uint8_t cmd)
{
	fp_send_packet(cmd, NULL, 0);
}

static void fp_send_cmd2(uint8_t cmd, uint8_t param1)
{
	fp_send_packet(cmd, &param1, 1);
}

static void fp_send_cmd3(uint8_t cmd, uint8_t param1,
		uint16_t param2, uint16_t param3)
{
	uint8_t params[5];
	params[0] = param1;
	params[1] = (param2 >> 8) & 0xFF;
	params[2] =  param2       & 0xFF;
	params[3] = (param3 >> 8) & 0xFF;
	params[4] =  param3       & 0xFF;
	fp_send_packet(cmd, params, 5);
}

/* ==================== 协议层：接收 ==================== */

/**********************************************************************
 * 函数名称： fp_recv_response
 * 功能描述： 接收应答包, 存入全局 g_fp_response, 检查确认码
 * 返 回 值： true=成功(确认码 0x00), false=超时/失败
 * 说    明： 轮询接收直到数据到达, 超时 1s
 ***********************************************************************/
static bool fp_recv_response(void)
{
	memset(g_fp_response, 0, FP_RESP_BUF_SIZE);
	g_fp_resp_len = 0;

	/* 轮询接收: 等待至少 12 字节到达 (最小应答包)               */
	uint32_t start = HAL_GetTick();
	while (HAL_GetTick() - start < FP_RESPONSE_TIMEOUT)
	{
		uint8_t ch;
		if (HAL_UART_Receive(&huart3, &ch, 1, 10) == HAL_OK)
		{
			if (g_fp_resp_len < FP_RESP_BUF_SIZE)
				g_fp_response[g_fp_resp_len++] = ch;

			/* 收到校验和两个字节后, 帧完整 */
			if (g_fp_resp_len >= 12 && g_fp_resp_len >= 12 +
					(g_fp_response[7] << 8) + g_fp_response[8])
				break;
		}
	}

	if (g_fp_resp_len >= 12 && g_fp_response[9] == 0x00)
		return true;
	return false;
}

/* ==================== 公开接口 ==================== */

/**********************************************************************
 * 函数名称： Finger_ReadSysPara
 * 功能描述： 读取模组基本参数（指纹库容量、模板大小、设备地址等）
 * 返 回 值： true=成功
 ***********************************************************************/
bool Finger_ReadSysPara(void)
{
	fp_send_cmd(FP_CMD_READ_SYSPARA);
	return fp_recv_response();
}

/**********************************************************************
 * 函数名称： Finger_Enroll
 * 功能描述： 注册一枚新指纹。流程：
 *           获取图像→生成特征 (重复 5 次, 对应 BufferID 1~5)
 *           →合并特征→存储模板(默认位置 1)
 * 返 回 值： true=注册成功
 ***********************************************************************/
bool Finger_Enroll(void)
{
	/* 步骤 1+2：采集 5 次图像, 每次生成特征存入不同 Buffer  */
	for (uint8_t id = 1; id <= 5; id++)
	{
		fp_send_cmd(FP_CMD_GET_IMAGE);
		if (!fp_recv_response()) { HAL_Delay(500); continue; }

		fp_send_cmd2(FP_CMD_GEN_CHAR, id);
		if (fp_recv_response()) continue;   /* 本次成功, 下一个 */
		/* 失败重试: id 不自增, 但循环内 id++ 已执行所以补一次 id-- */
		id--;
		HAL_Delay(500);
		if (id > 0) continue;
	}

	/* 步骤 3：合并 Buffer 1+2 的特征  */
	fp_send_cmd(FP_CMD_REG_MODEL);
	if (!fp_recv_response()) return false;

	/* 步骤 4：存储到模板位置 1 (BufferID=1, TemplateID=1)  */
	{
		uint8_t p[] = { 1, 0, 1 };             /* BufferID + TemplateID(2B) */
		fp_send_packet(FP_CMD_STORE_CHAR, p, 3);
	}
	return fp_recv_response();
}

/**********************************************************************
 * 函数名称： Finger_Search
 * 功能描述： 搜索指纹库中是否存在当前手指的模板
 *           流程：获取图像→生成特征(Buffer 1)→搜索(起 1, 止 100)
 * 返 回 值： true=匹配成功
 ***********************************************************************/
bool Finger_Search(void)
{
	/* 步骤 1：获取图像                  */
	fp_send_cmd(FP_CMD_GET_IMAGE);
	if (!fp_recv_response()) return false;

	/* 步骤 2：生成特征到 Buffer 1      */
	fp_send_cmd2(FP_CMD_GEN_CHAR, 1);
	if (!fp_recv_response()) return false;

	/* 步骤 3：搜索指纹库 (ID 1~100)    */
	fp_send_cmd3(FP_CMD_SEARCH, 1, 1, 100);
	return fp_recv_response();
}

/**********************************************************************
 * 函数名称： Finger_ClearAll
 * 功能描述： 清空指纹库所有模板
 * 返 回 值： true=成功
 ***********************************************************************/
bool Finger_ClearAll(void)
{
	fp_send_cmd(FP_CMD_CLEAR_LIB);
	return fp_recv_response();
}
