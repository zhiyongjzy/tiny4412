#include "config.h"

#define GPA0CON 	(*(volatile u32 *)0x11400000)

#define ULCON 		(*(volatile u32 *)0x13800000)
#define UCON 		(*(volatile u32 *)0x13800004)
#define UFCON 		(*(volatile u32 *)0x13800008)
#define UTRSTAT 	(*(volatile u32 *)0x13800010)
#define UFSTAT 		(*(volatile u32 *)0x13800018)
#define UTXH 		(*(volatile u8  *)0x13800020)
#define URXH 		(*(volatile u8  *)0x13800024)
#define UBRDIV 		(*(volatile u32 *)0x13800028)
#define UFRACVAL 	(*(volatile u32 *)0x1380002c)
#define UINTP 		(*(volatile u32 *)0x13800030)
#define UINTSP 		(*(volatile u32 *)0x13800034)
#define UINTM 		(*(volatile u32 *)0x13800038)

#define SCLK_UART 100000000 //100MHZ，手册P1393，串口时钟
#define BAUD_RATE 115200

static void send_ch(u8 ch)
{
	while (!(UTRSTAT & (1 << 1))) {}
	//判断UTRSTAT
	UTXH = ch;
}

//串口协议遵循windows对于回车的标准，串口的一个回车是由\n\r两个字符组成
void uart_print_ch(u8 ch)
{
	if (ch == '\n') {
		send_ch('\r');
	} else if (ch == '\r') {
		send_ch('\n');
	}
	send_ch(ch);
}

void uart_print_str(const char *str)
{
	for (; *str; ++str) {
		uart_print_ch(*str);
	}
}

static u8 uart_recv_poll()
{
	while (!(UTRSTAT & 1)) {}
	//根据UTRSTAT来判断是否收到数据
	return URXH;
}

static void uart_handler()
{
	//printk("%s\n", __FUNCTION__);

	if (UFSTAT & 1 << 8) {
		printk("uart rx fifo full\n");
	}

	while (UFSTAT & 0xff) {
		uart_print_ch(URXH);
	}

	//清除中断标志
	UINTP = 0xf;
}

void uart_init()
{
	//1 管脚GPIO设置GPA0_0和GPA0_1
	GPA0CON = (GPA0CON & ~0xff) | 0x22;

	//2 初始化寄存器
	/* ULCON: 8N1 */
	ULCON = 3;

	/* UCON
	 * [3:0]: 中断或轮询，非DMA模式
	 * [4]: 是否发送停止信号，不发送
	 * [5]: 非环回模式，环回loop-back即自发自收
	 * [6]: 错误中断使能，关
	 * [7]: 接收超时是否产生接收中断，如果工作在中断模式则必开
	 * [9:8]: 中断触发类型，只能是电平触发
	 * [10]: 接收超时的dma休眠使能
	 * [11]: 当接收缓冲区为0字节时触发中断，关掉
	 * [15:12]: 接收超时中断的触发间隔时间
	 * [18:16][22:20]: dma burst size
	 */
	UCON = 0b0101 | 1 << 7 | 1 << 8 | 1 << 9 | 0x3 << 12;

	//fifo
	UFCON = 0b111 << 8 | 0b111 << 4; //fifo触发边沿最大
	UFCON |= 1 << 1 | 1 << 2; //reset
	UFCON |= 1; //打开

	/* UBRDIV UFRACVAL, 分频得到波特率，详情和公式见手册P1410 */
	//DIV_VAL = SCLK_UART / (bps * 16) - 1 = 100MHZ / (115200 * 16) - 1 = 53.25
	//DIV_VAL = UBRDIV + UFRACVAL/16
	//UBRDIV是DIV_VAL的整数部分53，UFRACVAL/16是小数部分，UFRACVAL = 0.25 * 16 = 4
	UBRDIV = 53;
	UFRACVAL = 4;

	//清除中断标志
	UINTP = 0xf;

	//注册中断
	request_irq(84, uart_handler);

	//只留接收中断，关闭其他中断
	UINTM = 1 << 1 | 1 << 2 | 1 << 3;
}

void uart_test()
{
	uart_init();
	uart_print_str("uart_test\n");
	
	while (1) {
		uart_print_ch(uart_recv_poll());
	}
}

//module_init(uart_init);
