package com.example.security.ui.screen.settings

import androidx.lifecycle.ViewModel
import kotlinx.coroutines.flow.MutableStateFlow
import kotlinx.coroutines.flow.StateFlow

data class DeviceInfo(
    val deviceId:   String = "device_001",
    val firmware:   String = "v1.0.0",
    val broker:     String = "c0dde8f6.ala.cn-hangzhou.emqxsl.cn",
    val wifiSsid:   String = "(•‿•)",
    val wifiRssi:   Int = -45
)

data class Thresholds(
    val fire:  Int = 2000,
    val smoke: Int = 10,
    val co:    Int = 10,
    val temp:  Int = 45,
    val pir:   Int = 1
)

class SettingsViewModel : ViewModel() {

    val deviceInfo  = MutableStateFlow(DeviceInfo())
    val thresholds  = MutableStateFlow(Thresholds())
    val mqttStatus  = MutableStateFlow("已连接")
}
