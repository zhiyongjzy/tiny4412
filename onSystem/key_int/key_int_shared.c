#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>

//arch/arm/mach-exynos/include/mach/regs-gpio.h
#include <mach/regs-gpio.h> //S5P_VA_GPIO2
#include <linux/interrupt.h> //request_irq

static irqreturn_t key_handler1(int irqno, void *arg)
{
	printk("%s\n", __FUNCTION__);
	return IRQ_HANDLED;
}
static irqreturn_t key_handler2(int irqno, void *arg)
{
	printk("%s\n", __FUNCTION__);
	return IRQ_HANDLED;
}

static int ldm_init(void)
{
	printk("%s\n", __FUNCTION__);
	int ret = 0;
	ret = request_irq(IRQ_EINT(26), key_handler1, IRQF_TRIGGER_FALLING | IRQF_SHARED, "ldm", "a");
	if (ret < 0) {
		printk("request_irq failed\n");
		goto err_request_irq;
	}
	ret = request_irq(IRQ_EINT(26), key_handler2, IRQF_TRIGGER_FALLING| IRQF_SHARED, "ldm", "b");
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
	free_irq(IRQ_EINT(26), "a");
	free_irq(IRQ_EINT(26), "b");
}

module_init(ldm_init);
module_exit(ldm_exit);
MODULE_LICENSE("GPL");