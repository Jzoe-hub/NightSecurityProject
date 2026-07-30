/**********************************************************************
 * 文件名称： task_ui.c
 * 功能描述： UITask — OLED 显示 + 按键处理 + 菜单 (200ms / Priority 2 / Stack 384)
 * 说    明： 三段式结构 — 报警页(霸屏) → 按键处理(翻页/设置) → 绘图刷新
 ***********************************************************************/
#include "task_config.h"
#include "oled.h"
#include "key.h"
#include <stdio.h>

/* ==================== 内部状态变量 ==================== */
static uint8_t cur_page   = 0;      /* 0=监控页, 1=设置页          */
static uint8_t need_clear = 1;      /* 页面切换标记, 1=需要清屏    */
static uint8_t sw_armed   = 0;      /* 0=撤防, 1=布防              */
static uint8_t cursor     = 0;      /* 设置页光标位置              */

/* ---- 阈值 (默认值, 可在设置页调节) ---- */
static uint16_t th_fire  = 2000;
static uint8_t  th_smoke = 10;
static uint8_t  th_co    = 10;

/* ==================== 显示子函数 ==================== */

/**********************************************************************
 * 函数名称： ui_show_sensors
 * 功能描述： 页面 0 — 实时传感器数据, 4 行 OLED, 用 sprintf 格式化数值
 *           数据来源: Sensor_Data.fire.xxx 和 Sensor_Data.intrusion.xxx
 ***********************************************************************/
static void ui_show_sensors(void)
{
	char buf[16];
	OLED_PrintString(0, 0, "SENSOR DATA");
	sprintf(buf, "MQ2:%.1f PPM", Sensor_Data.fire.smoke_ppm);
	OLED_PrintString(0, 2, buf);
	sprintf(buf, "CO:%.1f PPM", Sensor_Data.fire.co_ppm);
	OLED_PrintString(0, 4, buf);
	sprintf(buf, "T:%dC H:%d%%", Sensor_Data.fire.temp, Sensor_Data.fire.hum);
	OLED_PrintString(0, 6, buf);
}

/**********************************************************************
 * 函数名称： ui_show_alarm
 * 功能描述： 页面2 报警页 — 覆盖所有页面, 显示报警类型和等级
 *           KEY1 可退出报警
 * 输入参数： type — 1=火灾 2=入侵
 *            level — 1=预警 2=报警
 ***********************************************************************/
static void ui_show_alarm(uint8_t type, uint8_t level)
{
	OLED_PrintString(0, 0, "!! WARNING !!");
	OLED_PrintString(0, 1, (type == 1) ? "TYPE: FIRE" : "TYPE: INTR");
	OLED_PrintString(0, 2, (level == 1) ? "LEV: WARN" : "LEV: ALARM");
	OLED_PrintString(0, 3, "KEY1: STOP ALARM");
}

/**********************************************************************
 * 函数名称： ui_show_setting
 * 功能描述： 页面 1 — 阈值调整界面, 光标 '>' 指示当前调节项
 *           每行一种阈值: fire / smoke / CO
 *           用 cursor 变量控制 '>' 显示在哪一行
 ***********************************************************************/
static void ui_show_setting(void)
{
	/* TODO */
	char buf[16];
	OLED_PrintString(0, 0, "SETTING");
	sprintf(buf, "%sF-Lim: %d",  (cursor == 0) ? ">" : " ", th_fire);
	OLED_PrintString(0, 2, buf);
	sprintf(buf, "%sS-Lim: %d",  (cursor == 1) ? ">" : " ", th_smoke);
	OLED_PrintString(0, 4, buf);
	sprintf(buf, "%sCO-Lim: %d", (cursor == 2) ? ">" : " ", th_co);
	OLED_PrintString(0, 6,buf);
}

/* ==================== 按键子函数 ==================== */

