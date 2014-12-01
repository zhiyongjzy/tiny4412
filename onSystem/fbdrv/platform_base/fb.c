#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>

#include <linux/platform_device.h>
#include <linux/clk.h>
#include <linux/fs.h>
#include <linux/ioport.h>
#include <linux/io.h>
#include <plat/fb.h>
#include <linux/fb.h>
#include <linux/dma-mapping.h> 

#include "fb.h"


static struct fb_info *fb;
struct fb_pri_data {
	size_t fb_reg;
	size_t sclk_fimd;
	struct fb_ops fops;
	struct platform_driver pdrv;
	struct resource *res;
	size_t reg;
	struct s3c_fb_platdata *pd;
};
static struct fb_pri_data *fb_pri;

static int ldm_probe(struct platform_device *pdev)
{
	printk("%s:%s\n", __FILE__, __FUNCTION__);
	//使能lcd时钟
	struct clk *lcd_clk = clk_get(&pdev->dev, "lcd");
	if (IS_ERR(lcd_clk)) {
		printk("struct clk lcd not found\n");
		goto err_clk_enable;
	}
	clk_enable(lcd_clk);

	//获取时钟频率
	struct clk *bus_clk = clk_get(&pdev->dev, "sclk_fimd");
	if (IS_ERR(bus_clk)) {
		fb_pri->sclk_fimd = 800000000; //100MHZ
	} else {
		fb_pri->sclk_fimd = clk_get_rate(bus_clk);
		printk("get sclk_fimd %d\n", fb_pri->sclk_fimd);
	}
	clk_enable(bus_clk);

	//获取resources中的寄存器地址
	fb_pri->res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	fb_pri->reg = (size_t)ioremap(fb_pri->res->start, fb_pri->res->end - fb_pri->res->start + 1);

	//取得platform_device提供的私有数据空间中的setup_gpio函数指针,设置gpio
	fb_pri->pd = (struct s3c_fb_platdata *)pdev->dev.platform_data;
	fb_pri->pd->setup_gpio();
	

	fill_fb_info();
	hardware_init();
	//5 注册主对象
	int ret = register_framebuffer(fb);
	if (ret < 0) {
		printk("register_framebuffer failed\n");
		goto err_register_framebuffer;
	}


	return 0;
err_clk_enable:
	return -1;
err_register_framebuffer:
	return ret;
}
static int ldm_init(void)
{
	printk("%s:%s\n", __FILE__, __FUNCTION__);
	alloc_framebuffer_space();

	fb_pri->pdrv.probe = ldm_probe;
	fb_pri->pdrv.driver.name = "exynos4-fb";
	platform_driver_register(&fb_pri->pdrv);
	return 0;
}
static void ldm_exit(void)
{
	printk("ByeBye\n");

	unregister_framebuffer(fb);
	dma_free_writecombine(NULL, fb->screen_size, fb->screen_base, (dma_addr_t)fb->fix.smem_start);
	iounmap((void *)fb_pri->reg);
	
	framebuffer_release(fb);
	platform_driver_unregister(&fb_pri->pdrv);


	return;
}
module_init(ldm_init);         
module_exit(ldm_exit);

MODULE_LICENSE("GPL");



