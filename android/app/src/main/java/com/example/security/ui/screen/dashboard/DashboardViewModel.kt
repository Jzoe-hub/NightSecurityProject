package com.example.security.ui.screen.dashboard

import android.app.Application
import androidx.lifecycle.AndroidViewModel
import androidx.lifecycle.viewModelScope
import com.example.security.data.model.SensorState
import com.example.security.data.repository.MqttRepository
import kotlinx.coroutines.flow.MutableStateFlow
import kotlinx.coroutines.flow.StateFlow
import kotlinx.coroutines.launch

class DashboardViewModel(application: Application) : AndroidViewModel(application) {

    private val repository = MqttRepository.getInstance()

    val state: StateFlow<SensorState> = repository.sensorState
    val rawLog: StateFlow<List<String>> = repository.rawLog

    private val _isLoading = MutableStateFlow(true)
    val isLoading: StateFlow<Boolean> = _isLoading

    init {
        /* 监听 MQTT 连接状态, 在线后取消 loading */
        viewModelScope.launch {
            repository.sensorState.collect { s ->
                if (s.online) _isLoading.value = false
            }
        }
    }

    fun arm() {
        repository.sendCommand("""{"action":"arm","params":{"armed":1}}""")
    }

    fun disarm() {
        repository.sendCommand("""{"action":"disarm","params":{"armed":0}}""")
    }
}
