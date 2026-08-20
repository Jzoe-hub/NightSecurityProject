#include "main.h"

/*① 设 VTOR
SCB->VTOR = 0x08004000 · 电话本拨过去
② 设栈顶
读 APP 第 0 格 → 写进 MSP · 架好工作台
③ 读入口
读 APP 第 1 格 → 得到 Reset_Handler 地址
④ 跳转
调用 Reset_Handler · 永不返回*/
#define  APP_ADDR 0x08002000 //app的向量表

typedef void (*pFunction)(void);      //定义函数指针

void jump_to_app(void)
{
	uint32_t appStack = *(volatile uint32_t*)APP_ADDR;  //取出app栈顶所需的向量表第0格
/*APP_ADDR 是个数字 0x08004000；
 * • (uint32_t*)APP_ADDR = 把这个数字当成一个"指向 4 字节整数的地址"（地址→指针）；
 * • 最前面的 * = 去那个地址读出 4 字节。读出来的就是「初始栈顶值」。
 * • volatile = 告诉编译器「别自作聪明优化掉，必须真的去内存读」。*/
	pFunction appEntry = (pFunction)(*(volatile uint32_t*)(APP_ADDR + 4));//取出跳转所需的第一格

	SCB->VTOR = APP_ADDR;   // 切换向量表
	__set_MSP(appStack);    // 设栈顶
	appEntry();             // 跳转，永不返回

}