static int alloc_framebuffer_space(void)
{
	int ret;
	//1 创建主对象，基于基类fb_info的派生，包含私有数据
	fb = framebuffer_alloc(sizeof(struct fb_pri_data), NULL);
	if (fb == NULL) {
		printk("framebuffer_alloc failed\n");
		ret = -ENOMEM;
		goto err_framebuffer_alloc;
	}
	fb_pri = (struct fb_pri_data *)fb->par;

	// 2.创建显存空间
	fb->fix.smem_len = ROW * COL * pixel_size;
	fb->screen_size = fb->fix.smem_len;
	fb->screen_base = dma_alloc_writecombine(NULL, fb->screen_size, (dma_addr_t *)&fb->fix.smem_start, GFP_KERNEL);
	if (fb->screen_base == NULL) {
		printk("dma_alloc_writecombine failed\n");
		ret = -ENOMEM;
		goto err_dma_alloc_writecombine;
	}

	return 0;
err_framebuffer_alloc:
err_dma_alloc_writecombine:
	return ret;
}
static void fill_fb_info(void)
{
	//3 继续填充fb_info成员
	fb->var.xres = ROW;
	fb->var.yres = COL;
	fb->var.xres_virtual = ROW;
	fb->var.yres_virtual = COL;
	fb->var.xoffset = 0;
	fb->var.yoffset = 0;
	fb->var.bits_per_pixel = pixel_size * 8;

	fb->var.red.offset = 16;
	fb->var.green.offset = 8;
	fb->var.blue.offset = 0;
	fb->var.transp.offset = 24;

	fb->var.red.length = 8;
	fb->var.green.length = 8;
	fb->var.blue.length = 8;
	fb->var.transp.length = 8;

	fb->var.red.msb_right = fb->var.green.msb_right = fb->var.blue.msb_right = fb->var.transp.msb_right = 1;

	fb->fix.line_length = ROW * pixel_size;
	fb->fix.visual = FB_VISUAL_TRUECOLOR; //真彩色（非颜色表）

	// 填充操作方法
	fb_pri->fops.fb_fillrect = cfb_fillrect;
	fb_pri->fops.fb_copyarea = cfb_copyarea;
	fb_pri->fops.fb_imageblit = cfb_imageblit;

	u32 palette[16] = {0};
	fb->pseudo_palette = palette;
	fb_pri->fops.fb_setcolreg = fb_setcolreg;

	fb->fbops = &fb_pri->fops;
}
static inline void lcd_off(void)
{
	WINCON0 &= ~1;
	WINCON1 &= ~1;
	WINCON2 &= ~1;
	WINCON3 &= ~1;
	WINCON4 &= ~1;

	VIDCON0 &= ~0b11; // 关闭lcd控制器
}

static inline void lcd_on(void)
{
	VIDCON0 |= 0b11;

	WINCON0 |= 1;
	// WINCON1 |= 1;
	// WINCON2 |= 1;
}
static int hardware_init(void)
{

	lcd_off();
	VIDCON0 &= 0; //先清零
	VIDCON0 = VIDCON0 | (1 << 5) | ((fb_pri->sclk_fimd / LCD_CLK - 1) << 6) | (1 << 16) | (0 << 17) | (0 << 18) | (0b000 << 26) ;

	/* VIDCON1： 设置时序极性，与fimd时序极性相同的为normal，相反的为inverted
	 */
	VIDCON1 = IVDEN << 4 | IVSYNC << 5 | IHSYNC << 6 | IVCLK << 7;

	//设置时序
	VIDTCON0 = ( VSPW & 0xff ) | ( VFPD & 0xff ) << 8 | ( VBPD & 0xff ) << 16 ;
	VIDTCON1 = ( HSPW & 0xff ) | ( HFPD & 0xff ) << 8 | ( HBPD & 0xff ) << 16 ;

	//分辨率
	VIDTCON2 = (COL - 1) << 11 | (ROW - 1);

	SHADOWCON |= 0b11111;

	//======================================
	//WINCON0: window0的主控制寄存器

	WINCON0 = bpp << 2 | word_swap << 15 | halfword_swap << 16 | byte_swap << 17 | bit_swap << 18;
	//window显示范围的左上角坐标
	VIDOSD0A = 0;
	//window显示范围的右下角坐标
	VIDOSD0B = (ROW - 1) << 11 | (COL - 1);
	//显示范围的所需字数 OSDSIZE
	VIDOSD0C = ROW * COL * pixel_size / 4;

	//显存首末地址，关于显存终止地址公式的由来，见手册P1813
	VIDW00ADD0B0 = fb->fix.smem_start;
	VIDW00ADD1B0 = fb->fix.smem_start + ROW * COL * pixel_size;
	VIDW00ADD2 = ROW * pixel_size;

	//开启lcd控制器
	lcd_on();


	return 0;
}




static inline u_int chan_to_field(u_int chan, struct fb_bitfield *bf)
{
	chan &= 0xffff;
	chan >>= 16 - bf->length;
	return chan << bf->offset;
}
static int fb_setcolreg(unsigned regno, unsigned red, unsigned green, unsigned blue, unsigned transp, struct fb_info *info)
{
	u32 val;
	if (regno < 16) {
		u32 *pal = info->pseudo_palette;
		val  = chan_to_field(red,   &info->var.red);
		val |= chan_to_field(green, &info->var.green);
		val |= chan_to_field(blue,  &info->var.blue);
		pal[regno] = val;
	}
	return 0;
}