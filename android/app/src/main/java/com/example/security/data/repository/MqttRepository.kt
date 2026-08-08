package com.example.security.data.repository

import com.example.security.data.model.SensorState
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
            val msg = MqttMessage(json.toByteArray()).apply { qos = 1 }
            client.publish(topic, msg)
            Log.d("MQTT", "PUB topic=$topic payload=$json")
        } catch (e: Exception) {
            Log.e("MQTT", "Publish failed", e)
        }
    }
}
