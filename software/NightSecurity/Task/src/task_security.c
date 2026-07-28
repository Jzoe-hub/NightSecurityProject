/**********************************************************************
 * 文件名称： task_security.c
 * 功能描述： SecurityTask — 安全判定任务 (100ms / Priority 4 / Stack 512)
 *           传感器数据融合 + 火灾/入侵判定 + 状态机 + 报警调度
 ***********************************************************************/
#include "task_config.h"

/* ==================== 判定子函数 ==================== */

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
	/* TODO */
	return 0;
}

/**********************************************************************
 * 函数名称： check_intrusion
 * 功能描述： 综合 PIR + 雷达运动目标 + 雷达静止目标三个维度,
 *           加权评分判断是否有人入侵。
 *           评分逻辑：
 *           1. PIR 触发:   直接贡献 30 分
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
		uint16_t motion_dist, uint8_t has_motion, uint8_t has_static)
{
	/* TODO */
	return 0;
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
	/* TODO: 用 static 变量保存 current_state + confirm_counter */
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
	TickType_t xLastWakeTime = xTaskGetTickCount();
	(void)pvParameters;

	for (;;)
	{
		/* TODO: 队列接收 → check_fire → check_intrusion → 状态机 → dispatch */
		vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(100));
	}
}