/**********************************************************************
 * 函数名称： ui_scan_keys
 * 功能描述： 阻塞式去抖 — 检测到按下后等 20ms 再确认,
 *           确认后 while 循环等松开, 再等 20ms 消除抖动, 最后返回键号。
 *           只有上一个键完全松开后才能检测下一个键 (else if 链)。
 *           执行流程:
 *           1. if (Key1_IsPressed()) → HAL_Delay(20)
 *           2.   if (Key1_IsPressed()) → while(Key1_IsPressed()); 等松开
 *           3.   HAL_Delay(20); return 1;
 *           4. else if (Key2_IsPressed()) → 同上, return 2;
 *           5. else if (Key3_IsPressed()) → 同上, return 3;
 * 返 回 值： 0=无按键, 1=KEY1, 2=KEY2, 3=KEY3
 ***********************************************************************/
static uint8_t ui_scan_keys(void)
{
	/* TODO */
	if (Key1_IsPressed()==1)
	{
		HAL_Delay(20);
		if (Key1_IsPressed())
		while(Key1_IsPressed());
		HAL_Delay(20);
		return 1;
	}else if(Key2_IsPressed()==1){
		HAL_Delay(20);
		if (Key2_IsPressed())
		while(Key2_IsPressed());
		HAL_Delay(20);
		return 2;
	}else if(Key3_IsPressed()==1){
		HAL_Delay(20);
		if (Key3_IsPressed())
		while(Key3_IsPressed());
		HAL_Delay(20);
		return 3;
	}
	return 0;
}

/* ==================== 核心逻辑 ==================== */

/**********************************************************************
 * 函数名称： ui_run_menu
 * 功能描述： 三段式调度 — 报警优先 → 按键处理 → 绘图刷新。
 *           执行流程:
 *           A. 报警判断: state_result > 0 则霸屏显示报警页,
 *              KEY1 可退出报警 (sw_armed=0, cur_page=0, need_clear=1)。
 *              注意: return 跳过后续, 报警期间不响应 KEY2/KEY3。
 *           B. 按键处理 (同一按键在不同页面作用不同):
 *              KEY1 (翻页):           cur_page = !cur_page, need_clear = 1
 *              KEY2:
 *                监控页:              sw_armed = !sw_armed (布防/撤防)
 *                设置页:              光标下移 cursor++, 到头回绕
 *              KEY3:
 *                监控页:              无操作
 *                设置页:              当前选中阈值 +1 (或配合方向键加减)
 *           C. 绘图刷新:
 *              if (need_clear) { OLED_Clear(); need_clear = 0; }
 *              if (cur_page == 0) ui_show_sensors();
 *              else               ui_show_setting();
 * 输入参数： key_event — ui_scan_keys() 返回值, 0=无按键
 ***********************************************************************/
static void ui_run_menu(uint8_t key_event)
{
	/* TODO */
	if(state_result>0)
	{
		 ui_show_alarm(Security_Data.type,Security_Data.level);
		 return;
	}else if(key_event == 1)
	{
		cur_page = !cur_page;
		need_clear = 1;
	}else if(cur_page==0&&key_event==2)
	{
		sw_armed = !sw_armed;
	}else if(cur_page==1&&key_event==2)
	{
		cursor++;
		if (cursor > 2) cursor = 0;    // 3 个选项, 到头循环
	}else if(cur_page==1&&key_event==3)
	{
		if(cursor==0)th_fire++;
		if(cursor==1)th_smoke++;
		if(cursor==2)th_co++;
	}
	if(need_clear == 1)
	{
		OLED_Clear();
		need_clear = 0;
	}
	if (cur_page == 0)
	{
		ui_show_sensors();
	}
	else ui_show_setting();
}

/* ==================== 任务主函数 ==================== */

/**********************************************************************
 * 函数名称： UITask
 * 功能描述： 每 200ms: 扫描按键 → 处理菜单 → 刷新 OLED
 ***********************************************************************/
void UITask(void *pvParameters)
{
	(void)pvParameters;

	for (;;)
	{
		g_heartbeat_ui++;
		uint8_t key = ui_scan_keys();
		ui_run_menu(key);
		vTaskDelay(pdMS_TO_TICKS(200));
	}
}
