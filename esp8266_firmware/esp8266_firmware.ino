/**********************************************************************
 * 文件名称： esp8266_firmware.ino
 * 功能描述： ESP8266-01S WiFi+MQTT+UART桥接固件
 *           接收 STM32 帧 → 发布 MQTT / 订阅 MQTT → 封装帧 → 发给 STM32
 * 硬件连接： ESP8266-01S: RX→PA2(TX), TX→PA3(RX), 115200bps 8N1
 * 依赖库：  PubSubClient by Nick O'Leary (Arduino Library Manager 安装)
 * 烧录参数： Board: Generic ESP8266 Module, Flash Size: 1M(512K SPIFFS)
 * 说    明： 1. WiFi SSID/密码 需根据实际环境修改
 *           2. MQTT Broker: 117.72.167.226:8883 (TLS 加密, 不验证证书)
 *           3. 支持用户名/密码认证 (MQTT_USER/MQTT_PASSWORD)
 *           4. device_id 硬编码为 device_001
 ***********************************************************************/

#include <ESP8266WiFi.h>
#define MQTT_MAX_PACKET_SIZE 512    /* OTA: 默认128B装不下固件块(128B+帧头尾)，须在PubSubClient前调大 */
#include <PubSubClient.h>

/* ==================== 配置 (按实际环境修改) ==================== */
#define WIFI_SSID       "(•‿•)"
#define WIFI_PASSWORD   "123456789"
#define MQTT_BROKER     "c0dde8f6.ala.cn-hangzhou.emqxsl.cn"
#define MQTT_PORT       8883              /* TLS 默认端口           */
#define MQTT_USER       "Jxy"   /* 留空""则跳过认证       */
#define MQTT_PASSWORD   "12345678"     /* 留空""则跳过认证       */
#define DEVICE_ID       "device_001"

/* ==================== 帧协议常量 ==================== */
#define FRAME_STX       0xAA
#define FRAME_ETX       0x55
#define FRAME_BUF_MAX   512

/* TYPE 码 */
#define TYPE_STATE      0x01   /* 上行: 传感器状态上报        */
#define TYPE_HEARTBEAT  0x02   /* 上行: 心跳                  */
#define TYPE_CMD        0x03   /* 下行: 控制命令(arm/disarm) */
#define TYPE_CONFIG     0x04   /* 下行: 阈值配置              */
#define TYPE_OTA_START  0x05   /* 下行: OTA 开始(总大小+总CRC) */
#define TYPE_OTA_DATA   0x06   /* 下行: OTA 数据块(块序号+数据) */
#define TYPE_OTA_END    0x07   /* 下行: OTA 结束               */
#define TYPE_OTA_ACK    0x08   /* 上行: OTA 应答(状态+块序号)  */

/* ==================== MQTT Topic ==================== */
/* 上行 (ESP8266 发布 → Broker → APP) */
#define TOPIC_PUB_STATE     "women_safe/" DEVICE_ID "/state"
#define TOPIC_PUB_HEARTBEAT "women_safe/" DEVICE_ID "/heartbeat"
/* 下行 (APP → Broker → ESP8266 订阅接收) */
#define TOPIC_SUB_CMD       "women_safe/" DEVICE_ID "/cmd"
#define TOPIC_SUB_CONFIG    "women_safe/" DEVICE_ID "/config"
/* OTA 固件（下行: 手机发固件块 / 上行: STM32 回 ACK） */
#define TOPIC_SUB_FW_START  "women_safe/" DEVICE_ID "/fw/start"
#define TOPIC_SUB_FW_DATA   "women_safe/" DEVICE_ID "/fw/data"
#define TOPIC_SUB_FW_END    "women_safe/" DEVICE_ID "/fw/end"
#define TOPIC_PUB_FW_ACK    "women_safe/" DEVICE_ID "/fw/ack"

/* ==================== 全局对象 ==================== */
WiFiClientSecure wifiClient;              /* SSL/TLS 加密           */
PubSubClient     mqttClient(wifiClient);

/* UART 接收帧的状态机 */
static uint8_t  rx_buf[FRAME_BUF_MAX];
static uint16_t rx_pos    = 0;
static uint8_t  rx_state  = 0;   /* 0=等STX 1=读TYPE 2=读LEN_H 3=读LEN_L
                                     4=读PAYLOAD 5=读CRC_H 6=读CRC_L 7=等ETX */
static uint8_t  rx_type   = 0;
static uint16_t rx_len    = 0;
static uint16_t rx_crc    = 0;
static uint8_t  rx_payload[256];

/* 心跳 */
static uint32_t last_heartbeat = 0;

/* ==================== CRC16-MODBUS ==================== */

/**********************************************************************
 * 函数名称： crc16_modbus
 * 功能描述： 计算 MODBUS CRC-16 校验值
 * 输入参数： data — 数据指针
 *            len  — 数据长度
 * 返 回 值： 16 位 CRC 值
 ***********************************************************************/
