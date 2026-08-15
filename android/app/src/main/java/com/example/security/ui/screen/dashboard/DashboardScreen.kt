package com.example.security.ui.screen.dashboard

import androidx.compose.foundation.background
import androidx.compose.foundation.layout.*
import androidx.compose.foundation.lazy.LazyColumn
import androidx.compose.foundation.lazy.items
import androidx.compose.foundation.rememberScrollState
import androidx.compose.foundation.shape.CircleShape
import androidx.compose.foundation.verticalScroll
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.material3.*
import androidx.compose.runtime.Composable
import androidx.compose.runtime.collectAsState
import androidx.compose.runtime.getValue
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.draw.clip
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp
import androidx.lifecycle.viewmodel.compose.viewModel
import com.example.security.data.model.SensorState

/* ==================== 仪表盘主界面 ==================== */

@Composable
fun DashboardScreen(
    viewModel: DashboardViewModel = viewModel()
) {
    val state by viewModel.state.collectAsState()
    val rawLog by viewModel.rawLog.collectAsState()
    val loading by viewModel.isLoading.collectAsState()

    if (loading) {
        Box(Modifier.fillMaxSize(), contentAlignment = Alignment.Center) {
            CircularProgressIndicator()
        }
        return
    }

    Scaffold(
        topBar = { DashboardTopBar(state) },
        bottomBar = { ArmBottomBar(state.armed, viewModel::arm, viewModel::disarm) }
    ) { padding ->
        Column(
            modifier = Modifier
                .fillMaxSize()
                .padding(padding)
                .padding(16.dp)
                .verticalScroll(rememberScrollState()),
            verticalArrangement = Arrangement.spacedBy(12.dp)
        ) {
            SensorGrid(state)

            /* ---- MQTT 原始数据 (滚动日志) ---- */
            Text("MQTT 原始数据", fontSize = 13.sp, fontWeight = FontWeight.Bold,
                color = MaterialTheme.colorScheme.primary)
            Surface(
                modifier = Modifier.fillMaxWidth().height(160.dp),
                shape = RoundedCornerShape(8.dp),
                color = Color(0xFF263238)
            ) {
                LazyColumn(
                    modifier = Modifier.padding(8.dp)
                ) {
                    items(rawLog.size) { i ->
                        Text(
                            text = rawLog[i],
                            fontSize = 10.sp,
                            color = Color(0xFF4CAF50),
                            lineHeight = 14.sp
                        )
                    }
                }
            }
        }
    }
}

/* ==================== 顶部状态栏 ==================== */

@Composable
private fun DashboardTopBar(state: SensorState) {
    Surface(
        modifier = Modifier.fillMaxWidth(),
        shadowElevation = 4.dp,
        color = MaterialTheme.colorScheme.surface
    ) {
        Row(
            modifier = Modifier
                .fillMaxWidth()
                .padding(horizontal = 16.dp, vertical = 12.dp),
            horizontalArrangement = Arrangement.SpaceBetween,
            verticalAlignment = Alignment.CenterVertically
        ) {
            Column {
                Text("女性安防系统", fontSize = 20.sp, fontWeight = FontWeight.Bold)
                Row(verticalAlignment = Alignment.CenterVertically) {
                    // 在线指示器 (绿点)
                    Box(
                        Modifier
                            .size(8.dp)
                            .clip(CircleShape)
                            .background(if (state.online) Color(0xFF4CAF50) else Color.Gray)
                    )
                    Spacer(Modifier.width(4.dp))
                    Text(
                        if (state.online) "在线" else "离线",
                        fontSize = 12.sp,
                        color = MaterialTheme.colorScheme.onSurfaceVariant
                    )
                }
            }
            // 布防/撤防标签
            Surface(
                shape = RoundedCornerShape(8.dp),
                color = if (state.armed) Color(0xFF4CAF50) else Color(0xFF757575)
            ) {
                Text(
                    text = if (state.armed) "ARMED" else "DISARMED",
                    modifier = Modifier.padding(horizontal = 12.dp, vertical = 4.dp),
                    color = Color.White,
                    fontWeight = FontWeight.Bold
                )
            }
        }
    }
}

/* ==================== 传感器网格 2×3 ==================== */

