#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>

#include <linux/cdev.h>
#include <linux/fs.h>


struct ldm_info {
	struct cdev dev;
	struct file_operations fops;
};
static struct ldm_info ldm;

static int ldm_init(void)
{
	printk("%s:%s\n", __FILE__, __FUNCTION__);
	// ldm.dev.ops = &ldm.fops;
	cdev_init(&ldm.dev, &ldm.fops);

	ldm.dev.dev = MKDEV(26, 1);
	

	if (register_chrdev_region(ldm.dev.dev, 1, "ldm") < 0) {
		printk("register_chrdev_region failed\n");
		return -1;
	}
	printk("%d   %d\n", MAJOR(ldm.dev.dev), MINOR(ldm.dev.dev));
	cdev_add(&ldm.dev, ldm.dev.dev, 1);

	return 0;
}
static void ldm_exit(void)
{
	printk("%s:%s\n", __FILE__, __FUNCTION__);	
	cdev_del(&ldm.dev);
	unregister_chrdev_region(ldm.dev.dev, 1);

}
module_init(ldm_init);
module_exit(ldm_exit);

MODULE_LICENSE("GPL");