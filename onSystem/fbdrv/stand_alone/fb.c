#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>

#include <linux/fb.h>
#include <linux/dma-mapping.h> 
#include <linux/clk.h> 
#include <linux/io.h>

#include "fb.h"

static struct fb_info *fb;
struct fb_pri_data {
	size_t fb_reg;
	size_t clk_reg;
	size_t lcdblkcfg_reg;
	struct clk *clk_bus;
	struct clk *clk_sclk_fimd;
	u32 sclk_fimd;
	struct fb_ops fops; 
};
static struct fb_pri_data *fb_pri;

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
	//4.1 打开时钟，并获取sclk总线时钟
	fb_pri->clk_bus = clk_get(NULL, "fimd");
	if (IS_ERR(fb_pri->clk_bus)) {
		printk("struct clk lcd not found\n");
		// goto err_clk_get_lcd;
	}
	// clk_enable(fb_pri->clk_bus);

	//=========================================================
	CLK_GATE_IP_LCD0 |= 1; //最低位给1
	//==========================================================

	fb_pri->clk_sclk_fimd = clk_get(NULL, "sclk_fimd");
	if (IS_ERR(fb_pri->clk_sclk_fimd)) {
		printk("struct clk sclk_fimd not found\n");
		fb_pri->sclk_fimd = 160000000;
	} else {
		fb_pri->sclk_fimd = clk_get_rate(fb_pri->clk_sclk_fimd);
		printk("fb sclk = %u\n", fb_pri->sclk_fimd);
	}

	//4.2 ioremap   fb.h
	fb_pri->fb_reg = (size_t)ioremap(0x11c00000, 0x112); //没有映射颜色表
	fb_pri->clk_reg = (size_t)ioremap(0x10030000, 0xc538);
	fb_pri->lcdblkcfg_reg = (size_t)ioremap(0x10010210, 4);




	//2 配置输入时钟sclk, 手册P457右上角图
	CLK_SRC_LCD0 = (CLK_SRC_LCD0 & ~0xf) | 0b0110; //选择MPLL输出做时钟源
	CLK_DIV_LCD &= ~0xf; //选择FIMD的输入时钟源分频值 不分频

	//3 配置lcd系统时钟由FIMD来使用, LCDBLK_CFG[1], 手册P884
	LCDBLK_CFG = LCDBLK_CFG | (1 << 1); //选1旁路FIMD


	//设置寄存器
	//1 初始化lcd的GPIO管脚
	GPF0CON = 0x22222222;
	GPF1CON = 0x22222222;
	GPF2CON = 0x22222222;
	GPF3CON = (GPF3CON & ~0xfffff) | 0x22222;


	GPF0PUD = 0x0000ffff;
	GPF1PUD = 0x0000ffff;
	GPF2PUD = 0x0000ffff;
	GPF3PUD = 0x0000ffff;

	//MAX drive strength­­­­­­­­­//
	GPF0DRV = 0x0000ffff;
	GPF1DRV = 0x0000ffff;
	GPF2DRV = 0x0000ffff;
	GPF3DRV = 0x0000ffff;


	//4 关闭lcd控制器，必须先关闭lcd控制器才能做初始化设置
	lcd_off();
	// printk("================  sclk_fimd %d\n", fb_pri->sclk_fimd);
	VIDCON0 &= 0; //先清零
	VIDCON0 = VIDCON0 | (1 << 5) | ((MPLL_CLK / LCD_CLK - 1) << 6) | (1 << 16) | (0 << 17) | (0 << 18) | (0b000 << 26) ;

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
err_clk_get_lcd:
	return -1;

}
static int ldm_init(void)
{
	printk("%s:%s\n", __FILE__, __FUNCTION__);

	//1 创建主对象 申请显存，基于基类fb_info的派生，包含私有数据
	alloc_framebuffer_space();
	
	//2 继续填充fb_info成员
	fill_fb_info();
	
	//3 硬件初始化
	hardware_init();

	//4 填充操作方法
	fb_pri->fops.fb_fillrect = cfb_fillrect;
	fb_pri->fops.fb_copyarea = cfb_copyarea;
	fb_pri->fops.fb_imageblit = cfb_imageblit;
	fb->fbops = &fb_pri->fops;

	//5 注册主对象
	int ret = register_framebuffer(fb);
	if (ret < 0) {
		printk("register_framebuffer failed\n");
		goto err_register_framebuffer;
	}


	return 0;
err_register_framebuffer:
	return ret;
}
static void ldm_exit(void)
{
	printk("%s:%s\n", __FILE__, __FUNCTION__);
	
	unregister_framebuffer(fb);
	dma_free_writecombine(NULL, fb->screen_size, fb->screen_base, (dma_addr_t)fb->fix.smem_start);
	iounmap((void *)fb_pri->fb_reg);
	iounmap((void *)fb_pri->clk_reg);
	iounmap((void *)fb_pri->lcdblkcfg_reg);

	// clk_disable(fb_pri->clk_bus);
	
	framebuffer_release(fb);

}

module_init(ldm_init);
module_exit(ldm_exit);

MODULE_LICENSE("GPL");