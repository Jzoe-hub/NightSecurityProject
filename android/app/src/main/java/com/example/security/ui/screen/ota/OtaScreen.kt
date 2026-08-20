package com.example.security.ui.screen.ota

import android.net.Uri
import androidx.activity.compose.rememberLauncherForActivityResult
import androidx.activity.result.contract.ActivityResultContracts
import androidx.compose.foundation.layout.*
import androidx.compose.foundation.rememberScrollState
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.foundation.verticalScroll
import androidx.compose.material3.*
import androidx.compose.runtime.*
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.platform.LocalContext
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp
import com.example.security.data.repository.MqttRepository
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.launch
import kotlinx.coroutines.withContext

/**
 * OTA 固件升级界面
 * 流程：选择固件 .bin → 点「开始升级」→ 仓库走完整 OTA 流程（发指令/发固件块/发结束）
 * 进度和状态通过 MqttRepository 的 otaProgress / otaStatus 两个 StateFlow 实时驱动
 */
@OptIn(ExperimentalMaterial3Api::class)
@Composable
fun OtaScreen(onBack: () -> Unit) {
    val context = LocalContext.current
    val repo = remember { MqttRepository.getInstance() }
    val scope = rememberCoroutineScope()

    val progress by repo.otaProgress.collectAsState()
    val status by repo.otaStatus.collectAsState()

    var fileName by remember { mutableStateOf("未选择固件") }
    var firmware by remember { mutableStateOf<ByteArray?>(null) }
    var upgrading by remember { mutableStateOf(false) }

    /* 系统文件选择器：选 .bin 固件 */
    val pickFile = rememberLauncherForActivityResult(
        ActivityResultContracts.OpenDocument()
    ) { uri: Uri? ->
        uri ?: return@rememberLauncherForActivityResult
        scope.launch {
            val bytes = withContext(Dispatchers.IO) {
                try {
                    context.contentResolver.openInputStream(uri)?.use { it.readBytes() }
                } catch (e: Exception) {
                    null
                }
            }
            if (bytes != null && bytes.isNotEmpty()) {
                firmware = bytes
                fileName = uri.lastPathSegment ?: "firmware.bin"
            }
        }
    }

    Scaffold(
        topBar = {
            TopAppBar(
                title = { Text("OTA 固件升级") },
                navigationIcon = {
                    TextButton(onClick = onBack) { Text("返回") }
                },
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
            /* ---- 升级说明 ---- */
            Card(
                modifier = Modifier.fillMaxWidth(),
                shape = RoundedCornerShape(12.dp),
                colors = CardDefaults.cardColors(
                    containerColor = MaterialTheme.colorScheme.primaryContainer.copy(alpha = 0.4f)
                )
            ) {
                Column(Modifier.padding(16.dp), verticalArrangement = Arrangement.spacedBy(6.dp)) {
                    Text("升级流程", fontSize = 13.sp, fontWeight = FontWeight.Bold)
                    Text(
                        "① 发升级指令 → ② 设备重启进 Bootloader → " +
                        "③ 分块下发固件 → ④ 设备校验并跳转新固件。\n" +
                        "升级过程中请勿离开本页面。",
                        fontSize = 12.sp,
                        color = MaterialTheme.colorScheme.onSurfaceVariant
                    )
                }
            }

            /* ---- 选择固件 ---- */
            Card(
                modifier = Modifier.fillMaxWidth(),
                shape = RoundedCornerShape(12.dp)
            ) {
                Column(Modifier.padding(16.dp), verticalArrangement = Arrangement.spacedBy(12.dp)) {
                    Text("固件文件", fontSize = 13.sp, fontWeight = FontWeight.Bold,
                        color = MaterialTheme.colorScheme.primary)

                    OutlinedButton(
                        onClick = { pickFile.launch(arrayOf("*/*")) },
                        modifier = Modifier.fillMaxWidth().height(48.dp),
                        shape = RoundedCornerShape(12.dp),
                        enabled = !upgrading
                    ) {
                        Text("选择固件 (.bin)")
                    }

                    Text(
                        "已选：$fileName（${firmware?.size ?: 0} 字节）",
                        fontSize = 13.sp,
                        color = MaterialTheme.colorScheme.onSurfaceVariant
                    )
                }
            }

            /* ---- 升级按钮 ---- */
            Button(
                onClick = {
                    val fw = firmware ?: return@Button
                    upgrading = true
                    scope.launch {
                        repo.startOta(fw)
                        upgrading = false
                    }
                },
                modifier = Modifier.fillMaxWidth().height(52.dp),
                shape = RoundedCornerShape(12.dp),
                enabled = firmware != null && !upgrading,
                colors = ButtonDefaults.buttonColors(
                    containerColor = MaterialTheme.colorScheme.primary
                )
            ) {
                Text(
                    if (upgrading) "升级中…" else "开始升级",
                    fontSize = 16.sp,
                    fontWeight = FontWeight.Bold
                )
            }

            /* ---- 进度与状态 ---- */
            Card(
                modifier = Modifier.fillMaxWidth(),
                shape = RoundedCornerShape(12.dp)
            ) {
                Column(Modifier.padding(16.dp), verticalArrangement = Arrangement.spacedBy(12.dp)) {
                    Text("升级进度", fontSize = 13.sp, fontWeight = FontWeight.Bold,
                        color = MaterialTheme.colorScheme.primary)

                    LinearProgressIndicator(
                        progress = { progress },
                        modifier = Modifier.fillMaxWidth().height(8.dp),
                        color = MaterialTheme.colorScheme.primary
                    )

                    Text(
                        "${(progress * 100).toInt()}%",
                        fontSize = 14.sp,
                        fontWeight = FontWeight.Medium
                    )

                    Text(
                        status,
                        fontSize = 13.sp,
                        color = MaterialTheme.colorScheme.onSurfaceVariant
                    )
                }
            }
        }
    }
}
