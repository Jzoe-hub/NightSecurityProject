/**********************************************************************
 * 文件名称： task_config.c
 * 功能描述： 全局任务句柄、队列、信号量定义（全项目唯一实例）
 ***********************************************************************/
#include "task_config.h"

/* ===== 任务句柄定义 ===== */
TaskHandle_t g_SensorTaskHandle  = NULL;
TaskHandle_t g_SecurityTaskHandle = NULL;
TaskHandle_t g_AlarmTaskHandle   = NULL;
TaskHandle_t g_UITaskHandle      = NULL;
TaskHandle_t g_FingerTaskHandle  = NULL;
TaskHandle_t g_WatchdogTaskHandle = NULL;

/* ===== 队列/信号量句柄定义 ===== */
QueueHandle_t g_sensorQueue   = NULL;
QueueHandle_t g_securityQueue = NULL;
