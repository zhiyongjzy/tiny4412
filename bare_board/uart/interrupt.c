#include "config.h"

#define ICCICR_CPU0		(*(volatile u32*)0x10480000)
#define ICCPMR_CPU0		(*(volatile u32*)0x10480004)
#define ICCIAR_CPU0     (*(volatile u32*)0x1048000c)
#define ICCEOIR_CPU0    (*(volatile u32*)0x10480010)
#define ICDDCR          (*(volatile u32*)0x10490000)
#define ICDISER_CPU0(n) (*(volatile u32*)(0x10490100 + 4 * (n)))
#define ICDIPTR_CPU0(n) (*(volatile u32*)(0x10490800 + 4 * (n)))

static void (*irq_handler_array[160])();

void request_irq(int irqno, void (*handler)())
{
//	printk("%s: irqno=%d\n", __FUNCTION__, irqno);

	//打开cpu0的中断使能
	ICCICR_CPU0 |= 1;

	//设置cpu0的优先级为最低，中断号的优先级必须比cpu默认中断优先级高，才能触发该中断
	ICCPMR_CPU0 = 0xff;

	//中断控制器的外围中断总开关，关闭后就只能相应SGI
	ICDDCR |= 1;

	//开启对应中断号的开关
	ICDISER_CPU0(irqno / 32) |= 1 << (irqno % 32);

	//指定对应中断号交由哪几个cpu核来触发，每个寄存器管4个中断号，每个中断号由8位设置，8位中的每一位对应一个cpu核。我们只开启cpu0的使能
	ICDIPTR_CPU0(irqno / 4) &= ~(0xff << ((irqno % 4) * 8));
	ICDIPTR_CPU0(irqno / 4) |= 1 << ((irqno % 4) * 8);

	//挂接中断处理函数
	irq_handler_array[irqno] = handler;
}

void irq_handler()
{
	//printk("%s\n", __FUNCTION__);

	//读出当前触发的中断号
	int irqno = ICCIAR_CPU0 & 0x3ff;

	irq_handler_array[irqno]();

	//清除中断控制器的标志，通知中断控制器，可以发送下一个中断了
	ICCEOIR_CPU0 = irqno;
}

void swi_handler()
{
	led_test();
}
