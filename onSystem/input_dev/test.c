#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>

#include <linux/input.h>  //基于cdev的输入设备
#include <asm/bitops.h>   //set_bit

#include <linux/interrupt.h> // request_irq
#include <mach/regs-gpio.h> //S5P_VA_GPIO2　　//fixed

#define GPX3DAT (*(volatile u8*)(S5P_VA_GPIO2 + 0x0c64))  //keys


struct ldm_info {
	struct input_dev *dev;
};
static struct ldm_info ldm;

static irqreturn_t ldm_irq_handler(int irqno, void *arg)
{
	switch (irqno) {
	case IRQ_EINT(26) : 
		input_report_key(ldm.dev, KEY_L, GPX3DAT & (1 << 2) ? 0 : 1);
		break;
	case IRQ_EINT(27) :
		input_report_key(ldm.dev, KEY_S, GPX3DAT & (1 << 3) ? 0 : 1);
		break;
	case IRQ_EINT(28) :
		input_report_key(ldm.dev, KEY_P, GPX3DAT & (1 << 4) ? 0 : 1);
		break;
	case IRQ_EINT(29) :
		input_report_key(ldm.dev, KEY_ENTER, GPX3DAT & (1 << 5) ? 0 : 1);
		break;
	}

	//=======================================================================
	//这里中断处理函数也可以不用switch case 语句,可以用arg传递参数,而input_report_key的第二个参数可以从arg里面获得.
	input_sync(ldm.dev);
	return IRQ_HANDLED;
}

static int ldm_init(void)
{
	printk("%s:%s\n", __FILE__, __FUNCTION__);
	//创建主对象input_dev
	int ret;
	ldm.dev = input_allocate_device();
	if (!ldm.dev) {
		printk("input_allocate_device failed.\n");
		ret = -ENOMEM;
		goto err_input_allocate_device;
	}
	//初始化input_dev
	//注册输入设备的输入信号类型
	set_bit(EV_KEY, ldm.dev->evbit);
	set_bit(EV_REP, ldm.dev->evbit); //连发,一直按着不放，连续触发中断

	//注册键值
	set_bit(KEY_L, ldm.dev->keybit);
	set_bit(KEY_S, ldm.dev->keybit);
	set_bit(KEY_ENTER, ldm.dev->keybit);
	set_bit(KEY_P, ldm.dev->keybit);

	//申请中断
	int i;
	for (i = 26; i < 30; i++) {
		ret = request_irq(IRQ_EINT(i), ldm_irq_handler, IRQF_TRIGGER_FALLING | IRQF_TRIGGER_RISING, "ldm", NULL);
		if (ret < 0) {
			printk("request_irq failed\n");
			goto err_request_irq;
		}
	}

	//注册input_dev，并创建设备节点/dev/input/eventX
	ret = input_register_device(ldm.dev);
	if (ret != 0) {
		printk("input_register_device failed.\n");
		goto err_input_register_device;
	}

	return 0;
err_input_allocate_device:
	return ret;
err_input_register_device:
err_request_irq:
	for (; i > 26; i--) {
		free_irq(IRQ_EINT(i), NULL);
	}
	input_free_device(ldm.dev);
	return ret;
}
static void ldm_exit(void)
{
	printk("%s:%s\n", __FILE__, __FUNCTION__);
	int i;
	for (i = 26; i < 30; i++) {
		free_irq(IRQ_EINT(i), NULL);
	}
	input_unregister_device(ldm.dev);
}
module_init(ldm_init);
module_exit(ldm_exit);

MODULE_LICENSE("GPL");