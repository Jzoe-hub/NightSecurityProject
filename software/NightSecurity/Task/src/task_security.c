/**********************************************************************
 * 文件名称： task_security.c
 * 功能描述： SecurityTask — 安全判定任务 (100ms / Priority 4 / Stack 512)
 *           传感器数据融合 + 火灾/入侵判定 + 状态机 + 报警调度
 ***********************************************************************/
#include "task_config.h"
#include "voice.h"
#include <stdio.h>    /* snprintf */
#include <string.h>
#include <stdlib.h>   /* atoi */

/*======================宏定义和全局变量====================*/
#define STATE_DISARMED  0
#define STATE_ARMED     1
#define STATE_WARNING   2
#define STATE_ALARM     3

uint8_t fire_level;
uint8_t intrusion_level;
uint8_t state_result;
AlarmCMD Security_Data;
uint8_t g_sw_armed = 0;

/* ==================== 融合处理子函数 ==================== */

/**********************************************************************
 * 函数名称： check_fire
 * 功能描述： 综合火焰强度 + MQ-2 烟雾 + MQ-7 CO + 温度四个维度,
 *           加权评分判断是否发生火灾。
 *           评分逻辑（每项 0~100 分, 加权求和）：
 *           1. 火焰强度: Fire_GetIntensity() / 4095 * 100 * 0.4
 *           2. MQ-2 烟雾: smoke_ppm > 阈值 → 分值×0.3
 *           3. MQ-7 CO:   co_ppm   > 阈值 → 分值×0.2
 *           4. 温度:      temp > 45℃      → 分值×0.1
 *           总分 ≥ 60 → 返回 2(确认火灾)
 *           总分 ≥ 30 → 返回 1(预警)
 *           总分 < 30 → 返回 0(安全)
 * 输入参数： smoke_ppm / co_ppm / fire_int / temp — 传感器数据
 * 返 回 值： 0=安全, 1=火灾预警, 2=确认火灾
 ***********************************************************************/
static uint8_t check_fire(float smoke_ppm, float co_ppm,
        uint16_t fire_int, int temp)
{
    float score = 0;
    /* 1. 火焰强度: 超过阈值, 贡献 40 分 */
    if (fire_int > th_fire)
        score += 40;
    /* 2. MQ-2 烟雾: 超过阈值, 贡献 30 分 */
    if (smoke_ppm > th_smoke)
        score += 30;
    /* 3. MQ-7 CO: 超过阈值, 贡献 20 分 */
    if (co_ppm > th_co)
        score += 20;
    /* 4. 温度: 超过阈值, 贡献 10 分 */
    if (temp > th_temp)
        score += 10;
    /* 判定: 总分 100, ≥60 确认火灾, ≥30 预警 */
    if (score < 30)       return 0;
    else if (score < 60)  return 1;
    else                  return 2;
}


/**********************************************************************
 * 函数名称： check_intrusion
 * 功能描述： 综合 PIR + 雷达运动目标 + 雷达静止目标三个维度,
 *           加权评分判断是否有人入侵。
 *           评分逻辑：
 *           1. PIR/雷达检测到有人:   直接贡献 30 分
 *           2. 雷达运动目标: has_motion → 25 分
 *              + 距离 < 200cm → 额外 15 分
 *           3. 雷达静止目标: has_static → 50 分（静止人体置信度高）
 *           总分 ≥ 60 → 返回 2(确认入侵)
 *           总分 ≥ 30 → 返回 1(可疑)
 *           总分 < 30 → 返回 0(安全)
 * 输入参数： pir / has_person / motion_dist / has_motion / has_static
 * 返 回 值： 0=安全, 1=可疑, 2=确认入侵
 ***********************************************************************/
