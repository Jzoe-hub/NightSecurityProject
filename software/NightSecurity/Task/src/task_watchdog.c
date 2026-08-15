/**********************************************************************
 * 文件名称： task_watchdog.c
 * 功能描述： WatchdogTask — 心跳监控 + IWDG 喂狗 (500ms / Priority 6 / Stack 128)
 *           任一任务卡死 → 不喂狗 → 系统自动复位
 ***********************************************************************/
#include "task_config.h"
#include "iwdg.h"
#include <stdbool.h>

/*==========看门狗心跳全局变量=============*/
volatile uint32_t g_heartbeat_sensor;
volatile uint32_t g_heartbeat_security;
volatile uint32_t g_heartbeat_alarm;
volatile uint32_t g_heartbeat_ui;
volatile uint32_t g_heartbeat_finger;
volatile uint32_t g_heartbeat_comm;

/* ==================== 看门狗子函数 ==================== */

/**********************************************************************
 * 函数名称： wdg_check_heartbeats
 * 功能描述： 遍历所有需要监控的任务的心跳计数器,
 *           每个被监控任务在自己的循环里对心跳变量自增,
 *           本函数检查：相邻两次调用之间心跳值是否变化。
 *           监控列表：
 *           - g_heartbeat_sensor     (SensorTask 自增)
 *           - g_heartbeat_security   (SecurityTask 自增)
 *           - g_heartbeat_alarm      (AlarmTask 自增)
 *           - g_heartbeat_ui         (UITask 自增)
 *           - g_heartbeat_finger     (FingerTask 自增)
 *           执行流程：
 *           1. static 保存上次各心跳值
 *           2. 逐一比较 当前值 vs 上次值
 *           3. 任一未变化 → 该任务可能卡死 → 返回 false
 *           4. 全部有变化 → 更新上次值 → 返回 true
 * 输入参数： 无
 * 返 回 值： true=全部任务正常, false=至少一个卡死
 ***********************************************************************/
static bool wdg_check_heartbeats(void)
{
	/* TODO */
	static uint32_t last_sensor, last_security, last_alarm, last_ui;
	static uint32_t last_comm;
	static uint32_t last_finger;
	if (g_heartbeat_sensor   == last_sensor)   return false;
	if (g_heartbeat_security == last_security) return false;
	if (g_heartbeat_alarm    == last_alarm)    return false;
	if (g_heartbeat_ui       == last_ui)       return false;
	if (g_heartbeat_comm     == last_comm)     return false;
	if (g_heartbeat_finger   == last_finger)   return false;

	last_sensor   = g_heartbeat_sensor;
	last_security = g_heartbeat_security;
	last_alarm    = g_heartbeat_alarm;
	last_ui       = g_heartbeat_ui;
	last_comm     = g_heartbeat_comm;
	last_finger   = g_heartbeat_finger;

	return true;
}

/**********************************************************************
 * 函数名称： wdg_feed
 * 功能描述： 喂狗 — 重置 IWDG 倒计时。
 *           IWDG 是硬件倒计时器, 超时则芯片自动复位。
 *           每个周期调一次本函数, 倒计时重新从 5 秒开始数。
 *           如果程序卡死、不再调用本函数 → 5 秒后芯片自动重启。
 * 输入参数： 无
 * 返 回 值： 无
 ***********************************************************************/
static void wdg_feed(void)
{
	HAL_IWDG_Refresh(&hiwdg);   /* 重置倒计时, 防止芯片复位 */
}

/* ==================== 任务主函数 ==================== */

/**********************************************************************
 * 函数名称： WatchdogTask
 * 功能描述： 每 500ms 检查一轮心跳, 全部正常才喂狗。
 *           执行流程：
 *           1. if (wdg_check_heartbeats()) → wdg_feed()
 *           2. else → 不喂狗, 等待 IWDG 超时复位系统
 * 输入参数： pvParameters — 未使用
 * 返 回 值： 无
 ***********************************************************************/
void WatchdogTask(void *pvParameters)
{
	TickType_t xLastWakeTime = xTaskGetTickCount();
	(void)pvParameters;

	for (;;)
	{
		/* TODO: wdg_check_heartbeats → wdg_feed */
		if(wdg_check_heartbeats())
		{
			wdg_feed();
		}
		vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(500));
	}
}
