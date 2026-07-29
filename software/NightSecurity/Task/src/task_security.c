/**********************************************************************
 * 文件名称： task_security.c
 * 功能描述： SecurityTask — 安全判定任务 (100ms / Priority 4 / Stack 512)
 *           传感器数据融合 + 火灾/入侵判定 + 状态机 + 报警调度
 ***********************************************************************/
#include "task_config.h"

/*======================宏定义和全局变量====================*/
#define STATE_DISARMED  0
#define STATE_ARMED     1
#define STATE_WARNING   2
#define STATE_ALARM     3

uint8_t fire_level;
uint8_t intrusion_level;
uint8_t state_result;
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
    /* 1. 火焰强度: 大于 2000 说明有明显火焰, 贡献 40 分 */
    if (fire_int > 2000)
        score += 40;
    /* 2. MQ-2 烟雾: 超过 10ppm 说明有烟雾, 贡献 30 分 */
    if (smoke_ppm > 10)
        score += 30;
    /* 3. MQ-7 CO: 超过 10ppm 说明有一氧化碳, 贡献 20 分 */
    if (co_ppm > 10)
        score += 20;
    /* 4. 温度: 超过 45℃ 说明环境异常升温, 贡献 10 分 */
    if (temp > 45)
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
	/* TODO */
	float score = 0;
	if(pir == 1||has_person==1)
	{
		score = score + 30;
	}
	if(motion_dist < 200)
	{
		score = score + 15;
	}
	if(motion_energy >= 1)
	{
		score = score + 25;
	}
	if(static_energy >= 1)
	{
		score = score + 50;
	}
	if(score < 30) 		return 0;
	else if(score <60) 	return 1;
	else				return 2;
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
	/* TODO: 用 static 变量保存 state + counter */
	static uint8_t state = STATE_DISARMED;
	static uint8_t counter = 0;

	switch(state)
	{
	case STATE_DISARMED:
		if(fire_level>=1)
		{
			state   = STATE_WARNING;
			counter = 1;
			return 1;
		}
		return 0;
		break;
	case STATE_ARMED:
		if(fire_level>=1 || intrusion_level>=1)
		{
			state = STATE_WARNING;
			counter = 1;
			return 1;
		}else return 0;
		break;
	case STATE_WARNING:
		if(fire_level>=1 || intrusion_level>=1)
		{
			counter++;
			if(counter>=3)
			{
				state = STATE_ALARM;
				return 2;
			}else return 1;
		}else{
			counter--;
			if(counter<=0)
			{
				state = STATE_ARMED;
			}
			return 0;
		}
		break;
	case STATE_ALARM:
		return 2;
		break;
	}
	return 0;
}

/**********************************************************************
 * 函数名称： dispatch_alarm
 * 功能描述： 根据最终报警等级构建报警命令结构体,
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
static void dispatch_alarm(uint8_t level)
{
	/* TODO */

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
		/* TODO: 队列接收 → check_fire → check_intrusion → 状态机 → dispatch */
		xQueueReceive(g_sensorQueue,&Sensor_Data,portMAX_DELAY);
		fire_level = check_fire(Sensor_Data.fire.smoke_ppm,
				Sensor_Data.fire.co_ppm,
				Sensor_Data.fire.fire_int,
				Sensor_Data.fire.temp);
		intrusion_level = check_intrusion(Sensor_Data.intrusion.pir_triggered,
										Sensor_Data.intrusion.has_person,
										Sensor_Data.intrusion.motion_dist,
										Sensor_Data.intrusion.motion_energy,
										Sensor_Data.intrusion.static_energy);
		state_result = run_state_machine(fire_level,intrusion_level);
		dispatch_alarm(state_result);
		vTaskDelay(pdMS_TO_TICKS(100));
	}
}
