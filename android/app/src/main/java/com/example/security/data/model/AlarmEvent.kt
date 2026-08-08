package com.example.security.data.model

/**
 * 报警事件 — 对应 MQTT women_safe/+/log
 */
data class AlarmEvent(
    val id:        Long,
    val type:      String,   // "fire" | "intrusion"
    val level:     String,   // "WARNING" | "ALARM"
    val timestamp: String,   // "2026-08-08 14:30:00"
    val detail:    String    // "烟雾浓度超标: 15ppm"
)
