#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>

#include <linux/io.h>
#include <linux/interrupt.h>//request_irq
#include <linux/fs.h>

#include <linux/clk.h>

#include <linux/cdev.h>

#include <linux/platform_device.h>
#include <linux/ioport.h>

enum {
	PRESCALER = 99, //预分频值。实际分频值是(PRESCALER+1)
	DIV = 3, //二级分频值。实际分频值是(16 << DIV)
};
#define WDT_HZ ((ldm.perlr_clk) / (PRESCALER+1) / (16 << DIV))

struct wdt_reg {
	u32 con, dat, cnt, clrint;
};

struct ldm_info {
	struct platform_driver pdrv;
	struct resource *p_res_irq;//两个resource指针可以定义成指针数组
	struct resource *p_res_mem;
	struct cdev dev;
	struct file_operations fops;
	struct wdt_reg *reg;
	unsigned long perlr_clk;
}; 
struct ldm_info ldm;
	   
static irqreturn_t wtd_handler(int irqno, void *arg)
{
	printk("wtd timeout\n");
	ldm.reg->clrint = 22;  //
	return IRQ_HANDLED;
}

#define WTD_START 1000
#define WTD_STOP  1001
static long ldm_ioctl(struct file *fp, unsigned int request, unsigned long arg)
{
	switch (request) {
		case WTD_START : 
			printk("====================WTD_START===================\n");
			ldm.reg->con |= 1 << 5;			
			break;
		case WTD_STOP :
			printk("====================WTD_STOP===================\n");
			ldm.reg->con &= ~(1 << 5);		
			break;	
	}
	return 0;
}
static int ldm_probe(struct platform_device *dev)
{
	printk("%s:%s\n", __FILE__, __FUNCTION__);

	struct clk *wdt_clk = clk_get(NULL, "watchdog");
	if (IS_ERR(wdt_clk)) {
		printk("struct clk watchdog not found\n");
		goto err_clk_enable;
	}
	clk_enable(wdt_clk);

	struct clk *wdt_pclk = clk_get(NULL, "aclk_100");
	if (IS_ERR(wdt_pclk)) {
		ldm.perlr_clk = 100000000; //100MHZ
	} else {
		ldm.perlr_clk = clk_get_rate(wdt_pclk);
	}

	cdev_init(&ldm.dev, &ldm.fops);

	ldm.dev.dev = MKDEV(26, 1);

	ldm.fops.unlocked_ioctl  = ldm_ioctl;
	cdev_add(&ldm.dev, ldm.dev.dev, 1);

	

	ldm.p_res_irq = platform_get_resource(dev, IORESOURCE_IRQ, 0);

	int ret = request_irq(ldm.p_res_irq->start, wtd_handler, IRQF_TRIGGER_NONE, "wtd", NULL);
	if (ret < 0) {
		printk("request_irq failed\n");
		goto err_request_irq;
	}
	ldm.p_res_mem = platform_get_resource(dev, IORESOURCE_MEM, 0);
	ldm.reg = ioremap(ldm.p_res_mem->start , 16);
	ldm.reg->con = PRESCALER << 8 | DIV << 3 | 1 << 2;
	ldm.reg->dat = WDT_HZ;
	ldm.reg->cnt = WDT_HZ;

	return 0;
err_request_irq:
	return ret;
err_clk_enable:
	return -1;
}
static int ldm_init(void)
{
	printk("pdriver\n");

	ldm.pdrv.probe = ldm_probe;

	ldm.pdrv.driver.name = "jzywtd";
	platform_driver_register(&ldm.pdrv);
	return 0;

}
static void ldm_exit(void)
{
	printk("ByeBye\n");
	free_irq(ldm.p_res_irq->start, NULL);
	platform_driver_unregister(&ldm.pdrv);
	cdev_del(&ldm.dev);
	return;
}
module_init(ldm_init);
module_exit(ldm_exit);

MODULE_LICENSE("GPL");