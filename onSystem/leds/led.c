#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/io.h>

#define GPM4CON	 0x110002e0
#define GPM4DAT 	 0x110002e4
u32 *pa = NULL;
u32 *pb = NULL;
static int  leds_init(void)
{
	printk("hello world\n");
	pa = (int *)ioremap(GPM4CON, 4);
	*pa = (*pa & ~0xffff) | 1 | 1 << 4 | 1 << 8 | 1 << 12;
	pb = (int *)ioremap(GPM4DAT, 4);
	*pb = (*pb & ~0xf) | 0b0000;

	return 0;
}
static void leds_exit(void)
{
	printk("Bye \n");
	*pb = (*pb & ~0xf) | 0b1111;
	iounmap(pa);
	iounmap(pb);
}
module_init(leds_init);
module_exit(leds_exit);

MODULE_LICENSE("GPL");