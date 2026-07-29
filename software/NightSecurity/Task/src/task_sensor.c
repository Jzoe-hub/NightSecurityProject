/**********************************************************************
 * 文件名称： task_sensor.c
 * 功能描述： SensorTask — 传感器数据采集任务 (100ms / Priority 3 / Stack 256)
 ***********************************************************************/
#include "task_config.h"
#include "main.h"
#include "mq2.h"
#include "mq7.h"
#include "fire.h"
#include "pir.h"
#include "dht11.h"
#include "radar.h"

/* ---- DMA 缓冲区（main.c 定义） ---- */
extern uint16_t g_adc_raw[3];
/*--------------------全局变量定义--------------------*/
float smoke_ppm,co_ppm;
uint16_t fire_int;
uint8_t pir_triggered,fire_do;
int temp,hum;
uint8_t has_person,motion_energy,static_energy;
uint16_t motion_dist,static_dist;


/* ==================== 传感器读取子函数 ==================== */

/**********************************************************************
 * 函数名称： read_adc_sensors
 * 功能描述： 从 ADC DMA 缓冲区读取 MQ-2/MQ-7/火焰三个 ADC 传感器,
 *           调用各自 Lib 的换算函数得到物理量。
 *           执行流程：
 *           1. g_adc_raw[0] → MQ2_RawToPPM() → smoke_ppm
 *           2. g_adc_raw[1] → MQ7_RawToPPM() → co_ppm
 *           3. g_adc_raw[2] → Fire_GetIntensity() → fire_int
 * 输入参数： smoke_ppm — 用于接收 MQ-2 烟雾浓度 (指针)
 *           co_ppm   — 用于接收 MQ-7 CO 浓度   (指针)
 *           fire_int — 用于接收火焰强度 0~4095  (指针)
 * 返 回 值： 无
 ***********************************************************************/
static void read_adc_sensors(float *smoke_ppm, float *co_ppm, uint16_t *fire_int)
{
	/* TODO */
	*smoke_ppm = MQ2_RawToPPM(g_adc_raw[MQ2_ADC_INDEX]);
	*co_ppm = MQ7_RawToPPM(g_adc_raw[MQ7_ADC_INDEX]);
	*fire_int = Fire_GetIntensity(g_adc_raw[FIRE_ADC_INDEX]);
}

/**********************************************************************
 * 函数名称： read_binary_sensors
 * 功能描述： 读取两个开关量传感器——PIR 人体红外和火焰 DO 数字口,
 *           不涉及任何换算, 直接调 Lib 函数读 GPIO 电平。
 *           执行流程：
 *           1. PIR_Read() → 1=有人 0=无人
 *           2. Fire_IsDetected() → 0=检测到火 1=无火
 * 输入参数： pir_triggered — 用于接收有人/无人状态 (指针)
 *           fire_do      — 用于接收火焰 DO 状态 (指针)
 * 返 回 值： 无
 ***********************************************************************/
static void read_binary_sensors(uint8_t *pir_triggered, uint8_t *fire_do)
{
	/* TODO */
	*pir_triggered = PIR_Read();
	*fire_do  = Fire_IsDetected();
}

/**********************************************************************
 * 函数名称： read_dht11_data
 * 功能描述： DHT11 手册要求读取间隔至少 2 秒, 此函数内部用静态计数器
 *           累加调用次数, 每 20 次调用 (20×100ms=2s) 才真正读一次。
 *           执行流程：
 *           1. static uint8_t tick = 0; tick++;
 *           2. if (tick >= 20) { DHT11_Read(&t, &h); tick = 0; }
 *           3. 其余调用直接返回上次的缓存值
 * 输入参数： temp — 用于接收温度 ℃ (指针)
 *           hum  — 用于接收湿度 %RH (指针)
 * 返 回 值： 无
 ***********************************************************************/
static void read_dht11_data(int *temp, int *hum)
{
	/* TODO */
	static uint16_t tick=0;
	static int last_temp = 0;
	static int last_hum = 0;
	tick++;
	if(tick>=20)
	{
		DHT11_Read(hum,temp);
		last_temp = *temp; //更新缓存
		last_hum = *hum;
		tick = 0;
	}else{
		*temp = last_temp; //读取缓存值
		*hum = last_hum;
	}
}

