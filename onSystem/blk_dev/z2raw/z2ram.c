#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>

#include <linux/blkdev.h>
#include <linux/vmalloc.h>
#include <linux/hdreg.h>

//参考/drivers/block/z2ram.c

#define DEV_NAME "z2r"

enum {
	DEV_SIZE = 1024 * 1024 * 10, //设备的有效存储空间
	SECTOR_SIZE = 512, //最小存储单位的大小
};

struct z2ram_info {
	struct gendisk *disk;
	struct block_device_operations z2_fops;
	spinlock_t lock;
	void *addr; //伪存储设备存储空间首地址
};

static struct z2ram_info z2r;

static int z2r_getgeo(struct block_device *bdev, struct hd_geometry *geo)
{
	geo->heads = 2;
	geo->sectors = 10;
	geo->cylinders = DEV_SIZE / SECTOR_SIZE / geo->heads / geo->sectors;
	return 0;
}

//处理块设备上的操作请求
static void do_blk_req(struct request_queue *q)
{
	//1 从队列上提取出一个request请求
	struct request *req;
	req = blk_fetch_request(q);

	while (req) {
		int err = 0;
		unsigned long start = blk_rq_pos(req) * SECTOR_SIZE;//获取本次操作的起始字节位置 <<9 相当于 乘以512
		unsigned long len  = blk_rq_cur_bytes(req);

		//检查访问是否越界
		if (start + len >= DEV_SIZE) {
			printk("DEV_NAME: bad access: block=%llu, count=%u\n", (unsigned long long)blk_rq_pos(req), blk_rq_cur_sectors(req));
			err = -EIO;//IO输入输出错误
			goto out;
		}

		if (rq_data_dir(req) == READ) {
			memcpy(req->buffer, (u8 *)z2r.addr + start, len);
			printk("read %ld bytes from %ld\n", len, start);
		} else {
			memcpy((u8 *)z2r.addr + start, req->buffer, len);
			printk("write %ld bytes from %ld\n", len, start);
		}
	out:
		//检查当前request是否已经完成
		if (!__blk_end_request_cur(req, err))
			req = blk_fetch_request(q);
	}
}
static int __init z2ram_init(void)
{
	printk("%s:%s\n", __FILE__, __FUNCTION__);
	int ret = 0;
	//1.创建主对象gendisk,参数是次设备号上限。块设备的次设备号是分配个分区的
	z2r.disk = alloc_disk(10);
	if (!z2r.disk) {
		printk("alloc_disk failed.\n");
		ret = -ENOMEM;
		goto err_alloc_disk;
	}

	//2 申请设备号，如果第一个参数填0，则设备号自动分配
	z2r.disk->major = register_blkdev(0, DEV_NAME);
	if (z2r.disk->major < 0) {
		printk("register_blkdev failed.\n");
		ret = z2r.disk->major;
		goto err_register_blkdev;
	}

	//3 填充成员
	// z2r.disk->major = ;
    sprintf(z2r.disk->disk_name, DEV_NAME);
	// z2r.disk->minors = 10; //最大分区数
	z2r.z2_fops.getgeo = z2r_getgeo;
	z2r.disk->fops = &z2r.z2_fops;

	//4 创建块设备的操作队列
	spin_lock_init(&z2r.lock); //初始化自旋锁，该锁由块设备驱动框架使用
	z2r.disk->queue = blk_init_queue(do_blk_req, &z2r.lock);
	if (!z2r.disk->queue) {
		printk("blk_init_queue failed.\n");
		ret = -EINVAL;
		goto err_blk_init_queue;
	}
	//5 创建属于设备的存储空间。
	z2r.addr = vzalloc(DEV_SIZE);
	if (!z2r.addr) {
		ret = -ENOMEM;
		printk("vzalloc failed.\n");
		goto err_vzalloc;
	}
	set_capacity(z2r.disk, DEV_SIZE / SECTOR_SIZE);//设定块设备大小, 单位是最小存储单元的个数
	add_disk(z2r.disk);

	return 0;
err_vzalloc:
	blk_cleanup_queue(z2r.disk->queue);
err_blk_init_queue:
    unregister_blkdev(z2r.disk->major, DEV_NAME);
err_register_blkdev:
	put_disk(z2r.disk);
err_alloc_disk:
	return ret;
}

static void __exit z2ram_exit(void)
{
	printk("%s:%s\n", __FILE__, __FUNCTION__);
	del_gendisk(z2r.disk);
    blk_cleanup_queue(z2r.disk->queue);
	vfree(z2r.addr);
    unregister_blkdev(z2r.disk->major, DEV_NAME);
    put_disk(z2r.disk);
}

module_init(z2ram_init);
module_exit(z2ram_exit);
MODULE_LICENSE("GPL");
