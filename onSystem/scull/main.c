#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>

#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/slab.h> //kmalloc
#include <linux/fcntl.h>

#include <asm/uaccess.h>// copy_xxxx_user

#include "scull.h"


int scull_major 	= SCULL_MAJOR;
int scull_minor 	= 0;
int scull_nr_devs 	= SCULL_NR_DEVS;	/* scull设备数 scull */
int scull_quantum 	= SCULL_QUANTUM;
int scull_qset 		= SCULL_QSET;

module_param(scull_major, int, S_IRUGO);//S_IRUGO 所有人只读
module_param(scull_minor, int, S_IRUGO);
module_param(scull_nr_devs, int, S_IRUGO);
module_param(scull_quantum, int, S_IRUGO);
module_param(scull_qset, int, S_IRUGO);

struct scull_dev *scull_devices;
struct file_operations scull_fops = {
	.owner =    THIS_MODULE,//避免模块正在被使用时卸载该模块  THIS_MODULE定义在/linux/module.h中
	// .llseek =   scull_llseek,
	.read =     scull_read,
	.write =    scull_write,
	// .ioctl =    scull_ioctl,
	.open =     scull_open,
	// .release =  scull_release,
};

static int ldm_init(void)
{
	printk("%s:%s\n", __FILE__, __FUNCTION__);
	s32 ret = 0;
	dev_t devno = 0;
	if (scull_major) {
		devno = MKDEV(scull_major, scull_minor);
		ret = register_chrdev_region(devno, scull_nr_devs, "scull");
	} else {
		ret = alloc_chrdev_region(&devno, scull_minor, scull_nr_devs, "scull");
		scull_major = MAJOR(devno);
	}
	if (ret < 0) {
		printk("chardev reginon failed\n");
		goto err_chardev_region;
	}

	//为struct_devices分配空间
	scull_devices = (struct scull_dev *)kmalloc(scull_nr_devs * sizeof(struct scull_dev), GFP_KERNEL);
	if (!scull_devices) {
		ret = -ENOMEM;
		goto err_kmalloc;  
	}
	memset(scull_devices, 0, scull_nr_devs * sizeof(struct scull_dev));
	size_t i;
	for (i = 0; i < scull_nr_devs; i++) { //初始化每个device
		scull_devices[i].quantum = scull_quantum;
		scull_devices[i].qset = scull_qset;
		scull_setup_cdev(&scull_devices[i], i);
	}
	return 0;
err_chardev_region:
err_kmalloc:
	return ret;
}
static void ldm_exit(void)
{
	printk("%s:%s\n", __FILE__, __FUNCTION__);	
	int i;
	dev_t devno = MKDEV(scull_major, scull_minor);

	/* Get rid of our char dev entries */
	if (scull_devices) {
		for (i = 0; i < scull_nr_devs; i++) {
			scull_trim(scull_devices + i);
			cdev_del(&scull_devices[i].cdev);
		}
		kfree(scull_devices);
	}

	/* cleanup_module is never called if registering failed */
	unregister_chrdev_region(devno, scull_nr_devs);

}
module_init(ldm_init);
module_exit(ldm_exit);

MODULE_LICENSE("GPL");



/* 为每个设备进行一些初始化工作*/
static void scull_setup_cdev(struct scull_dev *scull_dev, int index)
{
	printk("%s:%s\n", __FILE__, __FUNCTION__);
	dev_t devno = MKDEV(scull_major, scull_minor + index);
	cdev_init(&scull_dev->cdev, &scull_fops);
	scull_dev->cdev.owner = THIS_MODULE;
	int ret = cdev_add(&scull_dev->cdev, devno, 1);
	if (ret < 0) {
		printk("cdev_add %d failed.\n", index);
	}
}

static int scull_open(struct inode *inode, struct file *filp)
{
	printk("%s:%s\n", __FILE__, __FUNCTION__);
	struct scull_dev *scull_dev;
	scull_dev = container_of(inode->i_cdev, struct scull_dev, cdev);
	filp->private_data = scull_dev;

	//判断是否以只读方式打开,如果是的话截断为0
	if ((filp->f_flags & O_RDONLY) || (filp->f_flags & O_RDWR)) {
		scull_trim(scull_dev);
	}
	return 0;
}
static int scull_trim(struct scull_dev *scull_dev)
{
	struct scull_qset *next, *dptr;
	int qset = scull_dev->qset;   /* "dev" is not-null */
	int i;

	for (dptr = scull_dev->data; dptr; dptr = next) { /* all the list items */
		if (dptr->data) {
			for (i = 0; i < qset; i++)
				kfree(dptr->data[i]);
			kfree(dptr->data);
			dptr->data = NULL;
		}
		next = dptr->next;
		kfree(dptr);
	}
	scull_dev->size = 0;
	scull_dev->quantum = scull_quantum;
	scull_dev->qset = scull_qset;
	scull_dev->data = NULL;
	return 0;
}

