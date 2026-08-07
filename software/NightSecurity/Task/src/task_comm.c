/**********************************************************************
 * 文件名称： task_comm.c
 * 功能描述： CommTask — MQTT 云端通信（合并 EspRxTask 逻辑省 RAM）
 *           每 20ms 轮询 Esp8266_GetFrame(), 每 1s 上报 JSON,
 *           下行命令直接转发给 SecurityTask
 * 任务参数： CommTask — Priority 3, Stack 384, 1s 周期(内含20ms轮询)
 * 说    明： 合并后省掉 EspRxTask 栈(1KB) + g_cloudRxQueue(~900B)
 ***********************************************************************/
#include "task_config.h"
#include "esp8266.h"
#include <stdio.h>
#include <string.h>

extern SensorPacket Sensor_Data;
extern AlarmCMD    Security_Data;
extern uint8_t     state_result;
extern uint8_t     g_sw_armed;

/* ==================== CommTask ==================== */

void CommTask(void *pvParameters)
{
    (void)pvParameters;
    static uint32_t     tick = 0;        /* 1s 计数器        */
    static char         json[192];       /* JSON 缓冲区       */
    static EspFrame     frame;           /* 接收帧 (省栈)     */
    static CloudRxPacket rx;             /* 下行命令 (省栈)   */

    for (;;)
    {
        /* ---- 每 20ms 轮询 ESP8266 接收帧 (共 50 次=1s) ---- */
        for (int i = 0; i < 50; i++)
        {
            g_heartbeat_comm++;              /* 每 20ms 喂一次心跳 */
            if (Esp8266_GetFrame(&frame))
            {
                if (frame.type == ESP_TYPE_CMD ||
                    frame.type == ESP_TYPE_CONFIG)
                {
                    rx.type = frame.type;
                    rx.len  = frame.len;
                    if (frame.len > 0)
                        memcpy(rx.json, frame.payload, frame.len);
                    rx.json[frame.len] = '\0';
                    xQueueSend(g_cmdQueue, &rx, 0);
                }
            }
            vTaskDelay(pdMS_TO_TICKS(20));
        }

        tick++;

        /* ===== 传感器状态上报 (每 1s) ===== */
        snprintf(json, sizeof(json),
            "{\"type\":\"state\","
            "\"temp\":%d,\"humi\":%d,"
            "\"smoke\":%d,\"co\":%d,"
            "\"pir\":%d,\"fire\":%d,"
            "\"armed\":%d,\"alarm\":%d}",
            Sensor_Data.fire.temp,
            Sensor_Data.fire.hum,
            (int)Sensor_Data.fire.smoke_ppm,
            (int)Sensor_Data.fire.co_ppm,
            Sensor_Data.intrusion.pir_triggered,
            (int)Sensor_Data.fire.fire_int,
            g_sw_armed,
            state_result);
        Esp8266_SendFrame(ESP_TYPE_STATE,
                          (uint8_t*)json, strlen(json));

        /* ===== 心跳 (每 30s) ===== */
        if (tick % 30 == 0)
        {
            snprintf(json, sizeof(json),
                "{\"type\":\"heartbeat\",\"uptime\":%lu}",
                tick);
            Esp8266_SendFrame(ESP_TYPE_HEARTBEAT,
                              (uint8_t*)json, strlen(json));
        }
    }
}
