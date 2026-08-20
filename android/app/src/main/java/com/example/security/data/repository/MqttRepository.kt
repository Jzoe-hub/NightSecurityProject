package com.example.security.data.repository

import com.example.security.data.model.SensorState
import kotlinx.coroutines.delay
import kotlinx.coroutines.flow.MutableStateFlow
import kotlinx.coroutines.flow.StateFlow
import android.util.Log
import org.eclipse.paho.client.mqttv3.*
import org.eclipse.paho.client.mqttv3.persist.MemoryPersistence
import org.json.JSONObject
import java.security.SecureRandom
import java.text.SimpleDateFormat
import java.util.Date
import java.util.Locale
import java.security.cert.X509Certificate
import javax.net.ssl.SSLContext
import javax.net.ssl.TrustManager
import javax.net.ssl.X509TrustManager

/**
 * MQTT 数据仓库 — 连接 EMQX Broker (TLS), 订阅传感器 Topic, 解析 JSON
 */
class MqttRepository {

    companion object {
        @Volatile private var instance: MqttRepository? = null
        fun getInstance(): MqttRepository = instance ?: synchronized(this) {
            instance ?: MqttRepository().also { instance = it }
        }
    }

    private val brokerUrl = "ssl://c0dde8f6.ala.cn-hangzhou.emqxsl.cn:8883"
    private val clientId  = "android_device_001"

    /* ---- MQTT 客户端 ---- */
    private val client = MqttClient(brokerUrl, clientId, MemoryPersistence())

    /* ---- 数据流 ---- */
    private val _sensorState = MutableStateFlow(SensorState())
    val sensorState: StateFlow<SensorState> = _sensorState

    private val _rawLog = MutableStateFlow(listOf("等待数据..."))
    val rawLog: StateFlow<List<String>> = _rawLog

    /* 乐观更新布防状态 (点击立即翻转, 不等 MQTT 回传) */
    fun updateLocalArmed(armed: Boolean) {
        _sensorState.value = _sensorState.value.copy(armed = armed)
    }

    init {
        connectAndSubscribe()
    }

    /* ==================== 连接与订阅 ==================== */

    private fun connectAndSubscribe() {
        try {
            val options = MqttConnectOptions().apply {
                userName = "Jxy"
                password = "12345678".toCharArray()
                isCleanSession = true
                connectionTimeout = 15
                keepAliveInterval = 60
                socketFactory = allTrustingSslSocketFactory()  /* TLS 不验证证书 */
            }

            client.setCallback(object : MqttCallback {
                override fun connectionLost(cause: Throwable?) {
                    _sensorState.value = _sensorState.value.copy(online = false)
                    /* 自动重连 */
                    Thread {
                        while (!client.isConnected) {
                            try {
                                Thread.sleep(3000)
                                client.connect(options)
                                client.subscribe("women_safe/device_001/state", 1)
                                _sensorState.value = _sensorState.value.copy(online = true)
                                Log.d("MQTT", "Reconnected!")
                            } catch (_: Exception) {}
                        }
                    }.start()
                }

                override fun messageArrived(topic: String?, message: MqttMessage?) {
                    try {
                        val payload = message?.let { String(it.payload) } ?: ""
                        Log.d("MQTT", "MSG topic=$topic len=${payload.length}")
                        message?.let {
                            parseStateJson(String(it.payload))
                        }
                    } catch (e: Exception) {
                        Log.e("MQTT", "messageArrived crash", e)
                    }
                }

                override fun deliveryComplete(token: IMqttDeliveryToken?) {}
            })

            Log.d("MQTT", "Connecting to $brokerUrl...")
            client.connect(options)
            Log.d("MQTT", "Connected! Subscribing...")
            client.subscribe("women_safe/device_001/state", 1)
            Log.d("MQTT", "Subscribed OK")
            try {
                _sensorState.value = _sensorState.value.copy(online = true)
            } catch (e: Exception) {
                Log.e("MQTT", "StateFlow update crash", e)
            }

        } catch (e: Exception) {
            Log.e("MQTT", "Connection failed: ${e.message}", e)
            _sensorState.value = _sensorState.value.copy(online = false)
        }
    }