static uint8_t check_intrusion(uint8_t pir, uint8_t has_person,
		uint16_t motion_dist, uint8_t motion_energy, uint8_t static_energy)
{
	float score = 0;
	/* 1. PIR 或雷达有人: 贡献 30 分 */
	if (pir == 1 || has_person == 1)
		score += 30;
	/* 2. 目标距离 < 200cm: 额外 15 分 */
	if (motion_dist < 200)
		score += 15;
	/* 3. 运动能量 ≥ 1: 有运动目标, 贡献 25 分 */
	if (motion_energy >= 1)
		score += 25;
	/* 4. 静止能量 ≥ 1: 有静止人体, 贡献 50 分（置信度最高） */
	if (static_energy >= 1)
		score += 50;
	/* 判定: 总分 100, ≥60 确认入侵, ≥30 可疑 */
	if      (score < 30) return 0;
	else if (score < 60) return 1;
	else                 return 2;
}

/**********************************************************************
 * 函数名称： run_state_machine
 * 功能描述： 安全状态机, 根据当前的火灾/入侵判定结果和历史状态,
 *           决定是否进入报警状态。
 *           状态定义：
 *           - STATE_DISARMED (撤防):  不检测入侵, 仅检测火灾
 *           - STATE_ARMED    (布防):  检测火灾 + 入侵
 *           - STATE_WARNING  (预警):  有可疑事件, 等待二次确认
 *           - STATE_ALARM    (报警):  确认危险, 触发声光
 *           状态转换规则：
 *           1. 撤防→预警: 火灾判定≥1（入侵判定被忽略）
 *           2. 布防→预警: 火灾判定≥1 或 入侵判定≥1
 *           3. 预警→报警: 连续 3 次判定≥1（防误报, 用 counter 计数）
 *           4. 预警→布防: 连续 5 次判定=0（自动恢复）
 *           5. 报警→布防: 收到撤防指令 + 判定=0
 * 输入参数： fire_level / intrusion_level — 本轮判定结果
 * 返 回 值： 0=无报警, 1=预警, 2=报警
 ***********************************************************************/
static uint8_t run_state_machine(uint8_t fire_level, uint8_t intrusion_level)
{
	static uint8_t state   = STATE_DISARMED;   // 当前状态
	static uint8_t counter = 0;                 // 连续确认/安全计数

	/* 根据 g_sw_armed 切换撤防/布防 */
	if (g_sw_armed && state == STATE_DISARMED)
		state = STATE_ARMED;
	if (!g_sw_armed && state != STATE_DISARMED)
		state = STATE_DISARMED;

	/*
	 * 状态转换总览:
	 *   撤防 ──(火灾≥1)──→ 预警 ──(3次确认)──→ 报警
	 *   布防 ──(火/入侵≥1)──→ 预警 ──(连续安全)──→ 布防
	 */
	switch (state)
	{
		case STATE_DISARMED:
			/* 1. 撤防: 不触发任何报警, 只采集数据 */
			return 0;
	case STATE_ARMED:
		/* 2. 布防: 火灾或入侵任一触发即进入预警 */
		if (fire_level >= 1 || intrusion_level >= 1)
		{
			state   = STATE_WARNING;
			counter = 1;
			return 1;
		}
		return 0;
		break;
	case STATE_WARNING:
		/* 3. 预警: 连续 3 次判定≥1 → 报警; 连续安全 → 回布防 */
		if (fire_level >= 1 || intrusion_level >= 1)
		{
			counter++;
			if (counter >= 3)               // 3 次确认 → 报警
			{
				state = STATE_ALARM;
				return 2;
			}
			return 1;
		}
		else
		{
			counter--;
			if (counter <= 0)               // 连续安全 → 回布防
			{
				state = STATE_ARMED;
			}
			return 0;
		}
		break;
	case STATE_ALARM:
		/* 4. 报警: 保持输出, 等撤防指令解锁 */
		return 2;
		break;
	}
	return 0;
}

