/**********************************************************************
 * 文件名称： task_finger.c
 * 功能描述： FingerTask — 指纹验证 (事件驱动 / Priority 2 / Stack 384)
 *           Phase 2 仅验证, 录入功能 Phase 5 由 APP 远程实现
 ***********************************************************************/
#include "task_config.h"
#include "finger.h"
#include "oled.h"
#include <stdbool.h>

/* ==================== 指纹流程子函数 ==================== */

/**********************************************************************
 * 函数名称： finger_wait_touch
 * 功能描述： 轮询 Finger_IsTouched() 等待手指按下。
 *           timeout_ms=0 → 永久等待, 其余值 → 每 ms 检查一次直到超时
 * 输入参数： timeout_ms — 超时时间 (ms), 0=永久等待
 * 返 回 值： true=检测到手指, false=超时
 ***********************************************************************/
static bool finger_wait_touch(uint32_t timeout_ms)
{
	if (timeout_ms == 0)                    /* 0 = 永久等待            */
	{
		while (!Finger_IsTouched())
			vTaskDelay(pdMS_TO_TICKS(50));   /* 50ms 轮询, 不占满 CPU   */
		return true;
	}
	for (int tick = 0; tick < timeout_ms; tick++)   /* 带超时, 1ms 精度 */
	{
		if (Finger_IsTouched())
			return true;
		vTaskDelay(pdMS_TO_TICKS(1));
	}
	return false;
}

/**********************************************************************
 * 函数名称： finger_do_enroll      (Phase 5 APP 远程录入, 当前留空)
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
	return false;
}

/**********************************************************************
 * 函数名称： finger_do_verify
 * 功能描述： 执行一次指纹验证 — 等手指 → 搜索指纹库 → 显示结果
 * 输入参数： 无
 * 返 回 值： true=验证通过
 ***********************************************************************/
static bool finger_do_verify(void)
{
	OLED_Clear();
	OLED_PrintString(0,0,"Verify Finger");

	if(!finger_wait_touch(5000))
		return false;
	bool ok = Finger_Search();
	OLED_PrintString(0,2,ok ? "Finger:Pass":"Finger:Fail");
	return ok;
}

/* ==================== 任务主函数 ==================== */

/**********************************************************************
 * 函数名称： FingerTask
 * 功能描述： 循环: 等手指 → 执行验证 → 延时 500ms, 周而复始
 *           录入功能 Phase 5 由 APP 远程实现, 届时加模式判断
 ***********************************************************************/
void FingerTask(void *pvParameters)
{
	(void)pvParameters;

	for (;;)
	{
		finger_wait_touch(0);               /* 永久等待手指按下         */
		finger_do_verify();                 /* 执行搜索 + 显示结果      */
		vTaskDelay(pdMS_TO_TICKS(500));
	}
}