    /* ==================== TLS 信任所有证书 ==================== */

    private fun allTrustingSslSocketFactory(): javax.net.ssl.SSLSocketFactory {
        val trustAll = arrayOf<TrustManager>(object : X509TrustManager {
            override fun checkClientTrusted(c: Array<X509Certificate>?, a: String?) {}
            override fun checkServerTrusted(c: Array<X509Certificate>?, a: String?) {}
            override fun getAcceptedIssuers(): Array<X509Certificate> = arrayOf()
        })
        val ctx = SSLContext.getInstance("TLS")
        ctx.init(null, trustAll, SecureRandom())
        return ctx.socketFactory
    }

    /* ==================== JSON 解析 ==================== */

    private fun parseStateJson(json: String) {
        try {
            val ts = SimpleDateFormat("HH:mm:ss", Locale.getDefault()).format(Date())
            val line = "[$ts] $json"
            val log = _rawLog.value.toMutableList()
            log.add(line)
            if (log.size > 50) log.removeAt(0)   /* 最多保留 50 条 */
            _rawLog.value = log
            val obj = JSONObject(json)
            /* 过滤非 state 消息 (online/offline/heartbeat) */
            if (obj.optString("type", "") != "state") return

            _sensorState.value = SensorState(
                temperature    = obj.optInt("temp", 0),
                humidity       = obj.optInt("humi", 0),
                smoke          = obj.optInt("smoke", 0),
                co             = obj.optInt("co", 0),
                pir            = obj.optInt("pir", 0) == 1,
                fireIntensity  = obj.optInt("fire", 0),
                armed          = obj.optInt("armed", 0) == 1,
                alarmLevel     = obj.optInt("alarm", 0),
                online         = true
            )
            Log.d("MQTT", "Parsed: temp=${obj.optInt("temp")} armed=${obj.optInt("armed")}")
        } catch (e: Exception) {
            Log.e("MQTT", "Parse error: $json", e)
        }
    }

    /* ==================== 下发阈值 ==================== */

    fun sendThresholds(fire: String, smoke: String, co: String, temp: String, pir: String) {
        val json = """{"type":"config","threshold":{"fire":$fire,"smoke":$smoke,"co":$co,"temp":$temp,"pir":$pir}}"""
        publish("women_safe/device_001/config", json)
    }

    /* ==================== 下发命令 ==================== */

    fun sendCommand(json: String) {
        publish("women_safe/device_001/cmd", json)
    }

    private fun publish(topic: String, json: String) {
        try {
            if (!client.isConnected) return
            val msg = MqttMessage(json.toByteArray()).apply { qos = 1 }
            client.publish(topic, msg)
            Log.d("MQTT", "PUB topic=$topic payload=$json")
        } catch (e: Exception) {
            Log.e("MQTT", "Publish failed", e)
        }
    }

    /* ==================== OTA 固件升级 ==================== */

    /* OTA 进度状态（0f~1f）与状态文字 */
    private val _otaProgress = MutableStateFlow(0f)
    val otaProgress: StateFlow<Float> = _otaProgress
    private val _otaStatus = MutableStateFlow("未开始")
    val otaStatus: StateFlow<String> = _otaStatus

    /**
     * CRC16-MODBUS 校验（与 STM32 Bootloader / ESP8266 端算法一致）
     */
    fun crc16Modbus(data: ByteArray): Int {
        var crc = 0xFFFF
        for (b in data) {
            crc = crc xor (b.toInt() and 0xFF)
            for (j in 0 until 8) {
                crc = if ((crc and 0x0001) != 0) (crc ushr 1) xor 0xA001 else crc ushr 1
            }
        }
        return crc and 0xFFFF
    }