static ssize_t scull_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos)
{
	struct scull_dev *scull_dev = filp->private_data; 
	struct scull_qset *dptr;	/* the first listitem */
	int quantum = scull_dev->quantum, qset = scull_dev->qset;
	int itemsize = quantum * qset; /* how many bytes in the listitem */
	int item, s_pos, q_pos, rest;
	ssize_t retval = 0;

	if (*f_pos >= scull_dev->size)
		goto out;
	if (*f_pos + count > scull_dev->size)
		count = scull_dev->size - *f_pos;

	/* find listitem, qset index, and offset in the quantum */
	item = (long)*f_pos / itemsize;
	rest = (long)*f_pos % itemsize;
	s_pos = rest / quantum; q_pos = rest % quantum;

	/* follow the list up to the right position (defined elsewhere) */
	dptr = scull_follow(scull_dev, item);

	if (!dptr || !dptr->data || ! dptr->data[s_pos])
		goto out; /* don't fill holes */

	/* read only up to the end of this quantum */
	if (count > quantum - q_pos)
		count = quantum - q_pos;

	if (copy_to_user(buf, dptr->data[s_pos] + q_pos, count)) {
		retval = -EFAULT;
		goto out;
	}
	*f_pos += count;
	retval = count;

out:
	return retval;
}

ssize_t scull_write(struct file *filp, const char __user *buf, size_t count, loff_t *f_pos)
{
	struct scull_dev *scull_dev = filp->private_data;
	struct scull_qset *dptr;
	int quantum = scull_dev->quantum, qset = scull_dev->qset;
	int itemsize = quantum * qset;
	int item, s_pos, q_pos, rest;
	ssize_t retval = -ENOMEM; /* 用于goto错误处理 */


	/* find listitem, qset index and offset in the quantum */

	item = (long)*f_pos / itemsize; // 在第几个链表项
	rest = (long)*f_pos % itemsize; //在某个链表项中的位置
	s_pos = rest / quantum; //在哪个数组
	q_pos = rest % quantum; //在该数组的位置
	

	/* follow the list up to the right position */
	dptr = scull_follow(scull_dev, item);
	if (dptr == NULL)
		goto out;
	if (!dptr->data) {
		dptr->data = kmalloc(qset * sizeof(char *), GFP_KERNEL);
		if (!dptr->data)
			goto out;
		memset(dptr->data, 0, qset * sizeof(char *));
	}
	if (!dptr->data[s_pos]) {
		dptr->data[s_pos] = kmalloc(quantum, GFP_KERNEL);
		if (!dptr->data[s_pos])
			goto out;
	}
	/* write only up to the end of this quantum */
	if (count > quantum - q_pos)
		count = quantum - q_pos;

	if (copy_from_user(dptr->data[s_pos]+q_pos, buf, count)) {
		retval = -EFAULT;
		goto out;
	}
	*f_pos += count;
	retval = count;

        /* update the size */
	if (scull_dev->size < *f_pos)
		scull_dev->size = *f_pos;

out:
	return retval;
}

struct scull_qset *scull_follow(struct scull_dev *scull_dev, int n)
{
	struct scull_qset *qs = scull_dev->data;

    /* Allocate first qset explicitly if need be */
	if (!qs) {
		qs = scull_dev->data = kmalloc(sizeof(struct scull_qset), GFP_KERNEL);
		if (qs == NULL)
			return NULL;  /* Never mind */
		memset(qs, 0, sizeof(struct scull_qset));
	}

	/* Then follow the list */
	while (n--) {
		if (!qs->next) {
			qs->next = kmalloc(sizeof(struct scull_qset), GFP_KERNEL);
			if (qs->next == NULL)
				return NULL;  /* Never mind */
			memset(qs->next, 0, sizeof(struct scull_qset));
		}
		qs = qs->next;
		continue;
	}
	return qs;
}