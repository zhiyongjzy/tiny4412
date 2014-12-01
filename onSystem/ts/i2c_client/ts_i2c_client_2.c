#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>

#include <linux/i2c.h>
#include <plat/irqs.h>
#include <linux/interrupt.h>

#define DEV_NAME "FTts"

static struct i2c_board_info __initdata i2c_board_info = {

	I2C_BOARD_INFO(DEV_NAME, 0x70 >> 1),
	.irq = IRQ_EINT(14),
};

static const unsigned short i2c_slaveaddr_list[] = { 0x70 >> 1, 0x70 >> 1, I2C_CLIENT_END };

static int i2c1_client_init(void)
{

	struct i2c_adapter *i2c_adap;
	i2c_adap = i2c_get_adapter(1);

	i2c_new_probed_device(i2c_adap, &i2c_board_info, i2c_slaveaddr_list, NULL);
	i2c_put_adapter(i2c_adap);
	return 0;
}

static void __exit i2c1_client_exit(void)
{

}
module_init(i2c1_client_init);
module_exit(i2c1_client_exit);

MODULE_LICENSE("GPL");


