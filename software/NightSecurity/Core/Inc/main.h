/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f1xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define MQ2_Pin GPIO_PIN_0
#define MQ2_GPIO_Port GPIOA
#define MQ7_Pin GPIO_PIN_1
#define MQ7_GPIO_Port GPIOA
#define ESP_RX_Pin GPIO_PIN_2
#define ESP_RX_GPIO_Port GPIOA
#define ESP_TX_Pin GPIO_PIN_3
#define ESP_TX_GPIO_Port GPIOA
#define Fire_AO_Pin GPIO_PIN_4
#define Fire_AO_GPIO_Port GPIOA
#define BUZZER_Pin GPIO_PIN_5
#define BUZZER_GPIO_Port GPIOA
#define TOUCH_Pin GPIO_PIN_6
#define TOUCH_GPIO_Port GPIOA
#define LED_B_Pin GPIO_PIN_7
#define LED_B_GPIO_Port GPIOA
#define LED_R_Pin GPIO_PIN_0
#define LED_R_GPIO_Port GPIOB
#define LED_G_Pin GPIO_PIN_1
#define LED_G_GPIO_Port GPIOB
#define ZW101_RX_Pin GPIO_PIN_10
#define ZW101_RX_GPIO_Port GPIOB
#define ZW101_TX_Pin GPIO_PIN_11
#define ZW101_TX_GPIO_Port GPIOB
#define JQ8900_BUSY_Pin GPIO_PIN_12
#define JQ8900_BUSY_GPIO_Port GPIOB
#define SW1_Pin GPIO_PIN_13
#define SW1_GPIO_Port GPIOB
#define SW2_Pin GPIO_PIN_14
#define SW2_GPIO_Port GPIOB
#define SW3_Pin GPIO_PIN_15
#define SW3_GPIO_Port GPIOB
#define PIR_OUT_Pin GPIO_PIN_8
#define PIR_OUT_GPIO_Port GPIOA
#define LD2410C_RX_Pin GPIO_PIN_9
#define LD2410C_RX_GPIO_Port GPIOA
#define LD2410C_TX_Pin GPIO_PIN_10
#define LD2410C_TX_GPIO_Port GPIOA
#define Fire_DO_Pin GPIO_PIN_11
#define Fire_DO_GPIO_Port GPIOA
#define DHT11_DATE_Pin GPIO_PIN_12
#define DHT11_DATE_GPIO_Port GPIOA
#define JQ8900_3_Pin GPIO_PIN_3
#define JQ8900_3_GPIO_Port GPIOB
#define JQ8900_4_Pin GPIO_PIN_4
#define JQ8900_4_GPIO_Port GPIOB
#define LD2410C_OUT_Pin GPIO_PIN_5
#define LD2410C_OUT_GPIO_Port GPIOB
#define JQ8900_1_Pin GPIO_PIN_6
#define JQ8900_1_GPIO_Port GPIOB
#define JQ8900_2_Pin GPIO_PIN_7
#define JQ8900_2_GPIO_Port GPIOB
#define OLED_SCL_Pin GPIO_PIN_8
#define OLED_SCL_GPIO_Port GPIOB
#define OLED_SDA_Pin GPIO_PIN_9
#define OLED_SDA_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