    /** 发升级指令：/cmd JSON "ota" → STM32 APP 写标志重启进 Bootloader */
    fun sendUpgradeCommand() {
        publish("women_safe/device_001/cmd", "{\"action\":\"ota\"}")
    }

    /** 发固件开始：/fw/start，payload = 总大小(4B 大端) + 总CRC(2B 大端) */
    private fun sendFwStart(totalSize: Int, totalCrc: Int) {
        val payload = ByteArray(6)
        payload[0] = ((totalSize shr 24) and 0xFF).toByte()
        payload[1] = ((totalSize shr 16) and 0xFF).toByte()
        payload[2] = ((totalSize shr 8)  and 0xFF).toByte()
        payload[3] = ( totalSize        and 0xFF).toByte()
        payload[4] = ((totalCrc shr 8)  and 0xFF).toByte()
        payload[5] = ( totalCrc         and 0xFF).toByte()
        publishBinary("women_safe/device_001/fw/start", payload)
    }

    /** 发固件数据块：/fw/data，payload = 块序号(2B 大端) + 数据 */
    private fun sendFwData(seq: Int, chunk: ByteArray) {
        val payload = ByteArray(2 + chunk.size)
        payload[0] = ((seq shr 8) and 0xFF).toByte()
        payload[1] = ( seq        and 0xFF).toByte()
        System.arraycopy(chunk, 0, payload, 2, chunk.size)
        publishBinary("women_safe/device_001/fw/data", payload)
    }

    /** 发固件结束：/fw/end，空 payload */
    private fun sendFwEnd() {
        publishBinary("women_safe/device_001/fw/end", ByteArray(0))
    }

    /** 二进制 publish（固件是二进制，不能用字符串版） */
    private fun publishBinary(topic: String, payload: ByteArray) {
        try {
            if (!client.isConnected) {
                _otaStatus.value = "MQTT 未连接"
                return
            }
            val msg = MqttMessage(payload).apply { qos = 1 }
            client.publish(topic, msg)
        } catch (e: Exception) {
            Log.e("MQTT", "Publish binary failed", e)
            _otaStatus.value = "发送失败: ${e.message}"
        }
    }

    /**
     * 完整 OTA 升级流程（协程中调用，避免阻塞 UI）：
     * 发升级指令 → 等重启 → 发固件头 → 分块发数据 → 发结束
     */
    suspend fun startOta(firmware: ByteArray) {
        val totalSize = firmware.size
        val totalCrc  = crc16Modbus(firmware)
        val totalBlocks = (totalSize + 125) / 126   /* 向上取整块数 */
        _otaProgress.value = 0f

        try {
            _otaStatus.value = "① 发送升级指令..."
            sendUpgradeCommand()

            /* 等 STM32 写标志并重启进 Bootloader */
            delay(2500)

            _otaStatus.value = "② 发送固件头（$totalSize 字节）..."
            sendFwStart(totalSize, totalCrc)
            delay(800)

            /* 分块发送：每块数据 126 字节（payload = 2B 块序号 + 126B 数据） */
            val chunkSize = 126
            var seq = 0
            var offset = 0
            while (offset < totalSize) {
                val len = minOf(chunkSize, totalSize - offset)
                val chunk = firmware.copyOfRange(offset, offset + len)
                sendFwData(seq, chunk)
                offset += len
                seq++
                _otaProgress.value = offset.toFloat() / totalSize
                _otaStatus.value = "③ 发送固件块 ${seq}/${totalBlocks}"
                delay(15)   /* 块间小延时，避免 ESP8266 处理不过来 */
            }

            _otaProgress.value = 1f
            _otaStatus.value = "④ 发送结束，等待设备校验..."
            sendFwEnd()
            _otaStatus.value = "升级完成 ✅"
        } catch (e: Exception) {
            Log.e("MQTT", "OTA failed", e)
            _otaStatus.value = "升级失败: ${e.message}"
        }
    }
}
