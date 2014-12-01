#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <mach/regs-clock.h>  //EXYNOS4_CLKGATE_IP_PERIR
#include <linux/io.h>
#include <linux/interrupt.h>//request_irq
#include <linux/fs.h>



#include <linux/clk.h>

#include <linux/miscdevice.h>


#include <mach/irqs.h>  //IRQ_WDT
//================================================================
//#define CLK_GATE_IP_PERIR (*((volatile u32 *)EXYNOS4_CLKGATE_IP_PERIR))
//==================================================================
#define WTCON (*(volatile u32 *)wdt_reg)
#define WTDAT (*(volatile u32 *)(wdt_reg + 1))
#define WTCNT (*(volatile u32 *)(wdt_reg + 2))
#define WTCLRINT (*(volatile u32 *)(wdt_reg + 3))

static u32 *wdt_reg;
struct ldm_info {
	struct miscdevice dev;
	struct file_operations fops;
}; 
struct ldm_info ldm;

static irqreturn_t wtd_handler(int irqno, void *arg)
{
	printk("wtd timeout\n");
	WTCLRINT = 22;  //
	return IRQ_HANDLED;
}
#define WTD_START 1000
#define WTD_STOP  1001
static long ldm_ioctl(struct file *fp, unsigned int request, unsigned long arg)
{
	switch (request) {
		case WTD_START : 
			WTCON = WTCON & ~1 & ~(1 << 2) & ~(0b11 << 3) & ~(1 << 5) & ~(0xff << 8);
			WTCON = WTCON | (1 << 2) | (0b01 << 3) | (1 << 5) | (0xff << 8);  // 100M / 256 = 390625; 390625 / 32 = 12207

			WTDAT = 12207;  
			WTCNT = 12207;			
			break;
		case WTD_STOP :
			printk("=======================================\n");
			WTCON &= ~(1 << 5);			
			break;	
	}
	return 0;
}
static int ldm_init(void)
{
	printk("hello world\n");
	int ret = request_irq(IRQ_WDT, wtd_handler, IRQF_TRIGGER_NONE, "wtd", NULL);
	if (ret < 0) {
		printk("request_irq failed\n");
		goto err_request_irq;
	}
	//===============================================================================
	// CLK_GATE_IP_PERIR |= (1 << 14);
	struct clk *clk = clk_get(NULL, "wathchdog");
	clk_enable(clk);


	clk = clk_get(NULL, "aclk_100");
	printk("get clk rate from kernel %lu\n", clk_get_rate(clk));
	//===============================================================================

	wdt_reg = ioremap(0x10060000, 16);
	
	ldm.dev.minor = MISC_DYNAMIC_MINOR;
	ldm.dev.name = "ldm";
	ldm.dev.fops = &ldm.fops;
	ldm.fops.unlocked_ioctl  = ldm_ioctl;

	misc_register(&ldm.dev);

	return 0;
err_request_irq:
	return -1;
}
static void ldm_exit(void)
{
	printk("ByeBye\n");
	iounmap(wdt_reg);
	free_irq(IRQ_WDT, NULL);
	misc_deregister(&ldm.dev);
	return;
}
module_init(ldm_init);
module_exit(ldm_exit);

MODULE_LICENSE("GPL");
