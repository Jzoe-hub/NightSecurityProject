/**********************************************************************
 * 文件名称： task_finger.c
 * 功能描述： FingerTask — 指纹录入/验证/删除 (事件驱动 / Priority 2 / Stack 384)
 *           由 TOUCH 引脚事件唤醒, 执行对应指纹操作
 ***********************************************************************/
#include "task_config.h"
#include "finger.h"
#include <stdbool.h>

/* ==================== 指纹流程子函数 ==================== */

/**********************************************************************
 * 函数名称： finger_wait_touch
 * 功能描述： 轮询 Finger_IsTouched() 等待手指按下, 带超时。
 *           执行流程：
 *           1. 记录起始 tick
 *           2. 循环: 读 Finger_IsTouched()
 *           3. 若为 1 → 返回 true
 *           4. 若超时(timeout_ms 到期) → 返回 false
 *           5. 每次循环 vTaskDelay(50ms) 避免占满 CPU
 * 输入参数： timeout_ms — 超时时间 (ms), 0=永久等待
 * 返 回 值： true=检测到手指, false=超时
 ***********************************************************************/
static bool finger_wait_touch(uint32_t timeout_ms)
{
	/* TODO */
	return false;
}

/**********************************************************************
 * 函数名称： finger_do_enroll
 * 功能描述： 执行一次指纹注册, 共 5 次采图, 每次 OLED 提示进度。
 *           执行流程：
 *           1. OLED_PrintString 提示 "Place Finger"
 *           2. finger_wait_touch(5000) — 等手指
 *           3. Finger_Enroll() — 调用 ZW101 注册流程
 *           4. 返回 true/false, OLED 显示结果
 * 输入参数： 无
 * 返 回 值： true=注册成功
 ***********************************************************************/
static bool finger_do_enroll(void)
{
	/* TODO */
	return false;
}

/**********************************************************************
 * 函数名称： finger_do_verify
 * 功能描述： 执行一次指纹验证, 结果通过事件组通知 SecurityTask。
 *           执行流程：
 *           1. OLED 提示 "Verify Finger"
 *           2. finger_wait_touch(5000) — 等手指
 *           3. Finger_Search() — 搜索指纹库
 *           4. 成功 → OLED 显示 "Pass" + 事件组置位通知 SecurityTask
 *           5. 失败 → OLED 显示 "Fail"
 * 输入参数： 无
 * 返 回 值： true=验证通过
 ***********************************************************************/
static bool finger_do_verify(void)
{
	/* TODO */
	return false;
}

/* ==================== 任务主函数 ==================== */

/**********************************************************************
 * 函数名称： FingerTask
 * 功能描述： 循环等待手指按下, 根据当前模式执行录入或验证。
 *           执行流程：
 *           1. finger_wait_touch(0) — 永久等待手指
 *           2. 检测到手指 → 根据全局 mode 变量决定：
 *              MODE_ENROLL → finger_do_enroll()
 *              MODE_VERIFY → finger_do_verify()
 * 输入参数： pvParameters — 未使用
 * 返 回 值： 无
 ***********************************************************************/
void FingerTask(void *pvParameters)
{
	(void)pvParameters;

	for (;;)
	{
		/* TODO: 等手指 → 根据模式执行操作 */
		vTaskDelay(pdMS_TO_TICKS(500));
	}
}
