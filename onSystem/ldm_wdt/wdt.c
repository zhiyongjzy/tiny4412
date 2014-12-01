#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <mach/regs-clock.h>
#include <linux/io.h>
#include <linux/interrupt.h>//request_irq

#include <mach/irqs.h>  //IRQ_WDT

#define CLK_GATE_IP_PERIR (*((volatile u32 *)EXYNOS4_CLKGATE_IP_PERIR))
#define WTCON (*(volatile u32 *)wdt_reg)
#define WTDAT (*(volatile u32 *)(wdt_reg + 1))
#define WTCNT (*(volatile u32 *)(wdt_reg + 2))
#define WTCLRINT (*(volatile u32 *)(wdt_reg + 3))

static u32 *wdt_reg;
static irqreturn_t wtd_handler(int irqno, void *arg)
{
	printk("wtd timeout\n");
	WTCLRINT = 22;  //
	return IRQ_HANDLED;
}
static int ldm_init(void)
{
	printk("hello world\n");
	CLK_GATE_IP_PERIR = (CLK_GATE_IP_PERIR & ~(1 << 14)) | (1 << 14);
	wdt_reg = ioremap(0x10060000, 16);
	WTCON = WTCON & ~1 & ~(1 << 2) & ~(0b11 << 3) & ~(1 << 5) & ~(0xff << 8);
	WTCON = WTCON | (1 << 2) | (0b01 << 3) | (1 << 5) | (0xff << 8);  // 100M / 256 = 390625; 390625 / 32 = 12207

	WTDAT = 12207;  
	WTCNT = 12207;
	int ret = request_irq(IRQ_WDT, wtd_handler, IRQF_TRIGGER_NONE, "wtd", NULL);
	if (ret < 0) {
		printk("request_irq failed\n");
		goto err_request_irq;
	}
	return 0;
err_request_irq:
	return -1;
}
static void ldm_exit(void)
{
	printk("ByeBye\n");
	free_irq(IRQ_WDT, NULL);
	return;
}
module_init(ldm_init);
module_exit(ldm_exit);

MODULE_LICENSE("GPL");