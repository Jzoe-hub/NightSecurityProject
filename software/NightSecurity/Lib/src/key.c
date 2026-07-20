/**********************************************************************
 * 文件名称： key.c
 * 功能描述： 按键驱动（PB13/14/15, 上拉输入, 按下=低电平）
 * 说    明： 1. GPIO 初始化 → CubeMX 已配置上拉输入
 *           2. 去抖由 task_ui.c 按 200ms 周期自然处理, 驱动层只读电平
 ***********************************************************************/
#include "key.h"

uint8_t Key1_IsPressed(void)
{
	return (HAL_GPIO_ReadPin(KEY1_PORT, KEY1_PIN) == GPIO_PIN_RESET) ? 1 : 0;
}

uint8_t Key2_IsPressed(void)
{
	return (HAL_GPIO_ReadPin(KEY2_PORT, KEY2_PIN) == GPIO_PIN_RESET) ? 1 : 0;
}

uint8_t Key3_IsPressed(void)
{
	return (HAL_GPIO_ReadPin(KEY3_PORT, KEY3_PIN) == GPIO_PIN_RESET) ? 1 : 0;
}
