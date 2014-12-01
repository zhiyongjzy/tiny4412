#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>

#include <linux/i2c.h>
#include <linux/input.h>  //基于cdev的输入设备
#include <asm/bitops.h>   //set_bit

#include <linux/interrupt.h> // request_irq
#include <linux/workqueue.h>
#include <linux/slab.h>  //kzalloc

#define DEV_NAME "FTts" 
struct ts_info {
	struct input_dev *dev;
	struct work_struct work;
	struct i2c_client *client;
	u16 touchx, touchy;
#define PUT_DOWN 0
#define PUT_UP 1
#define CONTACT 2
	u8 event_flag;

	u8 TOUCH1_XH, TOUCH1_XL;
	u8 TOUCH1_YH, TOUCH1_YL;
};	

static void irq_work(struct work_struct *work) 
{	
	// printk("%s : %s\n", __FILE__, __FUNCTION__);
	struct ts_info *ts = container_of(work, struct ts_info, work);
	//    /linux-3.5/Documentation/i2c/smbus-protocol  最后
	i2c_smbus_read_i2c_block_data(ts->client, 0x03, 1, &ts->TOUCH1_XH);
	i2c_smbus_read_i2c_block_data(ts->client, 0x04, 1, &ts->TOUCH1_XL);
	ts->touchx = (ts->TOUCH1_XH & 0xf) << 8 | ts->TOUCH1_XL;

	i2c_smbus_read_i2c_block_data(ts->client, 0x05, 1, &ts->TOUCH1_YH);
	i2c_smbus_read_i2c_block_data(ts->client, 0x06, 1, &ts->TOUCH1_YL);
	ts->touchy = (ts->TOUCH1_YH & 0xf) << 8 | ts->TOUCH1_YL;
	
	// printk("touch  %d, %d\n", ts->touchx, ts->touchy);

	ts->event_flag = ts->TOUCH1_XH >> 6;
	switch (ts->event_flag) {
	case PUT_UP:
		input_report_key(ts->dev, BTN_TOUCH, 0);
		break;
	case PUT_DOWN:
	case CONTACT:
		input_report_abs(ts->dev, ABS_X, ts->touchx);
		input_report_abs(ts->dev, ABS_Y, ts->touchy);
		input_report_key(ts->dev, BTN_TOUCH, 1);
		break;
	}

	input_sync(ts->dev);
}

static irqreturn_t irq_handler(int irqno, void *arg)
{
	// printk("%s : %s\n", __FILE__, __FUNCTION__);
	struct ts_info *ts = (	struct ts_info *)arg;
	schedule_work(&ts->work);
	return IRQ_HANDLED;
}


static int ts_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	printk("match keywords %s\n", id->name);
	int ret = 0;

	//创建私有数据空间, 并挂载到i2c_client中
	struct ts_info *ts = kzalloc(sizeof(*ts), GFP_KERNEL);
	if (!ts) {
		ret = -ENOMEM;
		printk("kzalloc failed\n");
		goto err_kzalloc;
	}
	i2c_set_clientdata(client, ts);
	ts->client = client;

	//创建主对象input_dev
	ts->dev = input_allocate_device();
	if (!ts->dev) {
		printk("input_allocate_device failed.\n");
		ret = -ENOMEM;
		goto err_input_allocate_device;
	}
	//初始化input_dev
	//注册输入设备的输入信号类型  /linux-3.5/Documentation/inputevent-codes
	set_bit(EV_KEY, ts->dev->evbit);// 触摸屏本身是一个大的按键
	// set_bit(EV_SYN, ts->dev->evbit);
	set_bit(EV_ABS, ts->dev->evbit);//注册绝对坐标作为发送类型


	//注册键值
	set_bit(ABS_X, ts->dev->absbit);
	set_bit(ABS_Y, ts->dev->absbit);
	set_bit(BTN_TOUCH, ts->dev->keybit);

	input_set_abs_params(ts->dev, ABS_X, 0, 799, 0, 0);
	input_set_abs_params(ts->dev, ABS_Y, 0, 479, 0, 0);

	ts->dev->name = DEV_NAME;
	ts->dev->phys = "input(ts)";
	ts->dev->id.bustype = BUS_I2C;

	//申请中断
	ret = request_irq(IRQ_EINT(14), irq_handler, IRQF_TRIGGER_FALLING, "ts", ts);
	if (ret < 0) {
		printk("request_irq failed\n");
		goto err_request_irq;
	}

	//初始化工作队列
	INIT_WORK(&ts->work, irq_work);


	//注册input_dev，并创建设备节点/dev/input/eventX
	ret = input_register_device(ts->dev);
	if (ret != 0) {
		printk("input_register_device failed.\n");
		goto err_input_register_device;
	}

	return 0;

err_input_register_device:
	free_irq(IRQ_EINT(14), ts);
err_request_irq:
	input_free_device(ts->dev);
err_input_allocate_device:
	kfree(ts);
err_kzalloc:
	return ret;
}
static int ts_remove(struct i2c_client *client)
{
	struct ts_info *ts = i2c_get_clientdata(client);
	input_unregister_device(ts->dev);
	free_irq(IRQ_EINT(14), ts);
	input_free_device(ts->dev);

	kfree(ts);

	return 0;
}



static const struct i2c_device_id ts_id_table[] = {
	{ DEV_NAME, 0},
	{}
};

static struct i2c_driver ts_driver = {
	.probe		= ts_probe,
	.remove     = ts_remove,
	.id_table	= ts_id_table,//起匹配作用
	.driver	= {
		.name	= DEV_NAME,//没有用 但必须填
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

