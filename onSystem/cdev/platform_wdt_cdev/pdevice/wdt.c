#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>

#include <linux/platform_device.h>//platform机制头文件
#include <linux/ioport.h>//platform机制中device中resource结构体定义
#include <mach/irqs.h>  //IRQ_WDT

#define NUM_OF_RESOURCES 2


struct ldm_info {
	struct platform_device pdev;
	struct resource res[NUM_OF_RESOURCES];
};
static struct ldm_info ldm;
void pdev_release(struct device *dev)
{
	printk("=============pdev_release==============\n");
}

static int ldm_init(void)
{
	printk("pdevice\n");
	ldm.res[0].start = 0x10060000;
	ldm.res[0].end = 0x1006000C;
	ldm.res[0].flags = IORESOURCE_MEM;

	ldm.res[1].start = IRQ_WDT;
	ldm.res[1].end = IRQ_WDT;
	ldm.res[1].flags = IORESOURCE_IRQ;

	ldm.pdev.name = "jzywtd";//用于匹配
	ldm.pdev.dev.release = pdev_release;
	ldm.pdev.num_resources = NUM_OF_RESOURCES;
	ldm.pdev.resource = ldm.res;	

	platform_device_register(&ldm.pdev);
	return 0;
}
static void ldm_exit(void)
{
	printk("ByeBye\n");
	platform_device_unregister(&ldm.pdev);

	return;
}
module_init(ldm_init);
module_exit(ldm_exit);

MODULE_LICENSE("GPL");