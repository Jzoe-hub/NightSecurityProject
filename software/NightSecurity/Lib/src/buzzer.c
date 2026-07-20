/**********************************************************************
 * 文件名称： buzzer.c
 * 功能描述： 蜂鸣器驱动（有源蜂鸣器, PA5 推挽输出）
 * 说    明： GPIO 初始化 → CubeMX 已配置 PA5 为推挽输出
 ***********************************************************************/
#include "buzzer.h"

void Buzzer_On(void)
{
	HAL_GPIO_WritePin(BUZZER_PORT, BUZZER_PIN, GPIO_PIN_SET);
}

void Buzzer_Off(void)
{
	HAL_GPIO_WritePin(BUZZER_PORT, BUZZER_PIN, GPIO_PIN_RESET);
}

void Buzzer_Beep(uint16_t ms)
{
	Buzzer_On();
	HAL_Delay(ms);
	Buzzer_Off();
}
