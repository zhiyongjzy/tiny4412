#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>

#include <linux/i2c.h>


static struct i2c_board_info __initdata i2c0_board_info[] = {
	{
		I2C_BOARD_INFO("eeprom0", 0b1010000),
	},
	{
		I2C_BOARD_INFO("eeprom1", 0b1010001),
	},
	{
		I2C_BOARD_INFO("eeprom2", 0b1010010),
	},
	{
		I2C_BOARD_INFO("eeprom3", 0b1010011),
	},
};

static int __init i2c0_client_init(void)
{
	i2c_register_board_info(0, i2c0_board_info, ARRAY_SIZE(i2c0_board_info));
}

static void __exit i2c0_client_exit(void)
{
	
}
module_init(i2c0_client_init);
module_exit(i2c0_client_exit);

MODULE_LICENSE("GPL");