static uint16_t crc16_modbus(uint8_t *data, uint16_t len)
{
    uint16_t crc = 0xFFFF;
    for (uint16_t i = 0; i < len; i++) {
        crc ^= data[i];
        for (uint8_t j = 0; j < 8; j++) {
            if (crc & 0x0001)
                crc = (crc >> 1) ^ 0xA001;
            else
                crc >>= 1;
        }
    }
    return crc;
}

/* ==================== 帧发送 ==================== */

/**********************************************************************
 * 函数名称： send_frame
 * 功能描述： 构建帧 (STX+TYPE+LEN+PAYLOAD+CRC16+ETX) 并通过 UART 发出
 * 输入参数： type    — 帧类型
 *            payload — 负载数据
 *            len     — 负载长度
 ***********************************************************************/
static void send_frame(uint8_t type, uint8_t *payload, uint16_t len)
{
    uint8_t frame[FRAME_BUF_MAX];
    uint16_t pos = 0;

    frame[pos++] = FRAME_STX;                    /* 0xAA              */
    frame[pos++] = type;                         /* TYPE              */
    frame[pos++] = (len >> 8) & 0xFF;            /* LEN 高字节        */
    frame[pos++] =  len       & 0xFF;            /* LEN 低字节        */

    if (len > 0 && payload != NULL) {
        memcpy(&frame[pos], payload, len);
    }
    pos += len;

    /* CRC16 计算范围: TYPE + LEN(2B) + PAYLOAD */
    uint16_t crc = crc16_modbus(&frame[1], 1 + 2 + len);
    frame[pos++] = (crc >> 8) & 0xFF;
    frame[pos++] =  crc       & 0xFF;

    frame[pos++] = FRAME_ETX;                    /* 0x55              */

    Serial.write(frame, pos);
}

/* ==================== 帧接收 (状态机, 逐字节) ==================== */

/**********************************************************************
 * 函数名称： parse_byte
 * 功能描述： 将收到的字节喂入帧解析状态机, 收到完整帧时处理
 * 输入参数： ch — 收到的一个字节
 ***********************************************************************/
static void parse_byte(uint8_t ch)
{
    switch (rx_state) {

        case 0:  /* 等待 STX */
            if (ch == FRAME_STX) {
                rx_state = 1;
                rx_pos   = 0;
            }
            break;

        case 1:  /* 读 TYPE */
            rx_type  = ch;
            rx_state = 2;
            break;

        case 2:  /* 读 LEN 高字节 */
            rx_len   = (uint16_t)ch << 8;
            rx_state = 3;
            break;

        case 3:  /* 读 LEN 低字节 */
            rx_len  |= ch;
            if (rx_len > 255) {               /* 长度异常, 丢弃       */
                rx_state = 0;
                break;
            }
            rx_pos   = 0;
            rx_state = (rx_len > 0) ? 4 : 5;  /* 无负载跳过 payload  */
            break;

        case 4:  /* 读 PAYLOAD */
            if (rx_pos < 255) rx_payload[rx_pos] = ch;
            rx_pos++;
            if (rx_pos >= rx_len) rx_state = 5;
            break;

        case 5:  /* 读 CRC 高字节 */
            rx_crc   = (uint16_t)ch << 8;
            rx_state = 6;
            break;

        case 6:  /* 读 CRC 低字节 */
            rx_crc  |= ch;
            rx_state = 7;
            break;

        case 7:  /* 等 ETX */
            if (ch == FRAME_ETX) {
                /* 校验 CRC: TYPE + LEN(2B) + PAYLOAD */
                uint8_t  check_buf[2 + 256];
                uint16_t check_len = 0;
                check_buf[check_len++] = rx_type;
                check_buf[check_len++] = (rx_len >> 8) & 0xFF;
                check_buf[check_len++] =  rx_len       & 0xFF;
                if (rx_len > 0) {
                    memcpy(&check_buf[check_len], rx_payload, rx_len);
                    check_len += rx_len;
                }
                uint16_t calc_crc = crc16_modbus(check_buf, check_len);

                if (calc_crc == rx_crc) {
                    handle_frame(rx_type, rx_payload, rx_len);
                }
                /* CRC 错误 → 静默丢弃 */
            }
            rx_state = 0;                      /* 回到等 STX 状态     */
            break;

        default:
            rx_state = 0;
            break;
    }
}

/* ==================== 帧处理 ==================== */

/**********************************************************************
 * 函数名称： handle_frame
 * 功能描述： 收到完整帧后, 根据 TYPE 发布到对应 MQTT Topic
 ***********************************************************************/
static void handle_frame(uint8_t type, uint8_t *payload, uint16_t len)
{
    if (!mqttClient.connected()) return;

    /* 确保 payload 以 '\0' 结尾 (JSON 是字符串) */
    if (len < 255) payload[len] = '\0';

    switch (type) {
        case TYPE_STATE:
            mqttClient.publish(TOPIC_PUB_STATE, (const char*)payload);
            break;
        case TYPE_HEARTBEAT:
            mqttClient.publish(TOPIC_PUB_HEARTBEAT, (const char*)payload);
            break;
        case TYPE_OTA_ACK:
            /* OTA 应答是二进制(状态1B+块序号2B)，用带长度的 publish，不能用字符串版 */
            mqttClient.publish(TOPIC_PUB_FW_ACK, payload, len);
            break;
        default:
            break;                             /* 未知类型, 忽略      */
    }
}

