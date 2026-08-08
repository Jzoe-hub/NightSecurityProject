package com.example.security.ui.navigation

import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.padding
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.filled.Home
import androidx.compose.material.icons.filled.Phone
import androidx.compose.material.icons.filled.Settings
import androidx.compose.material3.*
import androidx.compose.runtime.*
import androidx.compose.ui.Modifier
import androidx.compose.ui.graphics.vector.ImageVector
import com.example.security.data.repository.MqttRepository
import com.example.security.ui.screen.dashboard.DashboardScreen
import com.example.security.ui.screen.emergency.EmergencyCallScreen
import com.example.security.ui.screen.settings.SettingsScreen

private data class NavItem(
    val label: String,
    val icon: ImageVector
)

@Composable
fun AppNavigation() {
    val navItems = listOf(
        NavItem("仪表盘",   Icons.Filled.Home),
        NavItem("紧急求助", Icons.Filled.Phone),
        NavItem("设置",     Icons.Filled.Settings)
    )

    var selectedIndex by remember { mutableIntStateOf(0) }

    Scaffold(
        modifier = Modifier.fillMaxSize(),
        bottomBar = {
            NavigationBar {
                navItems.forEachIndexed { index, item ->
                    NavigationBarItem(
                        icon = {
                            Icon(
                                imageVector = item.icon,
                                contentDescription = item.label
                            )
                        },
                        label = { Text(item.label) },
                        selected = selectedIndex == index,
                        onClick = { selectedIndex = index }
                    )
                }
            }
        }
    ) { innerPadding ->
        Box(modifier = Modifier.fillMaxSize().padding(innerPadding)) {
            when (selectedIndex) {
                0 -> DashboardScreen()
                1 -> EmergencyCallScreen()
                2 -> {
                    val repo = remember { MqttRepository.getInstance() }
                    SettingsScreen(
                        onSendThresholds = { fire, smoke, co, temp, pir ->
                            repo.sendThresholds(fire, smoke, co, temp, pir)
                        }
                    )
                }
            }
        }
    }
}
