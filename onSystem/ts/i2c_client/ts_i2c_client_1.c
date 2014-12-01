#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>

#include <linux/i2c.h>
#include <plat/irqs.h>
#include <linux/interrupt.h>

#define DEV_NAME "FTts"
//四种方法 /linux-3.5/Documentation/i2c/instantiating-devices
//第一种方式只能静态编译, 符号表没有export到内核外

static struct i2c_board_info __initdata i2c1_board_info[] = {
	{
		I2C_BOARD_INFO(DEV_NAME, 0x70 >> 1),
		.irq = IRQ_EINT(14),
	},
	{}

};

static int __init i2c1_client_init(void)
{
	i2c_register_board_info(1, i2c1_board_info, ARRAY_SIZE(i2c1_board_info));

	return 0;
}

static void __exit i2c1_client_exit(void)
{
	
}
module_init(i2c1_client_init);
module_exit(i2c1_client_exit);

MODULE_LICENSE("GPL");