/* ==================== MQTT 回调 ==================== */

/**********************************************************************
 * 函数名称： mqtt_callback
 * 功能描述： MQTT 收到下行消息时, 封装成帧发给 STM32
 ***********************************************************************/
static void mqtt_callback(char *topic, uint8_t *data, unsigned int length)
{
    uint8_t type;

    if (strstr(topic, "/cmd")) {
        type = TYPE_CMD;
    } else if (strstr(topic, "/config")) {
        type = TYPE_CONFIG;
    } else if (strstr(topic, "/fw/start")) {
        type = TYPE_OTA_START;                 /* 固件开始: 总大小+总CRC */
    } else if (strstr(topic, "/fw/data")) {
        type = TYPE_OTA_DATA;                  /* 固件数据块: 块序号+数据 */
    } else if (strstr(topic, "/fw/end")) {
        type = TYPE_OTA_END;                   /* 固件结束 */
    } else {
        return;                                /* 未知 topic, 忽略    */
    }

    if (length > 255) length = 255;
    send_frame(type, data, (uint16_t)length);
}

/* ==================== WiFi 连接 ==================== */

/**********************************************************************
 * 函数名称： connect_wifi
 * 功能描述： 连接 WiFi, 失败则重试
 ***********************************************************************/
static void connect_wifi(void)
{
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    uint8_t retry = 0;
    while (WiFi.status() != WL_CONNECTED && retry < 40) {
        delay(500);
        retry++;
    }
}

/* ==================== MQTT 连接 ==================== */

/**********************************************************************
 * 函数名称： connect_mqtt
 * 功能描述： 连接 MQTT Broker, 设置遗嘱消息, 订阅下行 Topic
 ***********************************************************************/
static void connect_mqtt(void)
{
    if (mqttClient.connected()) return;

    /* 遗嘱消息: 断线时 broker 自动发布 offline 状态 */
    const char *will_msg = "{\"type\":\"offline\",\"ts\":0}";
    const char *user = (strlen(MQTT_USER) > 0) ? MQTT_USER : NULL;
    const char *pass = (strlen(MQTT_PASSWORD) > 0) ? MQTT_PASSWORD : NULL;

    if (mqttClient.connect(DEVICE_ID,
            user, pass,                          /* 用户名, 密码       */
            TOPIC_PUB_STATE, 1, true, will_msg)) {   /* will topic/qos/retain/msg */

        mqttClient.subscribe(TOPIC_SUB_CMD);
        mqttClient.subscribe(TOPIC_SUB_CONFIG);
        mqttClient.subscribe(TOPIC_SUB_FW_START);   /* 订阅固件转发下行 topic */
        mqttClient.subscribe(TOPIC_SUB_FW_DATA);
        mqttClient.subscribe(TOPIC_SUB_FW_END);

        /* 上线通知 */
        mqttClient.publish(TOPIC_PUB_STATE, "{\"type\":\"online\"}");
    }
}

/* ==================== 心跳 ==================== */

/**********************************************************************
 * 函数名称： send_esp_heartbeat
 * 功能描述： 每 30s 发送 ESP8266 自身心跳 (WiFi RSSI + 运行时间)
 ***********************************************************************/
static void send_esp_heartbeat(void)
{
    uint32_t now = millis();
    if (now - last_heartbeat < 30000) return;
    last_heartbeat = now;

    char buf[128];
    snprintf(buf, sizeof(buf),
        "{\"type\":\"esp_hb\",\"rssi\":%d,\"uptime\":%lu}",
        WiFi.RSSI(), now / 1000);
    mqttClient.publish(TOPIC_PUB_HEARTBEAT, buf);
}

/* ==================== 初始化 ==================== */

void setup()
{
    /* 1. 初始化 UART (与 STM32 通信, 115200bps 帧协议) */
    Serial.begin(115200);

    /* 2. 连接 WiFi */
    connect_wifi();

    /* 3. 配置 MQTT (TLS 不验证证书) */
    wifiClient.setInsecure();                     /* 跳过证书验证        */
    mqttClient.setServer(MQTT_BROKER, MQTT_PORT);
    mqttClient.setCallback(mqtt_callback);

    /* 4. 连接 MQTT Broker */
    connect_mqtt();
}

/* ==================== 主循环 ==================== */

void loop()
{
    /* 1. 保持 MQTT 连接 */
    if (!mqttClient.connected()) {
        connect_mqtt();
    }
    mqttClient.loop();

    /* 2. 保持 WiFi 连接 */
    if (WiFi.status() != WL_CONNECTED) {
        connect_wifi();
    }

    /* 3. 读取 STM32 发来的字节 → 帧解析 */
    while (Serial.available() > 0) {
        parse_byte((uint8_t)Serial.read());
    }

    /* 4. 心跳 */
    send_esp_heartbeat();
}
