/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
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
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <string.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define OTA_TYPE_START   0x05
#define OTA_TYPE_DATA    0x06
#define OTA_TYPE_END     0x07
#define OTA_TYPE_ACK     0x08

#define OTA_FLAG_ADDR    0x0800FC00   /* 标志页地址（Flash 最后一页） */
#define OTA_FLAG_MAGIC   0x5AA5       /* 升级标志魔数 */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */
uint8_t rx_byte;
//=====状态机的"记忆"变量 ======
static uint8_t  state = 0;         // 当前进行到第几步，初始=第0步
static uint8_t  rx_type = 0;       // 记 TYPE
static uint16_t rx_len  = 0;       // 记负载长度
static uint16_t rx_pos  = 0;       // 已收的负载字节数
static uint8_t  rx_payload[128];   // 负载数据暂存区
static uint16_t rx_crc  = 0;       // 记 CRC

static uint32_t ota_total_size = 0;   // 固件总大小
static uint16_t ota_total_crc  = 0;   // 固件总 CRC

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART2_UART_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
extern void jump_to_app(void);
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_USART2_UART_Init();
  /* USER CODE BEGIN 2 */
  if (*(volatile uint16_t*)OTA_FLAG_ADDR == OTA_FLAG_MAGIC)//查标志
  {
      HAL_UART_Receive_IT(&huart2, &rx_byte, 1); //有 → 进升级模式，启动串口等固件
  }
  else
  {
      jump_to_app();//无 → 直接跳 APP 正常跑（永不返回）
  }

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_RESET);

  /*Configure GPIO pin : PC13 */
  GPIO_InitStruct.Pin = GPIO_PIN_13;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */
static uint16_t crc16_modbus(uint8_t *data, uint16_t len);   // 前置声明（告诉编译器有这个函数）

void handle_frame(void)
{
    switch (rx_type)
    {
		case OTA_TYPE_START:                       // 开始帧
		{
			 // 解析：总大小(4字节) + 总CRC(2字节)，保存下来
			ota_total_size = (rx_payload[0] << 24) | (rx_payload[1] << 16)
						   | (rx_payload[2] << 8)  |  rx_payload[3];
			ota_total_crc  = (rx_payload[4] << 8)  |  rx_payload[5];
			__disable_irq();                              // ① 关中断

			FLASH_EraseInitTypeDef erase = {0};           // ② 填"擦除申请单"
			erase.TypeErase    = FLASH_TYPEERASE_PAGES;   // ③ 擦除方式：页擦除
			erase.PageAddress  = 0x08002000;              // ④ 从哪页开始：APP 区起点
			erase.NbPages      = 55;                      // ⑤ 擦多少页：55 页

			uint32_t page_error = 0;                      // ⑥ 错误记录变量
			HAL_FLASH_Unlock();                           // ⑦ 解锁
			HAL_FLASHEx_Erase(&erase, &page_error);       // ⑧ 执行擦除
			HAL_FLASH_Lock();                             // ⑨ 上锁

			__enable_irq();                               // ⑩ 开中断
			HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_SET);    // ⑪ 擦完灭灯（区别于上电的亮）

			break;
		}
		case OTA_TYPE_DATA:
		{
		    uint16_t seq = (rx_payload[0] << 8) | rx_payload[1];   // ① 块序号（前2字节）
		    uint16_t data_len = rx_len - 2;                         // ② 数据长度 = 负载长 - 序号2字节
		    uint32_t addr = 0x08002000 + (uint32_t)seq * 126;       // ③ 写入地址

		    __disable_irq();
		    HAL_FLASH_Unlock();
		    for (uint16_t i = 0; i < data_len; i += 2)              // ④ 每2字节写一次
		    {
		        uint16_t halfword = rx_payload[2 + i] | (rx_payload[2 + i + 1] << 8);  // ⑤ 拼半字
		        HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, addr + i, halfword);     // ⑥ 写入
		    }
		    HAL_FLASH_Lock();
		    __enable_irq();

		    HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);   // 写一块翻转一次灯
		    break;
		}
	 case OTA_TYPE_END:
	  {
		  uint16_t calc_crc = crc16_modbus((uint8_t*)0x08002000, ota_total_size);
		  if (calc_crc == ota_total_crc)
		  {
			  // ===== 升级成功：清标志 + 跳新 APP =====
			  __disable_irq();
			  HAL_FLASH_Unlock();
			  FLASH_EraseInitTypeDef erase = {0};
			  erase.TypeErase    = FLASH_TYPEERASE_PAGES;
			  erase.PageAddress  = OTA_FLAG_ADDR;
			  erase.NbPages      = 1;
			  uint32_t page_error = 0;
			  HAL_FLASHEx_Erase(&erase, &page_error); // 擦标志页 = 清标志
			  HAL_FLASH_Lock();
			  __enable_irq();

			  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_RESET);  // 灯亮
			  jump_to_app();                               // 跳新 APP（永不返回）
		  }
		  else
		  {
		     HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_SET);    // 校验失败 → 灯灭（标志保留，下次重启再升级）

		  }
		  break;
	  }

		default:                                   // 其他类型，忽略
			break;
    }
}

