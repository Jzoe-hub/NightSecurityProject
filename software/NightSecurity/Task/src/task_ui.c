/**********************************************************************
 * 文件名称： task_ui.c
 * 功能描述： UITask — OLED 显示 + 按键处理 + 菜单状态机
 *           (200ms / Priority 2 / Stack 384)
 ***********************************************************************/
#include "task_config.h"
#include "oled.h"
#include "key.h"
#include <stdio.h>

/* ==================== 显示子函数 ==================== */

/**********************************************************************
 * 函数名称： ui_show_sensors
 * 功能描述： 显示传感器实时数据页面, 4 行 OLED 分别显示：
 *           Line 0: "SENSOR DATA"
 *           Line 2: "MQ2:xxxx PPM:xx.x"   (sprintf 格式化)
 *           Line 4: "MQ7:xxxx CO: xx.x"
 *           Line 6: "TMP:xxC HUM:xx%"
 *           注意：各传感器数据从全局变量或队列获取,
 *           OLED_PrintString 前先用 sprintf(buf, ...) 格式化
 * 输入参数： （待定, 后续从队列/全局变量获取各传感器值）
 * 返 回 值： 无
 ***********************************************************************/
static void ui_show_sensors(void)
{
	/* TODO */
}

/**********************************************************************
 * 函数名称： ui_show_status
 * 功能描述： 显示系统状态页面：
 *           Line 0: "== STATUS =="
 *           Line 2: "ARMED" 或 "DISARMED"
 *           Line 4: "FIRE:OK" 或 "FIRE:WARN" 或 "FIRE:ALARM"
 *           Line 6: "INTR:OK" 或 "INTR:WARN" 或 "INTR:ALARM"
 * 输入参数： armed — 布防状态, fire_stat — 火灾状态, intr_stat — 入侵状态
 * 返 回 值： 无
 ***********************************************************************/
static void ui_show_status(uint8_t armed, uint8_t fire_stat, uint8_t intr_stat)
{
	/* TODO */
}

/* ==================== 按键子函数 ==================== */

/**********************************************************************
 * 函数名称： ui_scan_keys
 * 功能描述： 轮询 KEY1/KEY2/KEY3, 软件去抖后返回按键事件。
 *           去抖机制：每 200ms 读一次, 连续两次读到同一电平才确认。
 *           执行流程：
 *           1. 读 Key1_IsPressed() → 和前次比较 → 一致则确认按下
 *           2. 同法处理 Key2 / Key3
 *           3. 返回按键编号（同一时刻只返回一个键）
 * 返 回 值： 0=无按键, 1=KEY1按下, 2=KEY2按下, 3=KEY3按下
 ***********************************************************************/
static uint8_t ui_scan_keys(void)
{
	/* TODO */
	return 0;
}

/**********************************************************************
 * 函数名称： ui_run_menu
 * 功能描述： 菜单状态机, 根据按键事件切换页面和执行操作。
 *           菜单结构：
 *           PAGE_MAIN    (主界面):  显示传感器数据
 *              KEY1 → 切换到状态页
 *              KEY2 → 布防/撤防切换
 *           PAGE_STATUS  (状态页):  显示系统状态
 *              KEY1 → 切回主界面
 *              KEY3 → 进入设置页 (预留)
 *           PAGE_SETTING (设置页):  预留
 *           执行流程：
 *           1. switch(current_page)
 *           2. case PAGE_MAIN:   根据按键切换页面 / 触发操作
 *           3. case PAGE_STATUS: 根据按键切换页面
 *           4. 最后根据 current_page 调用对应的 ui_show_xxx()
 * 输入参数： key_event — 当前按键事件 (0=无, 1/2/3)
 * 返 回 值： 无
 ***********************************************************************/
static void ui_run_menu(uint8_t key_event)
{
	/* TODO */
}

/* ==================== 任务主函数 ==================== */

/**********************************************************************
 * 函数名称： UITask
 * 功能描述： 每 200ms 执行一轮 UI 刷新。执行顺序：
 *           1. ui_scan_keys()  — 读按键, 去抖
 *           2. ui_run_menu()   — 菜单状态机, 决定显示哪个页面
 *           (ui_show_xxx 由 ui_run_menu 内部调用)
 * 输入参数： pvParameters — 未使用
 * 返 回 值： 无
 ***********************************************************************/
void UITask(void *pvParameters)
{
	TickType_t xLastWakeTime = xTaskGetTickCount();
	(void)pvParameters;

	for (;;)
	{
		/* TODO: ui_scan_keys → ui_run_menu */
		vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(200));
	}
}
