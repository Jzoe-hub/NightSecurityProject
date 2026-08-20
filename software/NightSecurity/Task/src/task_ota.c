/**********************************************************************
 * 文件名称： task_ota.c
 * 功能描述： OTA 升级入口 — 收到升级指令后写标志 + 重启进 Bootloader
 ***********************************************************************/
#include "esp8266.h"          /* 顺带包含了 stm32f1xx_hal.h */

#define OTA_FLAG_ADDR    0x0800FC00   /* 标志页地址（Flash 最后一页） */
#define OTA_FLAG_MAGIC   0x5AA5       /* 升级标志魔数 */

void OTA_EnterBootloader(void)
{
	__disable_irq();                              // ① 关中断

	FLASH_EraseInitTypeDef erase = {0};           // ② 填"擦除申请单"
	erase.TypeErase    = FLASH_TYPEERASE_PAGES;   // ③ 擦除方式：页擦除
	erase.PageAddress  = OTA_FLAG_ADDR ;              // ④ 从哪页开始：标志页
	erase.NbPages      = 1;                      // ⑤ 擦多少页：1 页

	uint32_t page_error = 0;                      // ⑥ 错误记录变量
	HAL_FLASH_Unlock();                           // ⑦ 解锁
	HAL_FLASHEx_Erase(&erase, &page_error);       // ⑧ 执行擦除


	HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD,
	                  OTA_FLAG_ADDR, OTA_FLAG_MAGIC);   // ★ 写标志 魔数0x5AA5（关键，不能漏）

	HAL_FLASH_Lock();                             // ⑨ 上锁
	__enable_irq();                               // ⑩ 开中断

	NVIC_SystemReset();                      // 重启 → 交给 Bootloader（永不返回）
}








