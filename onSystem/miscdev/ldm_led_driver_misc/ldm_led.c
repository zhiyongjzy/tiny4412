#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <mach/regs-gpio.h> //S5P_VA_GPIO2
#include <linux/miscdevice.h>
#include <linux/fs.h>

#define GPM4CON (*(volatile u32 *)(S5P_VA_GPIO2 + 0x02e0)) //leds
#define GPM4DAT (*(volatile u8 *)(S5P_VA_GPIO2 + 0x02e4))

static void leds_init()
{
	GPM4CON = (GPM4CON & 0xffff0000) | 1 | 1 << 4 | 1 << 8 | 1 << 12;
}
static void leds_on(u8 n)
{
	GPM4DAT = (GPM4DAT & 0xf0) | (n & 0xf);
}
struct ldm_info {
	struct  miscdevice dev;
	struct file_operations fops;
#define BUF_SIZE 16
	char buf[BUF_SIZE];
};
struct ldm_info ldm;

static ssize_t ldm_read(struct file *fp, char __user *dest, size_t count, loff_t *f_pos)
{
	memcpy(dest, ldm.buf, count);
	return 2;
}
static ssize_t ldm_write(struct file *fp, const char __user *src, size_t count, loff_t *f_pos)
{
	leds_init();
	leds_on(*src);
	memcpy(ldm.buf, src, count);
	return 1;
}
static int  ldm_init(void)
{
	printk("hello world\n");
	ldm.dev.minor = MISC_DYNAMIC_MINOR;
	ldm.dev.name = "ldm";
	ldm.dev.fops = &ldm.fops;
	ldm.fops.write = ldm_write;
	ldm.fops.read = ldm_read;

	misc_register(&ldm.dev);
	return 0;
}
static void ldm_exit(void)
{
	printk("ByeBye \n");
	misc_deregister(&ldm.dev);
}
module_init(ldm_init);
module_exit(ldm_exit);

MODULE_LICENSE("GPL");