/**********************************************************************
 * 文件名称： finger.h
 * 功能描述： ZW101 半导体指纹模块驱动头文件
 * 硬件连接： TX → PB10 (USART3_TX), RX → PB11 (USART3_RX), 57600bps 8N1
 *           TOUCH → PA6 (GPIO 浮空输入, 1=手指按下)
 * 来    源： 基于 ZW101 Arduino 示例 (ZW101.ino) 协议逆向, HAL 重写
 * 协议格式： 帧头 EF 01 + 地址 4Byte(FFFFFFFF) + 包标识 01 +
 *           长度 2Byte + 数据 NByte + 校验和 2Byte
 ***********************************************************************/
#ifndef __FINGER_H
#define __FINGER_H

#include "stm32f1xx_hal.h"
#include <stdbool.h>

/* ---- ZW101 协议常量 ---- */
#define FP_HEADER_HIGH      0xEF
#define FP_HEADER_LOW       0x01
#define FP_ADDR_DEFAULT     0xFFFFFFFF
#define FP_PACKET_ID_CMD    0x01    /* 命令包标识          */
#define FP_PACKET_ID_RESP   0x07    /* 应答包标识          */

/* ---- 指令码 ---- */
#define FP_CMD_GET_IMAGE      0x01  /* 获取指纹图像        */
#define FP_CMD_GEN_CHAR       0x02  /* 生成特征            */
#define FP_CMD_SEARCH         0x04  /* 搜索指纹库          */
#define FP_CMD_REG_MODEL      0x05  /* 合并特征（注册用）  */
#define FP_CMD_STORE_CHAR     0x06  /* 存储模板到指纹库    */
#define FP_CMD_READ_SYSPARA   0x0F  /* 读模组基本参数      */
#define FP_CMD_CLEAR_LIB      0x0D  /* 清空指纹库          */

/* ---- TOUCH 引脚 ---- */
#define FP_TOUCH_PORT         GPIOA
#define FP_TOUCH_PIN          GPIO_PIN_6

/* ---- 接口函数 ---- */

uint8_t Finger_IsTouched(void);               /* TOUCH 引脚: 1=手指按下 */

bool    Finger_ReadSysPara(void);             /* 读模组参数            */
bool    Finger_Enroll(void);                  /* 注册一枚指纹          */
bool    Finger_Search(void);                  /* 搜索指纹库, 返回匹配  */
bool    Finger_ClearAll(void);                /* 清空指纹库            */

#endif /* __FINGER_H */
