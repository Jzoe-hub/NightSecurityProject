package com.example.security.ui.screen.history

import androidx.lifecycle.ViewModel
import com.example.security.data.model.AlarmEvent
import kotlinx.coroutines.flow.MutableStateFlow
import kotlinx.coroutines.flow.StateFlow

class HistoryViewModel : ViewModel() {

    private val _events = MutableStateFlow(mockEvents())
    val events: StateFlow<List<AlarmEvent>> = _events

    companion object {
        private fun mockEvents() = listOf(
            AlarmEvent(1, "fire",      "ALARM",   "2026-08-08 02:15:30", "火焰强度超标: 2500"),
            AlarmEvent(2, "intrusion", "WARNING", "2026-08-07 22:10:05", "PIR 检测到移动"),
            AlarmEvent(3, "fire",      "WARNING", "2026-08-07 18:45:12", "温度升高: 48°C"),
            AlarmEvent(4, "intrusion", "ALARM",   "2026-08-07 03:22:00", "雷达静止目标 + PIR 触发"),
            AlarmEvent(5, "fire",      "ALARM",   "2026-08-06 14:08:33", "烟雾超标: 20ppm + 火焰"),
        )
    }
}
