#include "config.h"

#define BAUD_RATE 115200
#define SCLK_UART 100000000 //100MHZ

#define UART_GPIO 			(*(volatile u32 *)0x11400000)  //GPA0CON

#define CLK_SRC_PERIL0 		(*(volatile u32*)0x1003c250)
#define CLK_DIV_PERIL0		(*(volatile u32*)0x1003c550)

#define ULCON 				(*(volatile u32*)0x13800000)
#define UCON 				(*(volatile u32*)0x13800004)
#define UFCON 				(*(volatile u32*)0x13800008)
#define UTRSTAT 			(*(volatile u32*)0x13800010)
#define UMCON 				(*(volatile u32*)0x1380000c)

#define UTXH 				(*(volatile u8*)0x13800020)
#define URXH 				(*(volatile u8*)0x13800024)

#define UBRDIV 				(*(volatile u16*)0x13800028)
#define UFRACVAL 			(*(volatile u8*)0x1380002c)





void uart_send(u8 ch)
{
	//轮询UTRSTAT[1]确定写入发送缓冲区的时机
	while (!((UTRSTAT >> 1) & 0x1));
	UTXH = ch;
}

u8 uart_recv()
{
	//轮询检查UTRSTAT[0]，确定什么时候收到了字节
	while (!(UTRSTAT & 0x1));
	return (u8)URXH;
}
void print_ch(u8 ch)
{
	if (ch == '\n') {
		uart_send('\r');
		uart_send('\n');
	} else if (ch == '\r') {
		uart_send('\r');
		uart_send('\n');
	} else {
		uart_send(ch);
	}
}

void print_str(u8 *str)
{
	while (*str != '\0')
		print_ch(*str++);
}

static void uart_init()
{
	//1 GPIO设置
	UART_GPIO &= ~0xff;
	UART_GPIO |= 0x22;
	
	//2 时钟源设置
	//CLK_SRC_PERIL0选择初始时钟源,手册P512
	CLK_SRC_PERIL0 &= ~0xf;
	CLK_SRC_PERIL0 |= 0b0110;

	//CLK_DIV_PERIL0设置对PERIL0的时钟源的分频值P534
	CLK_DIV_PERIL0 &= ~0xf;
	CLK_DIV_PERIL0 |= 0b0111;

	//3 串口初始化
	//协议设置，8n1 非红外模式,设置低７位
	ULCON = 0x3;
	//从后往前11word length 0 one stop bit 0xx No parity 0 normal mode operation(区别于红外）
	
	//设置中断和dma，如果采用轮询，则设为0b0101
	UCON = 0b0101;
	//轮询不能使用fifo，fifo关闭后，缓冲区就只剩下一个字节
	UFCON = 0;
	//关闭流控制
	UMCON = 0;
	//设置波特率
	//SCLK_UART / (BAUD_RATE * 16) - 1 = DIV_VAL
	//100000000 / (115200 * 16) - 1 = 53.25
	//UBRDIV = 53
	//UFRACVAL = 0.25 * 16 = 4
	UBRDIV = 53; //分频值的整数部分  十进制　５３
	UFRACVAL = 4; //小数部分
	
	//将发送寄存器里面值置零，默认值是随机的
	//UTXH = 0;
}
module_init(uart_init);
