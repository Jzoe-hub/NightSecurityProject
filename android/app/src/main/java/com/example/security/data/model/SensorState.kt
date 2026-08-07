package com.example.security.data.model

/**
 * 传感器实时状态 — 对应 MQTT women_safe/+/state JSON
 */
data class SensorState(
    val temperature: Int    = 0,    // 温度 ℃
    val humidity:    Int    = 0,    // 湿度 %RH
    val smoke:       Int    = 0,    // 烟雾浓度 ppm
    val co:          Int    = 0,    // CO 浓度 ppm
    val pir:         Boolean = false, // PIR 有人/无人
    val fireIntensity: Int  = 0,    // 火焰强度 0~4095
    val armed:       Boolean = false, // 布防/撤防
    val alarmLevel:  Int    = 0,    // 0=正常 1=预警 2=报警
    val online:      Boolean = true  // 设备在线
)
