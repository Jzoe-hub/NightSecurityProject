/**********************************************************************
 * 文件名称： rgb.c
 * 功能描述： RGB LED 驱动（共阳极, 低电平亮, 非 PWM 仅开关控制）
 * 说    明： 1. GPIO 初始化 → CubeMX 已配置推挽输出
 *           2. 共阳极: 写 0 亮, 写 1 灭
 ***********************************************************************/
#include "rgb.h"

void RGB_Set(uint8_t r, uint8_t g, uint8_t b)
{
	HAL_GPIO_WritePin(RGB_R_PORT, RGB_R_PIN, r ? GPIO_PIN_RESET : GPIO_PIN_SET);
	HAL_GPIO_WritePin(RGB_G_PORT, RGB_G_PIN, g ? GPIO_PIN_RESET : GPIO_PIN_SET);
	HAL_GPIO_WritePin(RGB_B_PORT, RGB_B_PIN, b ? GPIO_PIN_RESET : GPIO_PIN_SET);
}

void RGB_Off(void)
{
	HAL_GPIO_WritePin(RGB_R_PORT, RGB_R_PIN, GPIO_PIN_SET);
	HAL_GPIO_WritePin(RGB_G_PORT, RGB_G_PIN, GPIO_PIN_SET);
	HAL_GPIO_WritePin(RGB_B_PORT, RGB_B_PIN, GPIO_PIN_SET);
}
