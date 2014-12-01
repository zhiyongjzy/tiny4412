#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>

#include <linux/i2c.h>
#include <linux/input.h>  //基于cdev的输入设备
#include <asm/bitops.h>   //set_bit

#include <linux/interrupt.h> // request_irq
#include <linux/workqueue.h>

#define DEV_NAME "FTts" 
struct ts_info {
	struct input_dev *dev;
	struct work_struct work;
	struct i2c_client *client;
	u16 touchx, touchy;
	u8 TOUCH1_XH, TOUCH1_XL;
	u8 TOUCH1_YH, TOUCH1_YL;
};	
static struct ts_info ts;

static void irq_work(struct work_struct *work) 
{	
	// printk("%s : %s\n", __FILE__, __FUNCTION__);

///linux-3.5/Documentation/i2c/smbus-protocol  最后
	i2c_smbus_read_i2c_block_data(ts.client, 0x03, 8, &ts.TOUCH1_XH);
	i2c_smbus_read_i2c_block_data(ts.client, 0x04, 8, &ts.TOUCH1_XL);
	ts.touchx = (ts.TOUCH1_XH & 0xf) << 8 | ts.TOUCH1_XL;

	i2c_smbus_read_i2c_block_data(ts.client, 0x05, 8, &ts.TOUCH1_YH);
	i2c_smbus_read_i2c_block_data(ts.client, 0x06, 8, &ts.TOUCH1_YL);
	ts.touchy = (ts.TOUCH1_YH & 0xf) << 8 | ts.TOUCH1_YL;
	
	printk("touch  %d, %d\n", ts.touchx, ts.touchy);

	input_report_abs(ts.dev, ABS_X, ts.touchx);
	input_report_abs(ts.dev, ABS_Y, ts.touchy);
	input_report_key(ts.dev, BTN_TOUCH, 1);
	input_sync(ts.dev);

}

static irqreturn_t irq_handler(int irqno, void *arg)
{
	// printk("%s : %s\n", __FILE__, __FUNCTION__);
	schedule_work(&ts.work);
	// printk("schedule_work return %d\n", ret);
	return IRQ_HANDLED;
}


static int ts_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	printk("match keywords %s\n", id->name);
	ts.client = client;

	//创建主对象input_dev
	int ret;
	ts.dev = input_allocate_device();
	if (!ts.dev) {
		printk("input_allocate_device failed.\n");
		ret = -ENOMEM;
		goto err_input_allocate_device;
	}
	//初始化input_dev
	//注册输入设备的输入信号类型
	set_bit(EV_KEY, ts.dev->evbit);
	set_bit(EV_SYN, ts.dev->evbit);
	set_bit(EV_ABS, ts.dev->evbit);


	//注册键值
	set_bit(ABS_X, ts.dev->absbit);
	set_bit(ABS_Y, ts.dev->absbit);
	set_bit(BTN_TOUCH, ts.dev->keybit);

	input_set_abs_params(ts.dev, ABS_X, 0, 799, 0, 0);
	input_set_abs_params(ts.dev, ABS_Y, 0, 479, 0, 0);

	ts.dev->name = DEV_NAME;
	ts.dev->phys = "input(ts)";
	ts.dev->id.bustype = BUS_I2C;

	//申请中断
	ret = request_irq(IRQ_EINT(14), irq_handler, IRQF_TRIGGER_FALLING, "ts", NULL);
	if (ret < 0) {
		printk("request_irq failed\n");
		goto err_request_irq;
	}

	//初始化工作队列
	INIT_WORK(&ts.work, irq_work);


	//注册input_dev，并创建设备节点/dev/input/eventX
	ret = input_register_device(ts.dev);
	if (ret != 0) {
		printk("input_register_device failed.\n");
		goto err_input_register_device;
	}

	return 0;

	err_input_register_device:
	free_irq(IRQ_EINT(14), NULL);
	err_request_irq:
	input_free_device(ts.dev);
	err_input_allocate_device:
	return ret;
}
static int ts_remove(struct i2c_client *client)
{
	input_unregister_device(ts.dev);
	free_irq(IRQ_EINT(14), NULL);
	input_free_device(ts.dev);

	return 0;
}



static const struct i2c_device_id ts_id_table[] = {
	{ DEV_NAME, 0},
	{}
};

static struct i2c_driver ts_driver = {
	.probe		= ts_probe,
	.remove     = ts_remove,
	.id_table	= ts_id_table,
	.driver	= {
		.name	= DEV_NAME,
		.owner	= THIS_MODULE,
	},
};
static int __init ts_driver_init(void)
{
	printk("%s %s\n", __FILE__, __FUNCTION__);
	return i2c_add_driver(&ts_driver);
}

static void __exit ts_driver_exit(void)
{
	return i2c_del_driver(&ts_driver);
}
module_init(ts_driver_init);
module_exit(ts_driver_exit);

MODULE_LICENSE("GPL");

