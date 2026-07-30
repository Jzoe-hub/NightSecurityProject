/**********************************************************************
 * 文件名称： task_config.h
 * 功能描述： freertos任务配置头文件，含各任务函数声明/变量声明/
 ***********************************************************************/
#ifndef __TASK_CONFIG_H
#define __TASK_CONFIG_H

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

/*====================共用结构体和全局变量=========================*/

//传感器变量
typedef struct{
		float smoke_ppm;
		float co_ppm;
		uint16_t fire_int;
		uint8_t fire_do;
		int temp;
		int hum;
	}SensorFireData;
typedef struct{
		uint8_t pir_triggered;
		uint8_t has_person;
		uint8_t motion_energy;
		uint8_t static_energy;
		uint16_t motion_dist;
		uint16_t static_dist;
	}SensorIntrusionData;
typedef struct {
	SensorFireData      fire;
	SensorIntrusionData intrusion;
} SensorPacket; //* 打包传感器数据, 队列一次传一整包

//融合后报警状态结构体
typedef struct{
	int type;
	int level;
}AlarmCMD;

extern SensorPacket Sensor_Data;
extern AlarmCMD Security_Data;
extern uint8_t state_result;

/*==================任务函数和任务句柄======================*/
void SensorTask(void *pvParameters);
void SecurityTask(void *pvParameters);
void AlarmTask(void *pvParameters);
void UITask(void *pvParameters);
void FingerTask(void *pvParameters);
void WatchdogTask(void *pvParameters);

extern TaskHandle_t g_SensorTaskHandle;
extern TaskHandle_t g_SecurityTaskHandle;
extern TaskHandle_t g_AlarmTaskHandle;
extern TaskHandle_t g_UITaskHandle;
extern TaskHandle_t g_FingerTaskHandle;
extern TaskHandle_t g_WatchdogTaskHandle;

/*==================队列句柄======================*/

/* 队列句柄 */
extern QueueHandle_t g_sensorQueue;
extern QueueHandle_t g_securityQueue;


/*=====================看门狗心跳=======================*/
extern volatile uint32_t g_heartbeat_sensor;
extern volatile uint32_t g_heartbeat_security;
extern volatile uint32_t g_heartbeat_alarm;
extern volatile uint32_t g_heartbeat_ui;
extern volatile uint32_t g_heartbeat_finger;

#endif
