package com.example.security.ui.screen.settings

import androidx.compose.foundation.layout.*
import androidx.compose.foundation.rememberScrollState
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.foundation.text.KeyboardOptions
import androidx.compose.foundation.verticalScroll
import androidx.compose.material3.*
import androidx.compose.runtime.*
import androidx.compose.ui.Modifier
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.text.input.KeyboardType
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp
import androidx.lifecycle.viewmodel.compose.viewModel

@OptIn(ExperimentalMaterial3Api::class)
@Composable
fun SettingsScreen(
    viewModel: SettingsViewModel = viewModel(),
    onSendThresholds: ((String, String, String, String, String) -> Unit)? = null
) {
    val device by viewModel.deviceInfo.collectAsState()
    val status by viewModel.mqttStatus.collectAsState()

    /* ---- 可编辑阈值 (ViewModel 持有, 跨页面保持) ---- */
    val thFire  by viewModel.thFire.collectAsState()
    val thSmoke by viewModel.thSmoke.collectAsState()
    val thCo    by viewModel.thCo.collectAsState()
    val thTemp  by viewModel.thTemp.collectAsState()
    val thPir   by viewModel.thPir.collectAsState()

    Scaffold(
        topBar = {
            TopAppBar(
                title = { Text("设置") },
                colors = TopAppBarDefaults.topAppBarColors(
                    containerColor = MaterialTheme.colorScheme.surface
                )
            )
        }
    ) { padding ->
        Column(
            modifier = Modifier
                .fillMaxSize()
                .padding(padding)
                .verticalScroll(rememberScrollState())
                .padding(16.dp),
            verticalArrangement = Arrangement.spacedBy(16.dp)
        ) {
            /* ---- 阈值配置 (可编辑) ---- */
            SectionTitle("传感器阈值")
            Card(
                modifier = Modifier.fillMaxWidth(),
                shape = RoundedCornerShape(12.dp)
            ) {
                Column(
                    Modifier.padding(16.dp),
                    verticalArrangement = Arrangement.spacedBy(12.dp)
                ) {
                    ThresholdInput("🔥 火焰强度", thFire,  { viewModel.thFire.value  = it })
                    ThresholdInput("💨 烟雾 ppm", thSmoke, { viewModel.thSmoke.value = it })
                    ThresholdInput("☠ CO ppm",   thCo,    { viewModel.thCo.value    = it })
                    ThresholdInput("🌡 温度 °C",  thTemp,  { viewModel.thTemp.value  = it })
                    ThresholdInput("👤 PIR",      thPir,   { viewModel.thPir.value   = it })

                    Spacer(Modifier.height(8.dp))

                    /* ---- 下发按钮 ---- */
                    Button(
                        onClick = { onSendThresholds?.invoke(thFire, thSmoke, thCo, thTemp, thPir) },
                        modifier = Modifier.fillMaxWidth().height(48.dp),
                        shape = RoundedCornerShape(12.dp),
                        colors = ButtonDefaults.buttonColors(
                            containerColor = MaterialTheme.colorScheme.primary
                        )
                    ) {
                        Text("保存并下发到设备", fontSize = 16.sp, fontWeight = FontWeight.Bold)
                    }
                }
            }

            /* ---- 连接状态 ---- */
            SectionTitle("连接状态")
            InfoRow("MQTT Broker", status)
            InfoRow("设备 ID", device.deviceId)
            InfoRow("固件版本", device.firmware)
            InfoRow("WiFi", "${device.wifiSsid}  ${device.wifiRssi}dBm")

            /* ---- 关于 ---- */
            SectionTitle("关于")
            InfoRow("App 版本", "v1.0.0-alpha")
        }
    }
}

@Composable
private fun SectionTitle(title: String) {
    Text(
        text = title,
        fontSize = 13.sp,
        fontWeight = FontWeight.Bold,
        color = MaterialTheme.colorScheme.primary,
        modifier = Modifier.padding(top = 8.dp)
    )
}

@Composable
private fun InfoRow(label: String, value: String) {
    Card(
        modifier = Modifier.fillMaxWidth(),
        shape = RoundedCornerShape(8.dp),
        colors = CardDefaults.cardColors(
            containerColor = MaterialTheme.colorScheme.surfaceVariant.copy(alpha = 0.5f)
        )
    ) {
        Row(
            Modifier.fillMaxWidth().padding(12.dp),
            horizontalArrangement = Arrangement.SpaceBetween
        ) {
            Text(label, fontSize = 14.sp, color = MaterialTheme.colorScheme.onSurfaceVariant)
            Text(value, fontSize = 14.sp, fontWeight = FontWeight.Medium)
        }
    }
}

@Composable
private fun ThresholdInput(
    label: String,
    value: String,
    onValueChange: (String) -> Unit
) {
    Row(
        Modifier.fillMaxWidth(),
        horizontalArrangement = Arrangement.SpaceBetween,
        verticalAlignment = androidx.compose.ui.Alignment.CenterVertically
    ) {
        Text(label, fontSize = 14.sp, modifier = Modifier.weight(1f))
        OutlinedTextField(
            value = value,
            onValueChange = onValueChange,
            modifier = Modifier.width(100.dp),
            singleLine = true,
            keyboardOptions = KeyboardOptions(keyboardType = KeyboardType.Number),
            shape = RoundedCornerShape(8.dp)
        )
    }
}
