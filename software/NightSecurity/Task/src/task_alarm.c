/**********************************************************************
 * 文件名称： task_alarm.c
 * 功能描述： AlarmTask — 本地声光报警任务 (50ms / Priority 5 / Stack 256)
 *           接收 SecurityTask 的报警命令, 驱动蜂鸣器/语音/RGB 执行时序
 ***********************************************************************/
#include "task_config.h"
#include "buzzer.h"
#include "voice.h"
#include "rgb.h"

#define VOICE_INTERVAL_MS   3000    /* 报警语音播报间隔 (ms) */

/* ==================== 报警动作子函数 ==================== */

/* 时序 step 用文件作用域 static, 撤防时可从 AlarmTask 重置 */
static uint8_t fire_step = 0;       /* 火灾报警步进      */
static uint8_t intr_step = 0;       /* 入侵报警步进      */

/**********************************************************************
 * 函数名称： alarm_fire
 * 功能描述： 火灾报警时序——蜂鸣器急促短鸣 + RGB 红色闪烁。
 *           (语音由 AlarmTask 主循环独立触发)
 *           step 0~3:   蜂鸣器响, RGB 红亮 (200ms)
 *           step 4~5:   蜂鸣器停, RGB 灭   (100ms 间歇)
 * 输入参数： 无
 * 返 回 值： 无
 ***********************************************************************/
static void alarm_fire(void)
{
	fire_step++;
	if (fire_step <= 3)
	{
		Buzzer_On();
		RGB_Set(1, 0, 0);               /* 红 */
	}
	else if (fire_step <= 5)
	{
		Buzzer_Off();
		RGB_Off();
	}
	else
	{
		fire_step = 0;                  /* 复位, 循环 */
	}
}

/**********************************************************************
 * 函数名称： alarm_intrusion
 * 功能描述： 入侵报警时序——蜂鸣器长鸣 + RGB 蓝色闪烁。
 *           (语音由 AlarmTask 主循环独立触发)
 *           step 0~9:   蜂鸣器响, RGB 蓝亮 (500ms)
 *           step 10~11: 蜂鸣器停, RGB 灭   (100ms 间歇)
 * 输入参数： 无
 * 返 回 值： 无
 ***********************************************************************/
static void alarm_intrusion(void)
{
	intr_step++;
	if (intr_step <= 9)
	{
		Buzzer_On();
		RGB_Set(0, 0, 1);               /* 蓝 */
	}
	else if (intr_step <= 11)
	{
		Buzzer_Off();
		RGB_Off();
	}
	else
	{
		intr_step = 0;                  /* 复位, 循环 */
	}
}

/**********************************************************************
 * 函数名称： alarm_idle
 * 功能描述： 无报警时的安全状态——蜂鸣器停、RGB 全灭、不触发语音。
 * 输入参数： 无
 * 返 回 值： 无
 ***********************************************************************/
static void alarm_idle(void)
{
	Buzzer_Off();
	RGB_Off();
}

/* ==================== 任务主函数 ==================== */

/**********************************************************************
 * 函数名称： AlarmTask
 * 功能描述： 每 50ms 轮询报警队列, 切换报警模式。
 *           语音独立触发: 布防后延迟 3 秒(等布防成功语音播完),
 *           之后每 3 秒播报一次, 持续循环直到撤防或报警解除。
 *           撤防时: 强制静默 + 清空队列 + 重置时序, 立即停止播报。
 * 输入参数： pvParameters — 未使用
 * 返 回 值： 无
 ***********************************************************************/
void AlarmTask(void *pvParameters)
{
	(void)pvParameters;
	static uint8_t    mode = 0;          /* 当前报警模式: 0=无 1=火 2=入侵 */
	static uint8_t    prev_armed = 0;    /* 上一轮布防状态, 检测上升沿 */
	static TickType_t last_voice_time = 0; /* 上次语音触发时刻 */

	for (;;)
	{
		g_heartbeat_alarm++;

		/* 0. 布防上升沿: 记录时刻, 让报警语音延迟 3 秒 */
		if (g_sw_armed && !prev_armed)
		{
			last_voice_time = xTaskGetTickCount();
		}
		prev_armed = g_sw_armed;

		/* 1. 撤防: 强制静默 + 清空队列 + 重置时序 */
		if (!g_sw_armed)
		{
			mode = 0;
			fire_step = 0;
			intr_step = 0;
			while (xQueueReceive(g_securityQueue, &Security_Data, 0) == pdPASS)
				;   /* 循环丢弃所有旧命令 */
		}
		/* 2. 布防: 非阻塞收队列, 更新模式 */
		else if (xQueueReceive(g_securityQueue, &Security_Data, 0) == pdPASS)
		{
			mode = Security_Data.type;   /* 1=火灾 2=入侵 */
		}

		/* 3. 报警语音周期播报 (布防后延迟 3 秒, 之后每 3 秒一次) */
		if ((mode == 1 || mode == 2) &&
			(xTaskGetTickCount() - last_voice_time) >= pdMS_TO_TICKS(VOICE_INTERVAL_MS))
		{
			if (mode == 1)
				Voice_IO1_Trigger();     /* 火灾报警语音 */
			else
				Voice_IO2_Trigger();     /* 入侵报警语音 */
			last_voice_time = xTaskGetTickCount();
		}

		/* 4. 根据当前模式执行声光时序 */
		if (mode == 1)
			alarm_fire();
		else if (mode == 2)
			alarm_intrusion();
		else
			alarm_idle();

		vTaskDelay(pdMS_TO_TICKS(50));
	}
}
