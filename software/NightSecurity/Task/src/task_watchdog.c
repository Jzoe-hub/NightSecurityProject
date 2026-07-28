/**********************************************************************
 * 文件名称： task_watchdog.c
 * 功能描述： WatchdogTask — 心跳监控 + IWDG 喂狗 (500ms / Priority 6 / Stack 128)
 *           任一任务卡死 → 不喂狗 → 系统自动复位
 ***********************************************************************/
#include "task_config.h"
#include "stm32f1xx_hal.h"   /* IWDG_HandleTypeDef hiwdg */
#include <stdbool.h>

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
	return true;
}

/**********************************************************************
 * 函数名称： wdg_feed
 * 功能描述： 调用 HAL_IWDG_Refresh() 重置独立看门狗计数器。
 *           注意：IWDG 一旦使能就无法关闭, 必须在超时前喂狗。
 *           执行流程：
 *           1. HAL_IWDG_Refresh(&hiwdg)
 * 输入参数： 无
 * 返 回 值： 无
 ***********************************************************************/
static void wdg_feed(void)
{
	/* TODO */
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
		vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(500));
	}
}