/**********************************************************************
 * 函数名称： read_radar_data
 * 功能描述： LD2410C 数据由 UART DMA 回调自动解析到全局结构体
 *           Detection_Target_LD2410C（定义在 radar.h）。
 *           本函数只是把它的字段拷出来, 不做任何解析。
 *           执行流程：
 *           1. 读 Detection_Target_LD2410C.STATE_target
 *           2. 读 MOTION/STATIC 的 distance 和 energy
 * 输入参数： has_person    — 用于接收有人/无人 (指针)
 *           motion_dist   — 用于接收运动目标距离 cm (指针)
 *           motion_energy — 用于接收运动能量 (指针)
 *           static_dist   — 用于接收静止目标距离 cm (指针)
 *           static_energy — 用于接收静止能量 (指针)
 * 返 回 值： 无
 ***********************************************************************/
static void read_radar_data(uint8_t *has_person, uint16_t *motion_dist,
		uint8_t *motion_energy, uint16_t *static_dist, uint8_t *static_energy)
{
	/* TODO */
	*has_person = Detection_Target_LD2410C.STATE_target;
	*motion_dist = Detection_Target_LD2410C.MOTION_target_distance;
	*motion_energy = Detection_Target_LD2410C.MOTION_target_energy;
	*static_dist = Detection_Target_LD2410C.STATIC_target_distance;
	*static_energy = Detection_Target_LD2410C.STATIC_target_energy;
}

/**********************************************************************
 * 函数名称： publish_sensor_data
 * 功能描述： 把本轮采集的所有传感器数据打包成一个结构体,
 *           通过队列同步给 SecurityTask。
 *           执行流程：
 *           1. 定义一个 SensorData 结构体变量, 填入各字段
 *           2. xQueueSend(g_sensorQueue, &packet, 0)
 *           注意：队列句柄 g_sensorQueue 需要在 task_config.h 中声明
 * 输入参数： （所有传感器本轮最新值, 作为参数列表传入）
 * 返 回 值： 无
 ***********************************************************************/
static void publish_sensor_data(float smoke, float co, uint16_t fire_int,
		uint8_t pir, uint8_t fire_do, int temp, int hum,
		uint8_t has_person)
{
	/* TODO */
	SensorFireData Sensor_fire;
		Sensor_fire.smoke_ppm		   = smoke;
		Sensor_fire.co_ppm			   = co;
		Sensor_fire.fire_int		   = fire_int;
		Sensor_fire.fire_do			   =fire_do;
		Sensor_fire.temp			   =temp;
		Sensor_fire.hum 			   = hum;

	SensorIntrusionData Sensor_intrusion;
		Sensor_intrusion.pir_triggered  = pir;
		Sensor_intrusion.has_person     = has_person;
		Sensor_intrusion.motion_dist    = motion_dist;
		Sensor_intrusion.motion_energy  = motion_energy;
		Sensor_intrusion.static_dist    = static_dist;
		Sensor_intrusion.static_energy  = static_energy;

	SensorPacket Sensor_Data;
	Sensor_Data.fire = Sensor_fire;
	Sensor_Data.intrusion = Sensor_intrusion;
	xQueueSend(g_sensorQueue,&Sensor_Data,0);
}

/* ==================== 任务函数 ==================== */

/**********************************************************************
 * 函数名称： SensorTask
 * 功能描述： 每 100ms 执行一轮完整的传感器采集流程。执行顺序：
 *           1. read_gas_sensors()      — 3 个 ADC 传感器
 *           2. read_binary_sensors()   — 2 个开关量
 *           3. read_dht11_if_due()     — 温湿度 (内部自动 2s 一次)
 *           4. read_radar_data()       — 雷达目标数据
 *           5. publish_sensor_data()   — 打包发给 SecurityTask
 * 输入参数： pvParameters — 未使用
 * 返 回 值： 无
 ***********************************************************************/
void SensorTask(void *pvParameters)
{
	(void)pvParameters;

	for (;;)
	{
		read_adc_sensors(&smoke_ppm, &co_ppm, &fire_int);
		read_binary_sensors(&pir_triggered, &fire_do);
		read_dht11_data(&temp, &hum);
		read_radar_data(&has_person, &motion_dist, &motion_energy, &static_dist, &static_energy);
		publish_sensor_data(smoke_ppm, co_ppm, fire_int,pir_triggered, fire_do, temp, hum, has_person);
		vTaskDelay(pdMS_TO_TICKS(100));
	}
}
