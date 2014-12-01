#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>

#include <linux/i2c.h>


static int __init i2c0_driver_init(void)
{
	
}

static void __exit i2c0_driver_exit(void)
{

}
module_init(i2c0_init);
module_exit(i2c0_exit);

MODULE_LICENSE("GPL");