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

class SettingsViewModel : ViewModel() {

    val deviceInfo  = MutableStateFlow(DeviceInfo())
    val mqttStatus  = MutableStateFlow("已连接")

    /* ---- 阈值 (String 适配 TextField, ViewModel 持有跨页面保持) ---- */
    val thFire  = MutableStateFlow("2000")
    val thSmoke = MutableStateFlow("10")
    val thCo    = MutableStateFlow("10")
    val thTemp  = MutableStateFlow("45")
    val thPir   = MutableStateFlow("1")
}
