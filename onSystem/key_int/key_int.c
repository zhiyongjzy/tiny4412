#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>

//arch/arm/mach-exynos/include/mach/regs-gpio.h
#include <mach/regs-gpio.h> //S5P_VA_GPIO2
#include <linux/interrupt.h> //request_irq

#define GPX3CON (*(volatile u32*)(S5P_VA_GPIO2 + 0x0c60))  //keys
#define GPX3DAT (*(volatile u8*)(S5P_VA_GPIO2 + 0x0c64))

#define GPM4CON (*(volatile u32 *)(S5P_VA_GPIO2 + 0x02e0)) //leds
#define GPM4DAT (*(volatile u8 *)(S5P_VA_GPIO2 + 0x02e4))


static void key_init()
{
	//GPX3_2-5设为input
	GPX3CON &= ~(0xffff << 8);
}
void leds_init()
{
	GPM4CON = (GPM4CON & 0xffff0000) | 1 | 1 << 4 | 1 << 8 | 1 << 12;
}
void leds_on(u8 n)
{
	GPM4DAT = (GPM4DAT & 0xf0) | (n & 0xf);
}

static irqreturn_t key_handler(int irqno, void *arg)
{
	static int flag = 0;
	printk("%s\n", __FUNCTION__);
	switch (irqno) {
		case IRQ_EINT(26):
			if (flag == 0) {
				leds_on(0b1110);
				flag = 1;
			} else {
				leds_on(0b1111);
				flag = 0;
			}
			break;
		case IRQ_EINT(27):
			leds_on(0b1101);
			break;
		case IRQ_EINT(28):
			leds_on(0b1011);
			break;
		case IRQ_EINT(29):
			leds_on(0b0111);
			break;
}

	return IRQ_HANDLED;
}


static int ldm_init(void)
{
	printk("%s\n", __FUNCTION__);

	int ret = 0;

	ret = request_irq(IRQ_EINT(26), key_handler, IRQF_TRIGGER_FALLING, "ldm", NULL);
	if (ret < 0) {
		printk("request_irq failed\n");
		goto err_request_irq;
	}
	ret = request_irq(IRQ_EINT(27), key_handler, IRQF_TRIGGER_FALLING, "ldm", NULL);
	if (ret < 0) {
		printk("request_irq failed\n");
		goto err_request_irq;
	}
	ret = request_irq(IRQ_EINT(28), key_handler, IRQF_TRIGGER_FALLING, "ldm", NULL);
	if (ret < 0) {
		printk("request_irq failed\n");
		goto err_request_irq;
	}
	ret = request_irq(IRQ_EINT(29), key_handler, IRQF_TRIGGER_FALLING, "ldm", NULL);
	if (ret < 0) {
		printk("request_irq failed\n");
		goto err_request_irq;
	}

	return 0;

err_request_irq:
	return ret;
}

static void ldm_exit(void)
{
	printk("%s\n", __FUNCTION__);

	free_irq(IRQ_EINT(26), NULL);
	free_irq(IRQ_EINT(27), NULL);
	free_irq(IRQ_EINT(28), NULL);
	free_irq(IRQ_EINT(29), NULL);
}

module_init(ldm_init);
module_exit(ldm_exit);

MODULE_LICENSE("GPL");