@Composable
private fun SensorGrid(state: SensorState) {
    Row(
        modifier = Modifier.fillMaxWidth(),
        horizontalArrangement = Arrangement.spacedBy(12.dp)
    ) {
        SensorCard("🌡 温度", "${state.temperature}°C", color = if (state.temperature > 45) Color.Red else Color(0xFFFF9800),
            modifier = Modifier.weight(1f))
        SensorCard("💧 湿度", "${state.humidity}%", color = Color(0xFF2196F3),
            modifier = Modifier.weight(1f))
    }
    Row(
        modifier = Modifier.fillMaxWidth(),
        horizontalArrangement = Arrangement.spacedBy(12.dp)
    ) {
        SensorCard("🔥 烟雾", "${state.smoke} ppm",
            color = if (state.smoke > 10) Color.Red else Color(0xFF795548),
            modifier = Modifier.weight(1f))
        SensorCard("☠ CO", "${state.co} ppm",
            color = if (state.co > 10) Color.Red else Color(0xFF607D8B),
            modifier = Modifier.weight(1f))
    }
    Row(
        modifier = Modifier.fillMaxWidth(),
        horizontalArrangement = Arrangement.spacedBy(12.dp)
    ) {
        SensorCard("👤 PIR",
            if (state.pir) "有人" else "无人",
            color = if (state.pir) Color(0xFFF44336) else Color(0xFF4CAF50),
            modifier = Modifier.weight(1f))
        SensorCard("🔆 火焰", "${state.fireIntensity}",
            color = if (state.fireIntensity > 2000) Color.Red else Color(0xFFFFC107),
            modifier = Modifier.weight(1f))
    }
}

/* ==================== 传感器卡片 ==================== */

@Composable
private fun SensorCard(
    label: String,
    value: String,
    color: Color,
    modifier: Modifier = Modifier
) {
    Card(
        modifier = modifier.height(100.dp),
        shape = RoundedCornerShape(12.dp),
        colors = CardDefaults.cardColors(containerColor = color.copy(alpha = 0.1f))
    ) {
        Column(
            modifier = Modifier
                .fillMaxSize()
                .padding(12.dp),
            verticalArrangement = Arrangement.SpaceBetween
        ) {
            Text(label, fontSize = 13.sp, color = MaterialTheme.colorScheme.onSurfaceVariant)
            Text(
                value,
                fontSize = 24.sp,
                fontWeight = FontWeight.Bold,
                color = color
            )
        }
    }
}

/* ==================== 底部布防/撤防按钮 ==================== */

@Composable
private fun ArmBottomBar(armed: Boolean, onArm: () -> Unit, onDisarm: () -> Unit) {
    Surface(
        modifier = Modifier.fillMaxWidth(),
        shadowElevation = 8.dp,
        color = MaterialTheme.colorScheme.surface
    ) {
        Row(
            modifier = Modifier
                .fillMaxWidth()
                .padding(16.dp),
            horizontalArrangement = Arrangement.spacedBy(12.dp)
        ) {
            /* 布防按钮 */
            Button(
                onClick = onArm,
                modifier = Modifier.weight(1f).height(48.dp),
                enabled = !armed,                       /* 已布防则禁用 */
                colors = ButtonDefaults.buttonColors(
                    containerColor = Color(0xFF4CAF50),
                    disabledContainerColor = Color(0xFF4CAF50).copy(alpha = 0.3f)
                ),
                shape = RoundedCornerShape(12.dp)
            ) {
                Text("布 防", fontSize = 18.sp, fontWeight = FontWeight.Bold, color = Color.White)
            }
            /* 撤防按钮 */
            Button(
                onClick = onDisarm,
                modifier = Modifier.weight(1f).height(48.dp),
                enabled = armed,                        /* 未布防则禁用 */
                colors = ButtonDefaults.buttonColors(
                    containerColor = Color(0xFFF44336),
                    disabledContainerColor = Color(0xFFF44336).copy(alpha = 0.3f)
                ),
                shape = RoundedCornerShape(12.dp)
            ) {
                Text("撤 防", fontSize = 18.sp, fontWeight = FontWeight.Bold, color = Color.White)
            }
        }
    }
}
