/*************************************************************************
	> File Name: led.c
	> Created Time: 2014年11月22日 星期六 16时07分20秒
 ************************************************************************/

#include "config.h"

#define GPM4CON (*(volatile u32 *)0x110002e0)
#define GPM4DAT (*(volatile u8  *)0x110002e4)

static void delay(volatile u32 d)
{
	while(d--);
}

void leds_init()
{
	GPM4CON = (GPM4CON & (!0xffff)) | 1 | 1 << 4 | 1 << 8 | 1 << 12;
}

void leds_on(u8 n)
{
	GPM4DAT = (GPM4DAT & 0xf0) | (n & 0xf);
}

void led_test()
{
	while (1) { //好吧,我承认,方法......
		leds_on(0b1110);
		delay(0xffff);
		leds_on(0b1101);
		delay(0xffff);
		leds_on(0b1011);
		delay(0xffff);
		leds_on(0b0111);
		delay(0xffff);
	}
}
module_init(leds_init);