/* ==================== CRC16-MODBUS ==================== */

/**********************************************************************
 * 函数名称： crc16_modbus
 * 功能描述： 计算 MODBUS CRC-16 校验值（与 ESP8266 端一致）
 * 输入参数： data — 数据指针
 *            len  — 数据长度
 * 返 回 值： 16 位 CRC 值
 ***********************************************************************/
static uint16_t crc16_modbus(uint8_t *data, uint16_t len)
{
	uint16_t crc = 0xFFFF;              // 1. 从一个固定初值开始
	for (uint16_t i = 0; i < len; i++) {
	    crc ^= data[i];                 // 2. 把当前字节"揉进"crc（异或）
	    for (uint16_t j = 0; j < 8; j++) {       // 3. 揉完再搅 8 次
	        if (crc & 0x0001)           //    最低位是 1 就...
	            crc = (crc >> 1) ^ 0xA001;  // 右移再异或一个固定值
	        else
	            crc >>= 1;              //    否则只右移
	    }
	}
	return crc;                         // 4. 得到 2 字节"指纹"
}


void parse_byte(uint8_t ch)        // ch = 刚收到的 1 个字节
{
    switch (state)                 // 看"现在第几步"，走对应的分支
    {
    case 0:                        // 第0步：等帧头
        if (ch == 0xAA) state = 1; // 是帧头→进第1步；不是→继续等
        break;

    case 1:                        // 第1步：读 TYPE
        rx_type = ch;              // 把这个字节当 TYPE 存下
        state = 2;                 // 进第2步
        break;

    case 2:                        // 第2步：读 LEN 高字节
        rx_len = ch << 8;          // 存成"高半截"
        state = 3;
        break;

    case 3:                        // 第3步：读 LEN 低字节
        rx_len |= ch;              // 拼上"低半截"，得到完整长度
        rx_pos = 0;                // 负载还没开始收，计数清零
        state = (rx_len > 0) ? 4 : 5;  // 有负载去第4步，没负载直接去第5步
        break;

    case 4:                        // 第4步：收负载
        rx_payload[rx_pos++] = ch; // 存一个字节，计数+1
        if (rx_pos >= rx_len)      // 收够 len 个了？
            state = 5;             // 收够了，去读 CRC
        break;

    case 5:                        // 第5步：读 CRC 高字节
        rx_crc = ch << 8;
        state = 6;
        break;

    case 6:                        // 第6步：读 CRC 低字节
        rx_crc |= ch;              // 拼出完整 CRC
        state = 7;
        break;

    case 7:                        // 第7步：等帧尾
	if (ch == 0x55)
	{                       // ① 先判断帧尾对不对
		static uint8_t check_buf[1 + 2 + 128];
		uint16_t pos = 0;
		check_buf[pos++] = rx_type;
		check_buf[pos++] = (rx_len >> 8) & 0xFF;
		check_buf[pos++] =  rx_len       & 0xFF;
		memcpy(&check_buf[pos], rx_payload, rx_len);
		pos += rx_len;
		uint16_t calc = crc16_modbus(check_buf, pos);
		if (calc == rx_crc) 				// ② 再判断 CRC 对不对
		{
			handle_frame();                 // 两个都对 → 才处理
		}
	}
	state = 0;                              // 回到第 0 步等下一帧
	break;
    }
}



void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	if (huart->Instance == USART2)
	{
		parse_byte(rx_byte);                        // 1. 喂给状态机
		HAL_UART_Receive_IT(&huart2, &rx_byte, 1);  // 2. 再收下一个字节（关键！）
	}
}
/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
