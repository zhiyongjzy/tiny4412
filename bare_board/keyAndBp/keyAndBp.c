/*************************************************************************
	> File Name: leds.c
	> Created Time: 2014年11月29日 星期四 10时33分18秒
 ************************************************************************/

#define GPX3CON (*(volatile unsigned int *)0x11000c60)
#define GPX3DAT (*(volatile unsigned int *)0x11000c64)
#define GPD0CON (*(volatile unsigned int *)0x114000a0)
#define GPD0DAT (*(volatile unsigned int *)0x114000a4)

void delay(int d)
{
	while(d--);
}

int main(void)
{
	GPX3CON &= 0xfffff0ff;
	GPX3CON |= 0x00000000;
	
	GPD0CON &= 0xfffffff0;
	GPD0CON |= 0x00000001;
	int buf;
	while (1) {
		buf = GPX3DAT;
		buf |= 0xfffffffb;
		if (buf == 0xfffffffb) {
			GPD0DAT |= 0x00000001;
		} else {
			GPD0DAT &= 0xfffffffe;
		}
	}

	return 0;
}