/**********************************************************************
 * 函数名称： dispatch_alarm
 * 功能描述： 根据最终报警等级 构建报警命令结构体,
 *           通过队列发送给 AlarmTask。
 *           报警命令包含：
 *           - type:  1=火灾, 2=入侵
 *           - level: 1=预警, 2=报警
 *           执行流程：
 *           1. 构建 AlarmCmd 结构体
 *           2. xQueueSend(g_alarmQueue, &cmd, 0)
 * 输入参数： level — 报警等级
 * 返 回 值： 无
 ***********************************************************************/
static void dispatch_alarm(uint8_t level, uint8_t type)
{
	if (level == 0) return;                         /* 无报警, 不发送 */
	Security_Data.type  = type;
	Security_Data.level = level;
	xQueueSend(g_securityQueue, &Security_Data, 0); /* 发给 AlarmTask */
}

/**********************************************************************
 * 函数名称： json_get_int
 * 功能描述： 在 JSON 字符串中查找 "key":value, 返回 value 的整数值
 ***********************************************************************/
static int json_get_int(const char *json, const char *key)
{
    char search[16];
    snprintf(search, sizeof(search), "\"%s\":", key);
    char *pos = strstr((char*)json, search);
    if (pos) return atoi(pos + strlen(search));
    return -1;
}

/* ==================== 任务主函数 ==================== */

/**********************************************************************
 * 函数名称： SecurityTask
 * 功能描述： 每 100ms 执行一轮安全判定。执行顺序：
 *           1. 从队列接收 SensorTask 发来的传感器数据包
 *           2. check_fire()      — 火灾评分
 *           3. check_intrusion() — 入侵评分
 *           4. run_state_machine() — 状态机, 输出最终报警等级
 *           5. 若等级 > 0 → dispatch_alarm() 发送报警命令
 * 输入参数： pvParameters — 未使用
 * 返 回 值： 无
 ***********************************************************************/
void SecurityTask(void *pvParameters)
{
	(void)pvParameters;
	for (;;)
	{
		g_heartbeat_security++;

		/* ---- 处理 APP 下发命令 (非阻塞) ---- */
		CloudRxPacket cmd;
		if (xQueueReceive(g_cmdQueue, &cmd, 0) == pdPASS)
		{
			if (strstr((char*)cmd.json, "\"action\":\"arm\"")) {
				if (g_sw_armed == 0) {
					Voice_IO4_Trigger();          /* 只在 0→1 时触发布防语音 */
				}
				g_sw_armed = 1;
			} else if (strstr((char*)cmd.json, "\"action\":\"disarm\"")) {
				g_sw_armed = 0;
			} else if (strstr((char*)cmd.json, "\"config\"")) {
				int v;
				v = json_get_int((char*)cmd.json, "fire");  if (v >= 0) th_fire  = (uint16_t)v;
				v = json_get_int((char*)cmd.json, "smoke"); if (v >= 0) th_smoke = (uint8_t)v;
				v = json_get_int((char*)cmd.json, "co");    if (v >= 0) th_co    = (uint8_t)v;
				v = json_get_int((char*)cmd.json, "temp");  if (v >= 0) th_temp  = (uint8_t)v;
				v = json_get_int((char*)cmd.json, "pir");   if (v >= 0) th_pir   = (uint8_t)v;
			}
		}

		xQueueReceive(g_sensorQueue, &Sensor_Data, portMAX_DELAY); // 等队列同步过来传感器数据包
		fire_level = check_fire(Sensor_Data.fire.smoke_ppm,
				Sensor_Data.fire.co_ppm,
				Sensor_Data.fire.fire_int,
				Sensor_Data.fire.temp);
		intrusion_level = check_intrusion(Sensor_Data.intrusion.pir_triggered,
										Sensor_Data.intrusion.has_person,
										Sensor_Data.intrusion.motion_dist,
										Sensor_Data.intrusion.motion_energy,
										Sensor_Data.intrusion.static_energy);
		state_result = run_state_machine(fire_level, intrusion_level); // 状态机
		uint8_t type = (fire_level>=intrusion_level)?1:2;
		dispatch_alarm(state_result,type);
		vTaskDelay(pdMS_TO_TICKS(100));
	}
}
