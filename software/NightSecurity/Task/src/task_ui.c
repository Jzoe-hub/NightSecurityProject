/**********************************************************************
 * 文件名称： task_ui.c
 * 功能描述： UITask — OLED 显示 + 按键处理 + 菜单 (200ms / Priority 2 / Stack 384)
 * 说    明： 三段式结构 — 报警页(霸屏) → 按键处理(翻页/设置) → 绘图刷新
 ***********************************************************************/
#include "task_config.h"
#include "oled.h"
#include "key.h"
#include "voice.h"
#include <stdio.h>

/* ==================== 内部状态变量 ==================== */
static uint8_t cur_page   = 0;      /* 0=监控页, 1=设置页          */
static uint8_t need_clear = 1;      /* 页面切换标记, 1=需要清屏    */
/* g_sw_armed 已改为全局变量 g_sw_armed, 定义在 task_config.c */
static uint8_t cursor     = 0;      /* 设置页光标位置              */
static uint8_t adj_dir    = 0;      /* 0=上调(+) 1=下调(-)        */

/* ---- 阈值 (默认值, 可在设置页调节) ---- */
uint16_t th_fire  = 2000;   /* 火焰强度 — SecurityTask 共用  */
uint8_t  th_smoke = 10;     /* 烟雾 ppm  — SecurityTask 共用  */
uint8_t  th_co    = 10;     /* CO ppm    — SecurityTask 共用  */
uint8_t  th_temp  = 45;     /* 温度 ℃   — SecurityTask 共用  */
uint8_t  th_pir   = 1;      /* PIR 触发  — SecurityTask 共用  */

/* ==================== 显示子函数 ==================== */

/**********************************************************************
 * 函数名称： ui_show_sensors
 * 功能描述： 页面 0 — 实时传感器数据, 4 行 OLED, 用 sprintf 格式化数值
 *           数据来源: Sensor_Data.fire.xxx 和 Sensor_Data.intrusion.xxx
 ***********************************************************************/
static void ui_show_sensors(void)
{
	char buf[16];
	OLED_Clear();
	OLED_PrintString(0, 0, "- SAFETY MON -");                /* 15 字符 */
	sprintf(buf, "ARM:%s", g_sw_armed ? "ON " : "OFF");        /* 安防开关 */
	OLED_PrintString(0, 2, buf);
	sprintf(buf, "T:%dC S:%d G:%d", Sensor_Data.fire.temp,   /* 温/烟/气 */
	        (int)Sensor_Data.fire.smoke_ppm, (int)Sensor_Data.fire.co_ppm);
	OLED_PrintString(0, 4, buf);
	sprintf(buf, "F:%d P:%d R:%d",                            /* 火/PIR/雷达 */
	        (int)Sensor_Data.fire.fire_int,
	        Sensor_Data.intrusion.pir_triggered,
	        Sensor_Data.intrusion.has_person);
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
	char buf[16];
	OLED_Clear();
	OLED_PrintString(0, 0, "- WARNING -");                  /* 12 字符 */
	OLED_PrintString(0, 2, (type == 1) ? "TYPE: FIRE" : "TYPE: ENEMY");
	sprintf(buf, "LV:%s", (level == 1) ? "WARN" : "ALARM"); /* 报警等级 */
	OLED_PrintString(0, 4, buf);
	OLED_PrintString(0, 6, "FP UNLCK K1:EXIT");             /* 16 字符, 刚好 */
}

/**********************************************************************
 * 函数名称： ui_show_setting
 * 功能描述： 页面 1 — 阈值调整界面, 光标 '>' 指示当前调节项
 *           每行一种阈值: fire / smoke / CO
 *           用 cursor 变量控制 '>' 显示在哪一行
 ***********************************************************************/
static void ui_show_setting(void)
{
	char buf[16];
	OLED_Clear();
	OLED_PrintString(0, 0, "-SETTING-");
	sprintf(buf, "[%s]%s", adj_dir ? "-" : "+", (cursor == 5) ? "<" : " ");
	OLED_PrintString(10, 0, buf);
	sprintf(buf, "%sF:%d %sT:%dC", (cursor==0)?">":" ", th_fire, (cursor==1)?">":" ", th_temp);
	OLED_PrintString(0, 2, buf);
	sprintf(buf, "%sS:%d %sG:%d", (cursor==2)?">":" ", th_smoke, (cursor==3)?">":" ", th_co);
	OLED_PrintString(0, 4, buf);
	sprintf(buf, "%sPIR:%d", (cursor==4)?">":" ", th_pir);
	OLED_PrintString(0, 6, buf);
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
 *              KEY1 可退出报警 (g_sw_armed=0, cur_page=0, need_clear=1)。
 *              注意: return 跳过后续, 报警期间不响应 KEY2/KEY3。
 *           B. 按键处理 (同一按键在不同页面作用不同):
 *              KEY1 (翻页):           cur_page = !cur_page, need_clear = 1
 *              KEY2:
 *                监控页:              g_sw_armed = !g_sw_armed (布防/撤防)
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
	/* 报警优先: 有报警则霸屏 */
	if (state_result > 0)
	{
		ui_show_alarm(Security_Data.type, Security_Data.level);
		if (key_event == 1)             /* KEY1: 撤防 → 解除报警 */
				g_sw_armed = 0;
		return;
	}
	if (key_event == 1)              /* KEY1: 翻页 */
	{
		cur_page = !cur_page;
		need_clear = 1;
	}
	else if (cur_page == 0 && key_event == 2)    /* 监控页: 布防/撤防 */
	{
		g_sw_armed = !g_sw_armed;
		if (g_sw_armed)                          /* 布防成功 */
			Voice_IO4_Trigger();
	}
	else if (cur_page == 1 && key_event == 2)    /* 设置页: 光标下移 */
	{
		cursor++;
		if (cursor > 5) cursor = 0;            /* 0=F 1=T 2=S 3=G 4=P 5=Dir */
	}
	else if (cur_page == 1 && key_event == 3)    /* 设置页: 调值/切方向 */
	{
		if (cursor == 5)                       /* 光标在方向位..翻方向 */
			adj_dir = !adj_dir;
		else
		{
			int step = adj_dir ? -1 : 1;       /* 0=加 1=减 */
			if (cursor == 0) th_fire  += step;
			if (cursor == 1) th_temp  += step;
			if (cursor == 2) th_smoke += step;
			if (cursor == 3) th_co    += step;
			if (cursor == 4) th_pir   += step;
		}
	}
	if (cur_page == 0)
		ui_show_sensors();
	else
		ui_show_setting();
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
