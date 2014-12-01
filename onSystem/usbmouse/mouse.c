#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>

#include <linux/usb/input.h>
#include <linux/hid.h>
#include <linux/usb.h>
#include <linux/slab.h>

struct mouse_info {
	struct urb *urb;
	dma_addr_t data_phys;
	int maxp; //当前endpoint最大包的字节数
	struct input_dev *inputdev;
};
static struct mouse_info info;

static void mouse_irq(struct urb *urb)
{
	s8 *data = (s8 *)urb->context;

	input_report_key(info.inputdev, BTN_LEFT,   data[0] & 0x01);
	input_report_key(info.inputdev, BTN_RIGHT,  data[0] & 0x02);
	input_report_key(info.inputdev, BTN_MIDDLE, data[0] & 0x04);
	//input_report_key(info.inputdev, BTN_SIDE,   data[0] & 0x08);
	//input_report_key(info.inputdev, BTN_EXTRA,  data[0] & 0x10);
	printk("BTN left %d, right %d, middle %d\n",  data[0] & 0x01,  data[0] & 0x02,  data[0] & 0x04);

	input_report_rel(info.inputdev, REL_X,     data[1]);
	input_report_rel(info.inputdev, REL_Y,     data[2]);
	input_report_rel(info.inputdev, REL_WHEEL, data[3]);
	printk("x=%d, y=%d, wheel=%d\n", data[1], data[2], data[3]);

	input_sync(info.inputdev);

	//中断上下文，不允许有可能导致休眠的函数
	usb_submit_urb(urb, GFP_ATOMIC);
}


static int usb_mouse_probe(struct usb_interface *intf, const struct usb_device_id *id)
{
	printk("%s: %s\n", __FILE__, __FUNCTION__);

	struct usb_device *dev = interface_to_usbdev(intf);
	struct usb_device_descriptor *desc = &dev->descriptor;

	printk("idVendor=%x, idProduct=%x, class=%x, subclass=%x\n", desc->idVendor, desc->idProduct, desc->bDeviceClass, desc->bDeviceSubClass);

	struct usb_host_interface *interface = intf->cur_altsetting;
	//usb鼠标唯一的数据节点，输入，中断类型
	struct usb_endpoint_descriptor *endpoint = &interface->endpoint[0].desc;


	//为usb_fill_int_urb做准备
	info.urb = usb_alloc_urb(0, GFP_KERNEL);

	//pipe就是endpoint数据源，从usb设备接收中断型数据包
	int pipe = usb_rcvintpipe(dev, endpoint->bEndpointAddress);
	//当前endpoint的包的最大字节数
	//info.maxp = usb_maxpacket(dev, pipe, usb_pipeout(pipe));
	info.maxp = endpoint->wMaxPacketSize;
	printk("maxp = %d\n", info.maxp);

	//数据包的缓冲区
	s8 *data = usb_alloc_coherent(dev, info.maxp, GFP_ATOMIC, &info.data_phys);

	//初始化urb(usb request block)
	usb_fill_int_urb(info.urb, dev, pipe, data, info.maxp, mouse_irq, data, endpoint->bInterval);

	info.urb->transfer_dma = info.data_phys;
	info.urb->transfer_flags |= URB_NO_TRANSFER_DMA_MAP;

	//提交urb
	usb_submit_urb(info.urb, GFP_KERNEL);



	//创建输入子系统对象
	info.inputdev = input_allocate_device();
	if (!(info.inputdev)) {
		goto err_alloc_input_dev;
	}

	//设置输入功能，按键和相对位移
#if 0
	info.inputdev->evbit[0] = BIT_MASK(EV_KEY) | BIT_MASK(EV_REL);
	info.inputdev->keybit[BIT_WORD(BTN_MOUSE)] = BIT_MASK(BTN_LEFT) |
		BIT_MASK(BTN_RIGHT) | BIT_MASK(BTN_MIDDLE);
	info.inputdev->relbit[0] = BIT_MASK(REL_X) | BIT_MASK(REL_Y);
	info.inputdev->relbit[0] |= BIT_MASK(REL_WHEEL);
#endif
#if 1
	set_bit(EV_KEY, info.inputdev->evbit);
	set_bit(EV_REL, info.inputdev->evbit);
	set_bit(BTN_LEFT, info.inputdev->keybit);
	set_bit(BTN_RIGHT, info.inputdev->keybit);
	set_bit(BTN_MIDDLE, info.inputdev->keybit);
	set_bit(REL_X, info.inputdev->relbit);
	set_bit(REL_Y, info.inputdev->relbit);
	set_bit(REL_WHEEL, info.inputdev->relbit);
#endif


	if (input_register_device(info.inputdev) < 0) {
		goto err_register_dev;
	}

	return 0;

err_register_dev:
	input_free_device(info.inputdev);
err_alloc_input_dev:
	usb_kill_urb(info.urb);
	usb_free_urb(info.urb);
	usb_free_coherent(interface_to_usbdev(intf), info.maxp, info.urb->context, info.data_phys);
	return -1;
}

static void usb_mouse_disconnect(struct usb_interface *intf)
{
	printk("%s: %s\n", __FILE__, __FUNCTION__);
	input_free_device(info.inputdev);
	usb_kill_urb(info.urb);
	usb_free_urb(info.urb);
	usb_free_coherent(interface_to_usbdev(intf), info.maxp, info.urb->context, info.data_phys);
}

//usb鼠标设备的设备号信息
static struct usb_device_id usb_mouse_id_table [] = {
	{ USB_INTERFACE_INFO(USB_INTERFACE_CLASS_HID, USB_INTERFACE_SUBCLASS_BOOT, USB_INTERFACE_PROTOCOL_MOUSE) },
	{ }	/* 哨兵 */
};

static struct usb_driver usb_mouse_driver = {
	.name		= "mouse",
	.probe		= usb_mouse_probe,
	.disconnect	= usb_mouse_disconnect,
	.id_table	= usb_mouse_id_table,
};

static int __init usb_init(void)
{
	printk("%s: %s\n", __FILE__, __FUNCTION__);
	if (usb_register(&usb_mouse_driver) < 0) {
		printk("%s: usb_register failed", __FUNCTION__);
		goto err_usb_register;
	}
	return 0;
err_usb_register:
	return -1;
}

static void __exit usb_exit(void)
{
	printk("%s: %s\n", __FILE__, __FUNCTION__);
	usb_deregister(&usb_mouse_driver);
}

module_init(usb_init);
module_exit(usb_exit);

MODULE_LICENSE("GPL");
