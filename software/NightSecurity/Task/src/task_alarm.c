/**********************************************************************
 * 文件名称： task_alarm.c
 * 功能描述： AlarmTask — 本地声光报警任务 (50ms / Priority 5 / Stack 256)
 *           接收 SecurityTask 的报警命令, 驱动蜂鸣器/语音/RGB 执行时序
 ***********************************************************************/
#include "task_config.h"
#include "buzzer.h"
#include "voice.h"
#include "rgb.h"

/* ==================== 报警动作子函数 ==================== */

/**********************************************************************
 * 函数名称： alarm_fire
 * 功能描述： 火灾报警时序——蜂鸣器急促短鸣 + 语音播报 + RGB 红色闪烁。
 *           由于 50ms 周期太快, 用 static step 计数器控制节奏：
 *           step 0~3:   蜂鸣器响, RGB 红亮
 *           step 4~5:   蜂鸣器停, RGB 灭
 *           step 6:     语音触发一次 (Voice_IO1_Trigger)
 *           step ≥6:    复位 step=0, 循环
 *           每 50ms step 加 1, 一个完整周期 = 7×50ms = 350ms
 * 输入参数： 无
 * 返 回 值： 无
 ***********************************************************************/
static void alarm_fire(void)
{
	static uint8_t step = 0;               /* 50ms 步进, 7 步一个周期 (350ms) */
	step++;
	/* 1. step 0~3: 蜂鸣器响 + RGB 红亮 (200ms) */
	if (step <= 3)
	{
		Buzzer_On();
		RGB_Set(1, 0, 0);               /* 红 */
	}
	/* 2. step 4~5: 蜂鸣器停 + RGB 灭 (100ms 间歇) */
	else if (step <= 5)
	{
		Buzzer_Off();
		RGB_Off();
	}
	/* 3. step 6: 触发一次语音 (仅一次, 不下次不重复) */
	else if (step == 6)
	{
		Voice_IO1_Trigger();            /* 曲目 1: 火灾报警 */
	}
	/* 4. 复位, 循环 */
	else
	{
		step = 0;
	}
}

/**********************************************************************
 * 函数名称： alarm_intrusion
 * 功能描述： 入侵报警时序——蜂鸣器长鸣 + 语音播报 + RGB 蓝色闪烁。
 *           时序设计：
 *           step 0~9:   蜂鸣器响, RGB 蓝亮（持续 500ms）
 *           step 10~11: 蜂鸣器停, RGB 灭（间歇 100ms）
 *           step 12:    语音触发一次 (Voice_IO2_Trigger)
 *           step ≥12:   复位 step=0
 * 输入参数： 无
 * 返 回 值： 无
 ***********************************************************************/
static void alarm_intrusion(void)
{
	static uint8_t step = 0;               /* 50ms 步进, 13 步一个周期 (650ms) */
	step++;
	/* 1. step 0~9: 蜂鸣器长鸣 + RGB 蓝亮 (500ms) */
	if (step <= 9)
	{
		Buzzer_On();
		RGB_Set(0, 0, 1);               /* 蓝 */
	}
	/* 2. step 10~11: 蜂鸣器停 + RGB 灭 (100ms 间歇) */
	else if (step <= 11)
	{
		Buzzer_Off();
		RGB_Off();
	}
	/* 3. step 12: 触发一次语音 */
	else if (step == 12)
	{
		Voice_IO2_Trigger();            /* 曲目 2: 入侵报警 */
	}
	/* 4. 复位, 循环 */
	else
	{
		step = 0;
	}
}

/**********************************************************************
 * 函数名称： alarm_idle
 * 功能描述： 无报警时的安全状态——蜂鸣器停、RGB 全灭、
 *           不触发语音。此函数无状态, 每次调用直接写 GPIO。
 *           执行流程：
 *           1. Buzzer_Off()
 *           2. RGB_Off()
 *           (语音不触发, BUSY 不关心)
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
 * 功能描述： 每 50ms 轮询报警队列, 根据收到的命令切换报警模式。
 *           执行流程：
 *           1. xQueueReceive(g_alarmQueue, &Security_Data, 0) — 非阻塞读取
 *           2. 若收到新命令 → 更新当前报警模式 + 复位时序计数器
 *           3. 根据当前模式调用 alarm_fire / alarm_intrusion / alarm_idle
 * 输入参数： pvParameters — 未使用
 * 返 回 值： 无
 ***********************************************************************/
void AlarmTask(void *pvParameters)
{
	(void)pvParameters;
	static uint8_t mode = 0;
	for (;;)
	{
		/* 1. 非阻塞收队列, 有新命令则切换模式 */
		g_heartbeat_alarm++;
		if (xQueueReceive(g_securityQueue, &Security_Data, 0) == pdPASS)
			mode = Security_Data.type;      /* 1=火灾 2=入侵 */
		/* 2. 根据当前模式执行对应报警时序 */
		if (mode == 1)
			alarm_fire();
		else if (mode == 2)
			alarm_intrusion();
		else
			alarm_idle();                   /* mode=0: 安全, 保持静默 */
		vTaskDelay(pdMS_TO_TICKS(50));
	}
}
